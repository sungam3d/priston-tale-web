// settings.js - the "Settings" panel under the game: image quality, shadows, draw
// distance, music and sound effects. Loaded by the page (web\shell.html) after
// Module is defined and before admin.js; copied next to it by web\make_site.py.
// Kept in this browser (localStorage 'pt-settings').
//
// Live settings go through Module.ptOpts, which the game reads once per frame when
// its version (v) changes (Winmain.cpp PtWeb_PollOptions): music / effects volume
// (ptsound master volumes) and view (draw distance in map cells). Shadow strength
// is Module.ptShadowDark (read every frame by ptgfx_shadow.cpp).
// Restart settings are read when the game starts: Module.ptShadows, and the image
// quality lines in ptReg.rgx ("ColorBPP" 32 = 32-bit colour, "Graphic" 0 = full
// size textures; smReg.cpp / fileread.cpp).
//
// Module.ptFilePatches: functions that change game files after the start-up pack
// is unpacked and before the game reads them (also used by admin.js).
(function () {
	if (!window.Module) return;
	var KEY = 'pt-settings';
	var DEF = { quality: 'max', shadows: true, shadowDark: 0.6, view: 38, music: 70, musicOn: true, effects: 80, effectsOn: true };
	var VIEW_MIN = 16, VIEW_GAME = 38, VIEW_MAX = 110;		// map cells (64 world units); 110 = unlimited
	var s = {};
	try { s = JSON.parse(localStorage.getItem(KEY) || '{}') || {}; } catch (e) {}
	for (var k in DEF) if (!(k in s)) s[k] = DEF[k];
	function save() { try { localStorage.setItem(KEY, JSON.stringify(s)); } catch (e) {} }

	// ---- to the game ----
	var version = 1;
	function vol(v, on) { return on ? Math.pow(v/100, 2) : 0; }		// slider 0-100, felt as linear
	function pushLive() {
		Module.ptOpts = { v: ++version, music: vol(s.music, s.musicOn), effects: vol(s.effects, s.effectsOn),
			view: s.view==VIEW_GAME ? 0 : s.view };
		Module.ptShadowDark = s.shadowDark;
	}
	Module.ptShadows = !!s.shadows;
	pushLive();

	// ---- game file patches (before the game reads them) ----
	Module.ptFilePatches = Module.ptFilePatches || [];
	function readText(path) {			// bytes as characters 0-255 (the game's files are cp949)
		var b;
		try { b = FS.readFile(path); } catch (e) { return ''; }
		var t = '';
		for (var i = 0; i < b.length; i += 4096) t += String.fromCharCode.apply(null, b.subarray(i, i + 4096));
		return t;
	}
	function writeText(path, text) {
		var b = new Uint8Array(text.length);
		for (var i = 0; i < text.length; i++) b[i] = text.charCodeAt(i) & 0xFF;
		FS.writeFile(path, b);
	}
	Module.ptFilePatches.push(function () {
		// image quality in ptReg.rgx (the game may have saved it: rewrite our lines each start)
		var path = '/game/ptReg.rgx';
		var lines = readText(path).split(/\r?\n/).filter(function (l) { return l && !/^"(ColorBPP|Graphic)"/i.test(l); });
		if (s.quality == 'max') lines.push('"ColorBPP" "32"', '"Graphic" "0"');
		writeText(path, lines.join('\r\n') + '\r\n');
	});
	// hotuk.ini and ptReg.rgx arrive in the start-up pack, which web\pre.js unpacks in
	// its own preRun; when done it writes /game_saves_restored.txt. Hold the start
	// until then (only this dependency left), patch, and let the game start.
	Module.preRun = Module.preRun || [];
	Module.preRun.unshift(function () {
		var done = false;
		var orig = Module.monitorRunDependencies;
		Module.monitorRunDependencies = function (left) {
			if (orig) orig.call(Module, left);
			if (left != 1 || done) return;
			try { if (!FS.analyzePath('/game_saves_restored.txt').exists) return; } catch (e) { return; }
			done = true;
			Module.ptFilePatches.forEach(function (f) {
				try { f(); } catch (e) { Module.printErr('[web] setting not applied: ' + e); }
			});
			removeRunDependency('pt-files');
		};
		addRunDependency('pt-files');
	});

	// ---- the panel ----
	var css = document.createElement('style');
	css.textContent =
		'.pt-set{position:fixed;inset:0;background:rgba(5,5,8,.6);display:flex;align-items:flex-start;justify-content:center;z-index:50;overflow:auto;padding:24px 16px}' +
		'.pt-set[hidden]{display:none}' +
		'.pt-set-box{background:#15151c;border:1px solid #3a3426;border-radius:8px;color:#d8d4c8;max-width:520px;width:100%;padding:18px 22px 20px;font:13.5px/1.5 system-ui,sans-serif;box-sizing:border-box}' +
		'.pt-set-head{display:flex;justify-content:space-between;align-items:center;gap:12px;margin-bottom:4px}' +
		'.pt-set-box h2{font:600 19px/1.3 Georgia,"Times New Roman",serif;color:#e8cf8e;margin:0}' +
		'.pt-set-box h3{font:600 12px/1.3 system-ui,sans-serif;letter-spacing:.08em;text-transform:uppercase;color:#a8915c;margin:18px 0 8px}' +
		'.pt-row{display:grid;grid-template-columns:150px 1fr;align-items:center;gap:6px 14px;padding:6px 0;border-top:1px solid #23222b}' +
		'.pt-row label{color:#d8d4c8}' +
		'.pt-row .ctl{display:flex;align-items:center;gap:10px;min-width:0}' +
		'.pt-row input[type=range]{flex:1;min-width:0;accent-color:#c9a14d}' +
		'.pt-row .val{color:#a8a294;font-variant-numeric:tabular-nums;min-width:72px;text-align:right;font-size:12.5px}' +
		'.pt-seg{display:inline-flex;border:1px solid #34323c;border-radius:5px;overflow:hidden}' +
		'.pt-seg button{background:#1d1c24;color:#b8b2a4;border:0;padding:4px 12px;font:inherit;cursor:pointer}' +
		'.pt-seg button+button{border-left:1px solid #34323c}' +
		'.pt-seg button[aria-pressed=true]{background:#3a3020;color:#f0d896}' +
		'.pt-set-box .note{color:#8a8476;font-size:12px;margin:2px 0 0}' +
		'.pt-set-box .restart{margin-top:14px;display:flex;align-items:center;gap:12px;color:#e8cf8e}' +
		'.pt-set-box .restart[hidden]{display:none}' +
		'.pt-set-box button.act{background:#22212a;color:#d8d4c8;border:1px solid #34323c;border-radius:4px;padding:4px 12px;font:inherit;cursor:pointer}' +
		'.pt-set-box button.act:hover{border-color:#5a5566}' +
		'.pt-set-box button.primary{background:#3a3020;border-color:#7a6230;color:#f0d896}' +
		'@media (max-width:480px){.pt-row{grid-template-columns:1fr}}';
	document.head.appendChild(css);

	var bar = document.getElementById('bar');
	if (!bar) return;
	var btn = document.createElement('button');
	btn.type = 'button';
	btn.id = 'btnSettings';
	btn.textContent = 'Settings';
	btn.title = 'Graphics, draw distance and sound';
	bar.insertBefore(btn, document.getElementById('saveInfo'));

	var panel = document.createElement('div');
	panel.className = 'pt-set';
	panel.hidden = true;
	panel.setAttribute('role', 'dialog');
	panel.setAttribute('aria-label', 'Settings');
	panel.innerHTML =
		'<div class="pt-set-box">' +
		'<div class="pt-set-head"><h2>Settings</h2><button type="button" class="act" id="setClose">Close</button></div>' +

		'<h3>Graphics</h3>' +
		'<div class="pt-row"><label>Image quality</label><div class="ctl"><span class="pt-seg" id="setQuality">' +
			'<button type="button" data-v="standard">Standard</button><button type="button" data-v="max">Max</button></span></div></div>' +
		'<p class="note">Max: 32-bit colour (no banding) and full-size textures. Standard: the original 16-bit colour.</p>' +
		'<div class="pt-row"><label>Shadows</label><div class="ctl"><span class="pt-seg" id="setShadows">' +
			'<button type="button" data-v="0">Round</button><button type="button" data-v="1">Real-time</button></span></div></div>' +
		'<div class="pt-row"><label for="setShadowDark">Shadow darkness</label><div class="ctl">' +
			'<input type="range" id="setShadowDark" min="20" max="90" step="5"><span class="val" id="setShadowDarkV"></span></div></div>' +
		'<div class="pt-row"><label for="setView">Draw distance</label><div class="ctl">' +
			'<input type="range" id="setView" min="' + VIEW_MIN + '" max="' + VIEW_MAX + '" step="2"><span class="val" id="setViewV"></span></div></div>' +
		'<p class="note">Further is slower: the game draws everything on the CPU. Changes as you drag.</p>' +

		'<h3>Sound</h3>' +
		'<div class="pt-row"><label for="setMusic">Music</label><div class="ctl">' +
			'<input type="checkbox" id="setMusicOn" aria-label="Music on"><input type="range" id="setMusic" min="0" max="100" step="5"><span class="val" id="setMusicV"></span></div></div>' +
		'<div class="pt-row"><label for="setEffects">Sound effects</label><div class="ctl">' +
			'<input type="checkbox" id="setEffectsOn" aria-label="Sound effects on"><input type="range" id="setEffects" min="0" max="100" step="5"><span class="val" id="setEffectsV"></span></div></div>' +

		'<div class="restart" id="setRestart" hidden><span>Restart the game to apply image quality / shadows.</span>' +
			'<button type="button" class="act primary" id="setRestartBtn">Restart now</button></div>' +
		'</div>';
	document.body.appendChild(panel);

	function $(id) { return document.getElementById(id); }
	var start = { quality: s.quality, shadows: s.shadows };
	function viewText(v) {
		if (v >= VIEW_MAX) return 'Unlimited';
		if (v == VIEW_GAME) return 'Normal';
		return (v < VIEW_GAME ? (v <= 24 ? 'Close' : 'Near') : (v < 70 ? 'Far' : 'Very far'));
	}
	function seg(id, val) {
		Array.prototype.forEach.call($(id).querySelectorAll('button'), function (b) { b.setAttribute('aria-pressed', b.dataset.v == String(val)); });
	}
	function render() {
		seg('setQuality', s.quality);
		seg('setShadows', s.shadows ? 1 : 0);
		$('setShadowDark').value = Math.round(s.shadowDark*100);
		$('setShadowDarkV').textContent = Math.round(s.shadowDark*100) + '%';
		$('setShadowDark').disabled = !s.shadows;
		$('setView').value = s.view;
		$('setViewV').textContent = viewText(s.view);
		$('setMusicOn').checked = s.musicOn;
		$('setMusic').value = s.music; $('setMusic').disabled = !s.musicOn;
		$('setMusicV').textContent = s.musicOn ? s.music + '%' : 'Off';
		$('setEffectsOn').checked = s.effectsOn;
		$('setEffects').value = s.effects; $('setEffects').disabled = !s.effectsOn;
		$('setEffectsV').textContent = s.effectsOn ? s.effects + '%' : 'Off';
		$('setRestart').hidden = s.quality == start.quality && s.shadows == start.shadows;
	}
	function change() { save(); pushLive(); render(); }

	$('setQuality').addEventListener('click', function (e) { var v = e.target.dataset && e.target.dataset.v; if (v) { s.quality = v; change(); } });
	$('setShadows').addEventListener('click', function (e) { var v = e.target.dataset && e.target.dataset.v; if (v) { s.shadows = v == '1'; change(); } });
	$('setShadowDark').addEventListener('input', function () { s.shadowDark = this.value/100; change(); });
	$('setView').addEventListener('input', function () { s.view = +this.value; change(); });
	$('setMusicOn').addEventListener('change', function () { s.musicOn = this.checked; change(); });
	$('setMusic').addEventListener('input', function () { s.music = +this.value; change(); });
	$('setEffectsOn').addEventListener('change', function () { s.effectsOn = this.checked; change(); });
	$('setEffects').addEventListener('input', function () { s.effects = +this.value; change(); });
	$('setRestartBtn').addEventListener('click', function () { this.disabled = true; location.reload(); });

	function show(v) { panel.hidden = !v; if (v) { render(); $('setClose').focus(); } else btn.focus(); }
	btn.addEventListener('click', function () { show(true); });
	panel.addEventListener('click', function (e) { if (e.target === panel || e.target.id == 'setClose') show(false); });
	document.addEventListener('keydown', function (e) { if (e.key == 'Escape' && !panel.hidden) { e.stopPropagation(); show(false); } }, true);
	// keys typed in the panel are for the panel, not the game
	panel.addEventListener('keydown', function (e) { e.stopPropagation(); });
})();
