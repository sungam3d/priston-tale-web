// display.js - how the game's picture fits the browser window: windowed/fullscreen and the
// scaling mode (fit / fill / stretch / integer). Loaded by the page (web\shell.html) right
// after progress.js, which builds the ".pt-stage" box around the canvas; this file owns that
// box from here on.
//
// The engine's own backbuffer size (what the game actually renders at, "*SCREENSIZE" in
// hotuk.ini) is a separate concern, picked in the Settings panel (web\settings.js) and
// applied by writing hotuk.ini before the game starts - it takes a restart, like the other
// start-up settings there. This file never changes that: it only decides how the resulting
// canvas - whatever size it ends up - is shown on screen, and that applies at once, no
// restart. The two meet at the canvas's own width/height (the backbuffer, read live here so
// this always follows whatever the engine actually produced).
//
// Scaling is done by setting the CANVAS's own CSS width/height to an exact pixel size (never
// with CSS "object-fit"): the stage (its parent) is sized to the viewport box and shows
// letterbox/pillarbox bars as its own black background around a smaller canvas. Keeping the
// canvas itself always scaled uniformly (or stretched, axis-independent) like this, with no
// letterboxing *inside* the canvas element, matters for more than looks: Emscripten's SDL2
// maps mouse/touch positions from the canvas's own on-screen rect to its backing pixels with
// a plain per-axis scale (rect -> canvas.width/height), and that only lands correctly when
// the canvas's rect has no "dead" letterboxed margin of its own. ("object-fit: contain"
// would look identical here but silently break clicks in the letterboxed area.)
(function () {
	if (!window.Module) return;
	var canvas = document.getElementById('canvas');
	var stage = document.querySelector('.pt-stage');
	if (!canvas || !stage) return;			// progress.js didn't run (no Module?): nothing to manage

	var KEY = 'pt-display';
	var MODES = ['fit', 'fill', 'stretch', 'integer'];
	var ds = {};
	try { ds = JSON.parse(localStorage.getItem(KEY) || '{}') || {}; } catch (e) {}
	// default "fill": the engine's own resolution stays the safe 1024x768 it has always used
	// (see settings.js) - its HUD and floating name/HP labels were pixel-positioned for a 4:3
	// screen and come apart at a wide one - so a wide monitor is better served by blowing that
	// whole picture, UI included, up to cover the window (cropped top/bottom) than by rendering
	// at the monitor's own wide shape: no black bars, and the UI reads bigger instead of tiny
	// in a sea of black, at zero risk to the game itself. "Fit" (unscaled, letterboxed) is still
	// one click away for anyone who would rather see the whole picture with bars than lose edges.
	if (MODES.indexOf(ds.scaleMode) < 0) ds.scaleMode = 'fill';
	function saveDs() { try { localStorage.setItem(KEY, JSON.stringify(ds)); } catch (e) {} }

	// ---- the stage: a fixed-size box, letterbox bars are its own black background ----
	stage.style.display = 'flex';
	stage.style.alignItems = 'center';
	stage.style.justifyContent = 'center';
	stage.style.overflow = 'hidden';		// "fill" mode: the canvas can be larger than the box
	stage.style.background = '#000';
	stage.style.margin = '0 auto';
	stage.style.boxSizing = 'border-box';

	function isFullscreen() { return !!(document.fullscreenElement || document.webkitFullscreenElement); }

	// the box the canvas scales into: the whole screen in fullscreen; otherwise the full browser
	// window (less the page's own 12px side padding and, below, room for the status/button bar
	// and the log panel, ~260 px) - no extra cap, so windowed play actually fills the window
	// instead of sitting in a small box in the middle of it
	function viewportBox() {
		if (isFullscreen()) return { w: window.innerWidth, h: window.innerHeight };
		var w = Math.max(320, window.innerWidth - 24);
		var h = Math.max(320, window.innerHeight - 260);
		return { w: w, h: h };
	}

	function layout() {
		var box = viewportBox();
		stage.style.width = box.w + 'px';
		stage.style.height = box.h + 'px';
		// the engine's actual backbuffer size (hotuk.ini *SCREENSIZE once the game starts;
		// the HTML's own placeholder, 1024x768, before that)
		var cw = canvas.width || 1024, ch = canvas.height || 768;
		var sw, sh;
		switch (ds.scaleMode) {
		case 'stretch':
			sw = box.w; sh = box.h;
			break;
		case 'fill': {
			var s = Math.max(box.w / cw, box.h / ch);
			sw = cw * s; sh = ch * s;
			break;
		}
		case 'integer': {
			var s = Math.max(1, Math.floor(Math.min(box.w / cw, box.h / ch)));
			sw = cw * s; sh = ch * s;
			break;
		}
		default:		// 'fit': the whole picture, letterboxed/pillarboxed if the shape differs
			var s2 = Math.min(box.w / cw, box.h / ch);
			sw = cw * s2; sh = ch * s2;
		}
		canvas.style.width = Math.round(sw) + 'px';
		canvas.style.height = Math.round(sh) + 'px';
	}
	layout();
	window.addEventListener('resize', layout);
	window.addEventListener('orientationchange', layout);
	// the engine sets canvas.width/height once it starts (hotuk.ini's resolution): re-fit then
	if (window.MutationObserver) new MutationObserver(layout).observe(canvas, { attributes: true, attributeFilter: ['width', 'height'] });

	// ---- fullscreen ----
	function requestFS(el) {
		var f = el.requestFullscreen || el.webkitRequestFullscreen;
		return f ? f.call(el) : Promise.reject(new Error('Fullscreen API not available'));
	}
	function exitFS() {
		var f = document.exitFullscreen || document.webkitExitFullscreen;
		return f ? f.call(document) : Promise.reject(new Error('Fullscreen API not available'));
	}
	function toggleFullscreen() {
		if (isFullscreen()) exitFS().catch(function () {});
		else requestFS(stage).catch(function (e) { Module.printErr && Module.printErr('[web] fullscreen: ' + (e.message || e)); });
	}
	var fsBtn = null;
	function updateFsBtn() { if (fsBtn) fsBtn.textContent = isFullscreen() ? 'Exit fullscreen' : 'Fullscreen'; }
	['fullscreenchange', 'webkitfullscreenchange'].forEach(function (t) {
		document.addEventListener(t, function () {
			layout();
			updateFsBtn();
			// best effort only: most phones ignore this outside a user gesture, or don't have it at all
			try {
				if (isFullscreen() && screen.orientation && screen.orientation.lock) screen.orientation.lock('landscape').catch(function () {});
				else if (!isFullscreen() && screen.orientation && screen.orientation.unlock) screen.orientation.unlock();
			} catch (e) {}
		});
	});
	// F11: browsers often keep this for their own (whole-browser-chrome) fullscreen and never
	// hand the keydown to the page, so the on-screen button is the one that reliably works
	window.addEventListener('keydown', function (e) {
		if (e.key == 'F11') { e.preventDefault(); toggleFullscreen(); }
	}, true);

	var bar = document.getElementById('bar');
	if (bar) {
		fsBtn = document.createElement('button');
		fsBtn.type = 'button';
		fsBtn.title = 'Fullscreen the game (F11 in some browsers)';
		updateFsBtn();
		fsBtn.addEventListener('click', toggleFullscreen);
		bar.insertBefore(fsBtn, document.getElementById('saveInfo'));
	}

	// ---- the Settings panel's "Scaling" control reaches the manager through here ----
	window.ptDisplay = {
		getScaleMode: function () { return ds.scaleMode; },
		setScaleMode: function (m) { if (MODES.indexOf(m) < 0) return; ds.scaleMode = m; saveDs(); layout(); },
		isFullscreen: isFullscreen,
		toggleFullscreen: toggleFullscreen
	};
})();
