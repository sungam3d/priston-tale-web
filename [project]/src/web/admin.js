// admin.js - "Admin mode" switch and the list of admin hotkeys and commands, in the
// bar under the game. Loaded by the page (web\shell.html) after Module is defined;
// copied next to it by web\make_site.py.
//
// Admin mode is the game's own GM mode (smConfig.DebugMode, fileread.cpp): the
// client turns it on from a hidden line in hotuk.ini - the English-build keywords of
// szConfigIniCommand[21] and [20] followed by "/0" (works from any IP). With it on,
// the game asks the server for admin level 1 at login (SendAdminMode). Levels 2-4
// stay behind the passwords in server.ini (*ADMIN_NAME / *ADMIN_COMMAND).
// The setting is kept in this browser (localStorage); the game reads hotuk.ini when
// it starts, so switching reloads the page.
//
// Also the "Shadows" switch: real-time character shadows (ptgfx_shadow.cpp) read
// Module.ptShadows once at start-up; off = the old round shadows (slower PCs).
(function () {
	if (!window.Module) return;
	var KEY = 'pt-admin-mode', SHADOW_KEY = 'pt-shadows';
	var on = false, shadows = true;
	try { on = localStorage.getItem(KEY) == '1'; shadows = localStorage.getItem(SHADOW_KEY) != 'off'; } catch (e) {}
	Module.ptShadows = shadows;

	// ---- the hotuk.ini line (cp949 bytes, exactly as in fileread.cpp) ----
	var CMD21 = [0xC6, 0xC4, 0xBE, 0xC6, 0xB6, 0xF5];
	var CMD20 = [0xBE, 0xCB, 0xB7, 0xBA, 0xBD, 0xBA].concat(Array.from('*Bobby&', function (c) { return c.charCodeAt(0); }),
		[0xC0, 0xCC, 0xBD, 0xBA, 0xB8, 0xB6, 0xBF, 0xA4, 0x7E, 0x2D, 0xA4, 0xCC, 0xA4, 0xBB]);
	var LINE = [0x0D, 0x0A].concat(CMD21, [0x09], CMD20, [0x09, 0x2F, 0x30, 0x0D, 0x0A]);

	function addAdminLine() {
		var path = '/game/hotuk.ini', old;
		try { old = FS.readFile(path); } catch (e) { old = new Uint8Array(0); }
		var out = new Uint8Array(old.length + LINE.length);
		out.set(old); out.set(LINE, old.length);
		FS.writeFile(path, out);
		Module.print('[web] admin mode on');
	}

	if (on) {
		// hotuk.ini arrives in the start-up pack, which web\pre.js unpacks in its own
		// preRun; when done it writes /game_saves_restored.txt. Hold the start until
		// then (only this dependency left), and add the line before the game reads it.
		Module.preRun = Module.preRun || [];
		Module.preRun.unshift(function () {
			var done = false;
			var orig = Module.monitorRunDependencies;
			Module.monitorRunDependencies = function (left) {
				if (orig) orig.call(Module, left);
				if (left != 1 || done) return;
				try { if (!FS.analyzePath('/game_saves_restored.txt').exists) return; } catch (e) { return; }
				done = true;
				try { addAdminLine(); } catch (e) { Module.printErr('[web] admin mode not set: ' + e); }
				removeRunDependency('pt-admin');
			};
			addRunDependency('pt-admin');
		});
	}

	// ---- bar buttons and help panel ----
	var css = document.createElement('style');
	css.textContent =
		'#bar .pt-admin-on{border-color:#9a7430;color:#e8cf8e}' +
		'.pt-help{position:fixed;inset:0;background:rgba(5,5,8,.72);display:flex;align-items:flex-start;justify-content:center;z-index:50;overflow:auto;padding:24px 16px}' +
		'.pt-help[hidden]{display:none}' +
		'.pt-help-box{background:#15151c;border:1px solid #3a3426;border-radius:8px;color:#d8d4c8;max-width:680px;width:100%;padding:20px 22px;font:13.5px/1.55 system-ui,sans-serif;box-sizing:border-box}' +
		'.pt-help-box h2{font:600 19px/1.3 Georgia,"Times New Roman",serif;color:#e8cf8e;margin:0 0 4px}' +
		'.pt-help-box h3{font:600 14px/1.3 system-ui,sans-serif;color:#e8cf8e;margin:18px 0 6px}' +
		'.pt-help-box p{margin:6px 0;color:#a8a294}' +
		'.pt-help-box table{border-collapse:collapse;width:100%}' +
		'.pt-help-box td{padding:5px 8px;border-top:1px solid #26242e;vertical-align:top}' +
		'.pt-help-box td:first-child{white-space:nowrap;width:1%}' +
		'.pt-help-box kbd,.pt-help-box code{font:12.5px ui-monospace,Consolas,monospace;background:#22212a;border:1px solid #34323c;border-radius:4px;padding:1px 5px;color:#e4dfd2}' +
		'.pt-help-box pre{font:12.5px/1.5 ui-monospace,Consolas,monospace;background:#0e0e13;border:1px solid #26242e;border-radius:6px;padding:8px 10px;overflow-x:auto;margin:6px 0}' +
		'.pt-help-head{display:flex;justify-content:space-between;align-items:flex-start;gap:12px}' +
		'.pt-help-box button{background:#22212a;color:#d8d4c8;border:1px solid #34323c;border-radius:4px;padding:4px 10px;font:inherit;cursor:pointer}' +
		'.pt-help-box button:hover{border-color:#5a5566}';
	document.head.appendChild(css);

	var bar = document.getElementById('bar');
	if (!bar) return;
	var anchor = document.getElementById('saveInfo');

	var btnAdmin = document.createElement('button');
	btnAdmin.id = 'btnAdmin';
	btnAdmin.type = 'button';
	btnAdmin.textContent = on ? 'Admin mode: ON' : 'Admin mode: off';
	btnAdmin.title = on ? 'Turn admin mode off (restarts the game)' : 'Turn on the game\'s admin mode for this browser (restarts the game)';
	if (on) btnAdmin.className = 'pt-admin-on';
	btnAdmin.addEventListener('click', function () {
		try { if (on) localStorage.removeItem(KEY); else localStorage.setItem(KEY, '1'); }
		catch (e) { Module.setStatus('Admin mode needs browser storage, which is blocked here'); return; }
		btnAdmin.disabled = true;
		btnAdmin.textContent = on ? 'Turning admin mode off...' : 'Turning admin mode on...';
		location.reload();
	});

	var btnHelp = document.createElement('button');
	btnHelp.id = 'btnHelp';
	btnHelp.type = 'button';
	btnHelp.textContent = 'Commands';
	btnHelp.title = 'Admin hotkeys and chat commands';

	var btnShadows = document.createElement('button');
	btnShadows.id = 'btnShadows';
	btnShadows.type = 'button';
	btnShadows.textContent = shadows ? 'Shadows: on' : 'Shadows: off';
	btnShadows.title = shadows ? 'Use the simple round shadows instead (faster on old PCs; restarts the game)' :
		'Turn on real-time character shadows (restarts the game)';
	btnShadows.addEventListener('click', function () {
		try { if (shadows) localStorage.setItem(SHADOW_KEY, 'off'); else localStorage.removeItem(SHADOW_KEY); }
		catch (e) { Module.setStatus('This setting needs browser storage, which is blocked here'); return; }
		btnShadows.disabled = true;
		location.reload();
	});

	bar.insertBefore(btnAdmin, anchor);
	bar.insertBefore(btnHelp, anchor);
	bar.insertBefore(btnShadows, anchor);

	var panel = document.createElement('div');
	panel.className = 'pt-help';
	panel.hidden = true;
	panel.setAttribute('role', 'dialog');
	panel.setAttribute('aria-label', 'Admin hotkeys and commands');
	panel.innerHTML =
		'<div class="pt-help-box">' +
		'<div class="pt-help-head"><div><h2>Admin mode</h2>' +
		'<p>' + (on ? 'Admin mode is <b>on</b> in this browser.' : 'Admin mode is <b>off</b>. Click <b>Admin mode</b> under the game to turn it on; the game restarts.') +
		' It applies from your next login.</p></div><button type="button" id="btnHelpClose">Close</button></div>' +

		'<h3>Hotkeys</h3><table>' +
		'<tr><td><kbd>Shift</kbd> while moving</td><td>Run very fast</td></tr>' +
		'<tr><td><kbd>N</kbd> while moving</td><td>Move freely through walls and objects</td></tr>' +
		'<tr><td><kbd>B</kbd> + click a stat <b>+</b></td><td>Add 10 stat points at once (when you have 10 or more)</td></tr>' +
		'<tr><td><kbd>Ctrl</kbd> over an item</td><td>Show the item\'s serial number</td></tr>' +
		'<tr><td><kbd>Shift</kbd>+<kbd>Home</kbd> / <kbd>End</kbd></td><td>Dark (night) lighting / normal lighting</td></tr>' +
		'<tr><td><kbd>Shift</kbd>+<kbd>F3</kbd> / <kbd>F4</kbd></td><td>Cycle through NPC / character models</td></tr>' +
		'</table>' +

		'<h3>Chat commands (level 1: everyone in admin mode)</h3><table>' +
		'<tr><td><code>/userid</code> <i>account</i></td><td>Show a player\'s details by account ID</td></tr>' +
		'<tr><td><code>/username</code> <i>character</i></td><td>Show a player\'s details by character name</td></tr>' +
		'</table>' +

		'<h3>Higher levels (server owner)</h3>' +
		'<p>Levels 2&ndash;4 need a password set in <code>server.ini</code> on the server (restart server.exe after editing):</p>' +
		'<pre>*ADMIN_NAME      GameMaster\n*ADMIN_COMMAND   password-for-level-2\n*ADMIN_COMMAND   password-for-level-3\n*ADMIN_COMMAND   password-for-level-4</pre>' +
		'<p>In game, with admin mode on, whisper the name and a password: <code>/:GameMaster password-for-level-4</code>. The reply says <i>Command Level (3)</i> for level 4 (the game counts from level 2 as 1).</p>' +
		'<table>' +
		'<tr><td><code>/where</code> <i>name</i></td><td>Level 2: find where a player is</td></tr>' +
		'<tr><td><code>/near</code> <i>name</i></td><td>Level 2: go to a player</td></tr>' +
		'<tr><td><code>/call</code> <i>name</i></td><td>Level 2: bring a player to you</td></tr>' +
		'<tr><td><code>/kick</code> <i>name</i></td><td>Level 2: disconnect a player</td></tr>' +
		'<tr><td><code>/kill</code> <i>name</i></td><td>Level 2: kill a player\'s character</td></tr>' +
		'<tr><td><code>/open_warehouse</code></td><td>Level 2: open your warehouse anywhere</td></tr>' +
		'<tr><td><code>/@get</code> <i>item code</i></td><td>Level 4: create an item, by its code from the item files</td></tr>' +
		'<tr><td><code>/GiveMeExp!</code> <i>level</i></td><td>Level 4: set your experience to the start of that level</td></tr>' +
		'<tr><td><code>/GiveMeMoney!</code> <i>amount</i></td><td>Level 4: get gold (up to 1,000,000 at a time)</td></tr>' +
		'</table>' +
		'</div>';
	document.body.appendChild(panel);

	function show(v) { panel.hidden = !v; if (v) document.getElementById('btnHelpClose').focus(); else btnHelp.focus(); }
	btnHelp.addEventListener('click', function () { show(true); });
	panel.addEventListener('click', function (e) { if (e.target === panel || e.target.id == 'btnHelpClose') show(false); });
	document.addEventListener('keydown', function (e) { if (e.key == 'Escape' && !panel.hidden) { e.stopPropagation(); show(false); } }, true);
})();
