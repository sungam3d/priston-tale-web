# install_autostart.ps1 - keep server.exe running, with its window on your desktop:
# starts it when you sign in and starts it again within 5 minutes if it ever stops.
#
# Run once in an ADMIN PowerShell, signed in as the account that should own the window:
#   powershell -ExecutionPolicy Bypass -File install_autostart.ps1
#
# A window needs a signed-in user, so:
#   - after a reboot the server waits until you sign in
#   - disconnect from Remote Desktop rather than signing out: signing out closes the server
#   - closing the window saves and stops the server, and the task starts it again within 5 minutes
#
#   Stop the world:     Disable-ScheduledTask PristonServer, then close the server window
#   Start it again:     Enable-ScheduledTask PristonServer; Start-ScheduledTask PristonServer
#   Remove autostart:   Unregister-ScheduledTask PristonServer -Confirm:$false
# (Stop-ScheduledTask ends server.exe without its own shutdown, like a power cut:
#  characters of players still online lose what they did since their last save.)

$ErrorActionPreference = 'Stop'
$TaskName = 'PristonServer'
$dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe = Join-Path $dir 'server.exe'

if ( -not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator) ) {
	throw 'Run this from an admin PowerShell (right-click > Run as administrator).'
}
if ( -not [IO.File]::Exists($exe) ) { throw "server.exe not found next to this script ($exe)" }

# so an old task's 5-minute trigger cannot start another copy while this runs
if ( Get-ScheduledTask $TaskName -ErrorAction SilentlyContinue ) { Disable-ScheduledTask $TaskName | Out-Null }

# a running server.exe holds port 10007: close it the normal way (like closing its
# window, so characters are saved) and wait for it to finish. A copy running as SYSTEM
# has no window this session can close, so that one has to be ended.
foreach ( $p in Get-Process server -ErrorAction SilentlyContinue ) {
	Write-Host "Closing the running server.exe (pid $($p.Id)) so the task can take over..."
	if ( $p.CloseMainWindow() ) { $p.WaitForExit(120000) | Out-Null }
	if ( -not $p.HasExited ) {
		Write-Warning "server.exe (pid $($p.Id)) has no window to close here: ending it. Players online lose what they did since their last save."
		taskkill /F /PID $p.Id /T | Out-Null
		$p.WaitForExit(30000) | Out-Null
	}
	if ( -not $p.HasExited ) { throw "server.exe (pid $($p.Id)) did not stop - end it in Task Manager, then run this again." }
}

$user = "$env:USERDOMAIN\$env:USERNAME"
$action = New-ScheduledTaskAction -Execute $exe -WorkingDirectory $dir
# at sign-in, and every 5 minutes: while server.exe is running the new start is
# skipped (IgnoreNew), so this only restarts it after it stopped
$atLogon = New-ScheduledTaskTrigger -AtLogOn -User $user
$every5 = New-ScheduledTaskTrigger -Once -At (Get-Date).Date -RepetitionInterval (New-TimeSpan -Minutes 5) -RepetitionDuration (New-TimeSpan -Days 3650)
$settings = New-ScheduledTaskSettingsSet -MultipleInstances IgnoreNew -ExecutionTimeLimit ([TimeSpan]::Zero) `
	-AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -StartWhenAvailable -Priority 4
# Interactive: runs on this user's desktop, so the window can be seen and closed
$principal = New-ScheduledTaskPrincipal -UserId $user -LogonType Interactive -RunLevel Highest

Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $atLogon, $every5 -Settings $settings `
	-Principal $principal -Description 'Priston Tale game server for online.priston.world (server.exe on port 10007)' -Force | Out-Null
Enable-ScheduledTask -TaskName $TaskName | Out-Null
Start-ScheduledTask -TaskName $TaskName

Write-Host "Task '$TaskName' registered and started. The world takes about a minute to load."
for ( $i = 0; $i -lt 24; $i++ ) {
	Start-Sleep 5
	if ( Get-NetTCPConnection -LocalPort 10007 -State Listen -ErrorAction SilentlyContinue ) { Write-Host 'server.exe is listening on port 10007.'; return }
}
Write-Warning 'server.exe is not listening on port 10007 yet - check LogFile\ and Task Scheduler > PristonServer > History.'
