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
// it starts, so switching reloads the page. The line is added through
// Module.ptFilePatches (settings.js, loaded before this file).
(function () {
	if (!window.Module) return;
	var KEY = 'pt-admin-mode';
	var on = false;
	try { on = localStorage.getItem(KEY) == '1'; } catch (e) {}

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

	// hotuk.ini arrives in the start-up pack: settings.js runs this once it is unpacked,
	// before the game reads it
	if (on) (Module.ptFilePatches = Module.ptFilePatches || []).push(addAdminLine);

	// ---- bar buttons and help panel ----
	var css = document.createElement('style');
	css.textContent =
		'#bar .pt-admin-on{border-color:#9a7430;color:#e8cf8e}' +
		'.pt-help{position:fixed;inset:0;background:rgba(5,5,8,.72);display:flex;align-items:flex-start;justify-content:center;z-index:50;overflow:auto;padding:24px 16px}' +
		'.pt-help[hidden]{display:none}' +
		'.pt-help-box{background:#15151c;border:1px solid #3a3426;border-radius:8px;color:#d8d4c8;max-width:760px;width:100%;padding:20px 22px;font:13.5px/1.55 system-ui,sans-serif;box-sizing:border-box}' +
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

	bar.insertBefore(btnAdmin, anchor);
	bar.insertBefore(btnHelp, anchor);

	// the test site's server has *GM_CHEATS on (server.ini): its section shows only there
	var testSite = /\/tier5\//i.test(location.pathname);

	// one table: rows of [command, arguments, explanation]
	function table(rows) {
		return '<table>' + rows.map(function (r) {
			return '<tr><td><code>' + r[0] + '</code>' + (r[1] ? ' <i>' + r[1] + '</i>' : '') + '</td><td>' + r[2] + '</td></tr>';
		}).join('') + '</table>';
	}

	var panel = document.createElement('div');
	panel.className = 'pt-help';
	panel.hidden = true;
	panel.setAttribute('role', 'dialog');
	panel.setAttribute('aria-label', 'Admin hotkeys and commands');
	panel.innerHTML =
		'<div class="pt-help-box">' +
		'<div class="pt-help-head"><div><h2>Commands</h2>' +
		'<p>' + (on ? 'Admin mode is <b>on</b> in this browser.' : 'Admin mode is <b>off</b>. Click <b>Admin mode</b> under the game to turn it on; the game restarts.') +
		' It applies from your next login. Type commands in the chat box and press <kbd>Enter</kbd>. ' +
		'<i>Italic</i> words are what you fill in; [brackets] mean optional. Names with spaces go in quotes.</p></div>' +
		'<button type="button" id="btnHelpClose">Close</button></div>' +

		'<h3>Everyone (no admin mode needed)</h3>' + table([
			['/:', 'name message', 'Whisper to a player (<code>/;</code> works too)'],
			['//party', 'name', 'Ask a player to join your party'],
			['/TRADE&gt;', 'message', 'Trade chat, seen by everyone on the server'],
			['/CLAN&gt;', 'message', 'Clan chat'],
			['/couple', 'name', 'Warp to your partner (needs the couple ring)'],
			['/disable_whisper', '', 'Block whispers'],
			['/enable_whisper', '', 'Allow whispers again'],
			['/time', '', 'Show your play time'],
			['/latency', 'ms', 'Network smoothing, 0&ndash;500 ms (default 30; under 10 turns it off)'],
			['/camera_wave_off', '', 'Stop the camera swaying'],
			['/camera_wave_on', '', 'Let the camera sway again']
		]) +

		'<h3>Hotkeys (admin mode)</h3><table>' +
		'<tr><td><kbd>Shift</kbd> while moving</td><td>Run very fast</td></tr>' +
		'<tr><td><kbd>N</kbd> while moving</td><td>Move freely through walls and objects</td></tr>' +
		'<tr><td><kbd>B</kbd> + click a stat <b>+</b></td><td>Add 10 stat points at once (when you have 10 or more)</td></tr>' +
		'<tr><td><kbd>Ctrl</kbd> over an item</td><td>Show the item\'s serial number</td></tr>' +
		'<tr><td><kbd>Shift</kbd>+<kbd>Home</kbd> / <kbd>End</kbd></td><td>Dark (night) lighting / normal lighting</td></tr>' +
		'<tr><td><kbd>Shift</kbd>+<kbd>F3</kbd> / <kbd>F4</kbd></td><td>Cycle through NPC / character models</td></tr>' +
		'</table>' +

		(testSite ?
		'<h3>Test server cheats (this test site only)</h3>' +
		'<p>This server runs with <code>*GM_CHEATS</code>, so these work at admin level 1. Log out and back in after a big change if anything looks off.</p>' + table([
			['/MakeMeLevel', 'level', 'Go to that level with the right experience, stat and skill points (saves at once)'],
			['/DeckMeOut!', '[aging]', 'Put on the best gear you can wear for your level and class, at its best stats and specialised for your class: weapon (and shield or orb), armour or robe, boots, gauntlets, armlet, amulet, sheltom and two rings. Add a number to age it, e.g. <code>/DeckMeOut! 10</code>. What you wore goes to your bag, or the ground when it is full'],
			['/IDDQD', '', 'God mode on or off: your life, mana and stamina stay full'],
			['/GiveMeMoney!', 'amount', 'Drop gold at your feet, up to 1,000,000 at a time'],
			['/GiveMeExp!', 'level', 'Set your experience to the start of that level (server side only; <code>/ShowMeTheExp!</code> is the same)'],
			['/@get', 'item code', 'Drop an item at your feet, e.g. <code>/@get ws220</code> (Avenger Sword)'],
			['/@put', 'name item code', 'Give an item to a player'],
			['/PassRankUp', '', 'Rank up one step without the quest, up to rank 5'],
			['/LeeResetRankUp', '1&ndash;5', 'Set your rank back to that rank'],
			['/Lee5thRankUp', 'step', 'Jump to a step of the 5th rank-up quest (1&ndash;10)']
		]) : '') +

		'<h3>Admin level 1 (admin mode on)</h3>' +
		'<p>Server commands:</p>' + table([
			['/user', '', 'Count the players and active monsters on the server'],
			['/area_users', '', 'Count the players in your area'],
			['/userid', 'account', 'Show a player\'s account, character, IP and time online, by account ID'],
			['/username', 'character', 'The same, by character name'],
			['/usercode', 'number', 'The same, by the player\'s connection number'],
			['/hide', '', 'Turn invisible to players'],
			['/show', '', 'Turn visible again'],
			['/monster', 'name [count]', 'Spawn monsters (up to 20) by their exact name in the monster files, e.g. <code>/monster "Lord Hopy"</code>. Most names are Korean. They drop nothing below level 4'],
			['/mymonster', 'name [count]', 'Spawn monsters that fight for you for 4 minutes'],
			['/event_monster', 'name minutes chance', 'Make a monster appear from kills for some minutes (chance out of 1000)'],
			['/area_monster', '', 'Show the spawn points and monster counts of the field you are in'],
			['/arena_monster', '', 'Spawn the arena monster (Fury)'],
			['/specific', 'class code', 'For 1 minute, items you make are specialised to that class'],
			['/itemdaysrevision', 'days', 'For 1 minute, shift new items\' dates by up to &plusmn;30 days'],
			['/get_forceorb', '', 'Show your active force orb: time left and damage'],
			['/get_primitem', '', 'Show the time left on your premium items'],
			['/get_currency', '', 'Show the gold players have spent: crafting, aging, shops, potions, force orbs, warps, skills'],
			['/get_potion', '', 'Show potion use (needs the potion monitor)'],
			['/get_expgametime', '', 'Show your play-time experience bonus'],
			['/get_event_level', '', 'Show your star ticket event level'],
			['/get_bosstime', '', 'Show the minute of the hour bosses spawn'],
			['/localtime', '', 'Show the server\'s clock'],
			['/get_guid', '', 'Show your login IDs'],
			['/dm_info', '', 'Show your damage limits (the server\'s damage check)'],
			['/netlog', '', 'Send yourself the server\'s network log'],
			['/comic_player', 'name [minutes]', 'Give a player a big head'],
			['/player_sight', '[far]', 'How far players see each other during the siege: nothing = near, anything = far'],
			['/sod_view', '', 'Show the Bellatra (SoD) scores'],
			['/sod_enter', 'n', 'Enter a Bellatra (SoD) round'],
			['/show_blackuser', '', 'List blocked players'],
			['/reset_blackuser', '', 'Clear the block list'],
			['/free_blackuser', 'name', 'Unblock one player']
		]) +
		'<p>Bless Castle:</p>' + table([
			['/start_castle', '[minutes]', 'Start the castle siege'],
			['/end_castle', '', 'End the siege'],
			['/set_castle_time', 'minutes', 'Set the minutes left in the siege'],
			['/set_castle_master', '[clan]', 'Give the castle to a clan (none to clear)'],
			['/get_castle_info', '', 'Show the castle owner clan, tax and your clan'],
			['/get_fountain_info', '', 'Show the Soul Fountain\'s potions and timers'],
			['/reset_fountain_time', '', 'Reset the Soul Fountain timers'],
			['/set_castle_tax', '', 'Apply the owner\'s chosen next tax and clan skill now']
		]) +
		'<p>Commands the game runs itself (in your browser):</p>' + table([
			['/field', 'number', 'Warp to a field (map) by its number'],
			['/player_location', 'x y z', 'Move to exact coordinates'],
			['/save', '', 'Save your character now'],
			['/expup', 'amount', 'Add experience on your side only (the server does not move with it; use with care)'],
			['/dark', 'n', 'Lighting, -255 (bright) to 255 (dark); 0 = normal'],
			['/color', 'r g b', 'Tint the world, each -109 to 109'],
			['/time_offset', 'n', 'Shift the game clock (day and night)'],
			['/talk', 'text', 'Make the selected monster say something (click it first)'],
			['/PassRankUp', '', 'Rank up one step without the quest'],
			['/LeeResetRankUp', 'rank', 'Set your rank back to that rank'],
			['/body', 'file', 'Change your body model'],
			['/mymodel', 'file', 'Change your whole model (levels 1&ndash;3 only)'],
			['/myhairmodel', 'n', 'Change your head'],
			['/face_path', 'file', 'Change your head by its file path'],
			['/ren', 'name', 'Rename a character loaded with /load'],
			['/LeeWhereIs', 'level', 'Change your level on your side only; prefer /MakeMeLevel'],
			['/LeeHelpMeSkill', '', 'Reset your skills and get the points back'],
			['/LeeHelpMeStat', '', 'Reset your stats and get the points back'],
			['/LeeResetQuest', 'level', 'Reset a level quest: 30, 55, 70, 80, 85, 90, 802 or 902'],
			['/Lee3thRankUp', '', 'Pass the 3rd rank-up quest'],
			['/Lee4thRankUp', 'step', 'Jump to a step of the 4th rank-up quest'],
			['/LeeForce', '', 'Refill your force orb\'s time'],
			['/LeeShotKeyReset', '', 'Clear your skill hotkeys'],
			['/PassMature', '', 'Unlock the old developer cheat keys. They change things on your side only, so saves can be refused: avoid'],
			['/FuckingInven!', '', 'Delete <b>everything</b> in your inventory and all your gold. No undo'],
			['/MrIm\'s100', '1 or 2', 'Level 100 quest: 1 = cancel it, 2 = jump to its last step'],
			['/MrIm\'sbeginner', 'any', 'Cancel the beginner quests in progress']
		]) +

		'<h3>Higher levels (server owner)</h3>' +
		'<p>Levels 2&ndash;4 need a password set in <code>server.ini</code> on the server (restart server.exe after editing):</p>' +
		'<pre>*ADMIN_NAME      GameMaster\n*ADMIN_COMMAND   password-for-level-2\n*ADMIN_COMMAND   password-for-level-3\n*ADMIN_COMMAND   password-for-level-4</pre>' +
		'<p>In game, with admin mode on, whisper the name and a password: <code>/:GameMaster password-for-level-4</code>. The reply says <i>Command Level (3)</i> for level 4 (the game counts from level 2 as 1).</p>' +
		'<p>Level 2:</p>' + table([
			['/where', 'name', 'Find where a player is'],
			['/near', 'name', 'Go to a player'],
			['/call', 'name', 'Bring a player to you'],
			['/kick', 'name', 'Disconnect a player'],
			['/kill', 'name', 'Kill a player\'s character'],
			['/open_warehouse', '[account [n]]', 'Open your warehouse anywhere, or another account\'s'],
			['/insert', 'account character', 'Add a character to an account'],
			['/shutdown', '', 'Shut the server down (players are saved)']
		]) +
		'<p>Level 3:</p>' + table([
			['/@get', 'item code', 'Drop an item at your feet, by its code in the item files'],
			['/@put', 'name item code', 'Give an item to a player'],
			['/AgingSetting', 'item code grade class code', 'Make an aged item'],
			['/get_money', '', 'Show the gold the server has on record for you'],
			['/load', 'name', 'Load a player\'s saved character into your game to look at (saving is off until you log in again)'],
			['/deload', 'name', 'Load a deleted character the same way'],
			['/baload', 'name', 'Load a character\'s backup the same way']
		]) +
		'<p>Level 4:</p>' + table([
			['/GiveMeMoney!', 'amount', 'Drop gold at your feet, up to 1,000,000 at a time'],
			['/GiveMeExp!', 'level', 'Set your experience to the start of that level'],
			['/MakeMeLevel', 'level', 'Go to that level with the right experience'],
			['/init_premiumitem', 'n', 'Reset premium items'],
			['/set_absorb', 'value', 'Set the selected monster\'s absorb (click it first)'],
			['/set_pcbang', 'code', 'Set the PC-room bonus'],
			['/easy_castle', '', 'Make the castle siege easy (testing)']
		]) +
		'</div>';
	document.body.appendChild(panel);

	function show(v) { panel.hidden = !v; if (v) document.getElementById('btnHelpClose').focus(); else btnHelp.focus(); }
	btnHelp.addEventListener('click', function () { show(true); });
	panel.addEventListener('click', function (e) { if (e.target === panel || e.target.id == 'btnHelpClose') show(false); });
	document.addEventListener('keydown', function (e) { if (e.key == 'Escape' && !panel.hidden) { e.stopPropagation(); show(false); } }, true);
	// mouse, touch and keys in the panel are for the panel, not the game (SDL listens on the whole page)
	['keydown', 'keyup', 'keypress', 'mousedown', 'mouseup', 'mousemove', 'click', 'dblclick', 'wheel', 'contextmenu',
	 'pointerdown', 'pointerup', 'pointermove', 'touchstart', 'touchmove', 'touchend'].forEach(function (t) {
		panel.addEventListener(t, function (e) { e.stopPropagation(); });
	});
})();
