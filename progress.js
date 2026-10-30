// progress.js - shows what the game is loading, over the game picture, so a long
// download never looks like a frozen game. Loaded by the page (web\shell.html)
// after Module is defined; copied next to it by web\make_site.py.
//  - Start-up: a panel with a bar - game program, data pack (web\pre.js reports it
//    through Module.setStatus), unpacking.
//  - While playing: a small bar at the bottom while the game downloads files it
//    needs (maps, monsters, items). The service worker (web\sw.js) announces those
//    downloads on BroadcastChannel 'pt-downloads', also the ones from the game's
//    threads. Without the service worker only the start-up panel is shown.
(function () {
	var canvas = document.getElementById('canvas');
	if (!canvas || !window.Module) return;

	var css = document.createElement('style');
	css.textContent =
		'.pt-stage{position:relative;max-width:100%}' +
		'.pt-load{position:absolute;left:50%;transform:translateX(-50%);box-sizing:border-box;' +
			'background:rgba(11,11,16,.88);border:1px solid #3a3426;border-radius:8px;color:#d8d4c8;' +
			'font:13px/1.4 system-ui,sans-serif;pointer-events:none;display:flex;flex-direction:column;gap:8px}' +
		'.pt-load[hidden]{display:none}' +
		'.pt-load.big{top:50%;transform:translate(-50%,-50%);width:min(440px,86%);padding:18px 20px}' +
		'.pt-load.small{bottom:12px;width:min(360px,80%);padding:8px 12px;gap:5px;font-size:12px}' +
		'.pt-load b{font:600 15px/1.3 Georgia,"Times New Roman",serif;color:#e8cf8e;letter-spacing:.02em}' +
		'.pt-load.small b{font-size:12.5px}' +
		'.pt-bar{height:6px;border-radius:3px;background:#2a2620;overflow:hidden;position:relative}' +
		'.pt-load.small .pt-bar{height:4px}' +
		'.pt-bar i{position:absolute;left:0;top:0;bottom:0;background:linear-gradient(90deg,#9a7430,#e2bb62);border-radius:3px;transition:width .2s}' +
		'.pt-bar.busy i{width:30%!important;animation:pt-busy 1.2s ease-in-out infinite}' +
		'@keyframes pt-busy{0%{left:-30%}100%{left:100%}}' +
		'@media (prefers-reduced-motion:reduce){.pt-bar.busy i{animation:none;width:100%!important;opacity:.5}.pt-bar i{transition:none}}' +
		'.pt-load .d{color:#a8a294;font-variant-numeric:tabular-nums;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}' +
		'.pt-load .n{color:#807a6c;font-size:12px}';
	document.head.appendChild(css);

	// the panel sits over the canvas: wrap the canvas in a positioned box
	var stage = document.createElement('div');
	stage.className = 'pt-stage';
	canvas.parentNode.insertBefore(stage, canvas);
	stage.appendChild(canvas);
	var box = document.createElement('div');
	box.className = 'pt-load big';
	box.setAttribute('role', 'status');
	box.setAttribute('aria-live', 'polite');
	box.innerHTML = '<b></b><div class="pt-bar"><i></i></div><div class="d"></div><div class="n"></div>';
	stage.appendChild(box);
	var elTitle = box.children[0], elBar = box.children[1], elFill = elBar.firstChild,
		elDetail = box.children[2], elNote = box.children[3];

	function mb(n) { return n >= 1e7 ? Math.round(n / 1e6) + ' MB' : (n / 1e6).toFixed(1) + ' MB'; }
	function show(size, title, frac, detail, note) {
		box.hidden = false;
		box.className = 'pt-load ' + size;
		elTitle.textContent = title;
		elBar.classList.toggle('busy', frac == null);
		if (frac != null) elFill.style.width = Math.max(2, Math.min(100, frac * 100)) + '%';
		elDetail.textContent = detail || '';
		elDetail.hidden = !detail;
		elNote.textContent = note || '';
		elNote.hidden = !note;
	}

	// ---- start-up: from Module.setStatus (web\pre.js, Emscripten) ----
	var starting = true, startText = 'Loading...', pack = null;	// pack: download seen by the service worker
	function drawStart() {
		if (!starting) return;
		var m = /Downloading game data\.\.\. (\d+)%/.exec(startText), m2 = /Downloading game data\.\.\. (\d+) MB/.exec(startText);
		if (m || m2) {
			var frac = m ? +m[1] / 100 : null, detail = m ? m[1] + '%' : m2[1] + ' MB';
			// web\pre.js counts the pack as it reads it; the service worker's messages about it
			// can arrive late (the page is busy reading), so the page's own figures come first
			var own = Module.ptPack;
			if (own && own.size) pack = { size: own.size, got: own.got, t0: own.t0 };
			if (pack && pack.size) {
				frac = pack.got / pack.size;
				detail = mb(pack.got) + ' of ' + mb(pack.size) + speedText(pack);
			}
			if (frac != null && frac >= 1) show('big', 'Unpacking game data', null, 'Getting the files ready...', '');
			else show('big', 'Downloading game data', frac, detail,
				pack ? 'Only the first visit (and updates) download this. Later visits load it from your browser.' : '');
		}
		else if (/^Preparing/.test(startText)) show('big', 'Starting Priston Tale', null, 'Getting the game ready...', '');
		else if (/^Error/.test(startText)) show('big', 'Something went wrong', 0, startText.replace(/^Error: /, ''), 'Reload the page to try again.');
		else show('big', 'Starting Priston Tale', null, 'Loading the game program...', '');
	}
	function speedText(d) {
		var secs = (Date.now() - d.t0) / 1000;
		if (secs < 1.5 || !d.got) return '';
		var rate = d.got / secs, left = d.size ? (d.size - d.got) / rate : 0;
		return ' · ' + mb(rate) + '/s' + (left > 2 ? ' · about ' + (left < 90 ? Math.round(left) + ' s' : Math.round(left / 60) + ' min') + ' left' : '');
	}
	// a crash: the full error and where in the game it happened, into the log under
	// the game (copy it from there to report it)
	function logError(what, err) {
		var text = err && err.stack ? err.stack : String(err && err.message || err);
		if (Module.printErr) Module.printErr('[web] ' + what + ': ' + text);
	}
	window.addEventListener('error', function (e) { logError('error', e.error || e.message); });
	window.addEventListener('unhandledrejection', function (e) { logError('error (promise)', e.reason); });

	var origSetStatus = Module.setStatus;
	Module.setStatus = function (t) {
		if (origSetStatus) origSetStatus.call(Module, t);
		if (!starting) return;
		if (!t || t == 'Running') { starting = false; box.hidden = true; drawPlay(); return; }
		startText = t;
		drawStart();
	};
	drawStart();

	// ---- while playing: downloads announced by the service worker ----
	// A batch (the textures of a map or model, web\sw.js) is announced with its file
	// count and size before its downloads start, so the bar knows the whole amount.
	var active = {}, burst = null, hideTimer = 0, showTimer = 0;
	function newBurst() { return { done: 0, total: 0, left: 0 }; }
	function drawPlay() {
		if (starting || !burst) return;
		var ids = Object.keys(active);
		if (!ids.length) return;
		var got = burst.done, total = burst.total, name = '';
		ids.forEach(function (id) { got += Math.min(active[id].got, active[id].size || active[id].got); name = active[id].rel; });
		if (burst.left > 0)
			show('small', 'Loading the area', total ? got / total : null,
				mb(got) + ' of ' + mb(total) + ' · ' + burst.left + ' file' + (burst.left > 1 ? 's' : '') + ' left', '');
		else
			show('small', 'Downloading game files', total ? got / total : null,
				name.split('/').pop() + ' · ' + mb(got) + ' of ' + mb(total) + (ids.length > 1 ? ' · ' + ids.length + ' files at once' : ''), '');
	}
	function later() {			// quick downloads come and go without a flash: show after 300 ms
		if (!box.hidden) drawPlay();
		else if (!showTimer) showTimer = setTimeout(function () { showTimer = 0; drawPlay(); }, 300);
	}
	function idle() {
		clearTimeout(showTimer);
		showTimer = 0;
		clearTimeout(hideTimer);
		hideTimer = setTimeout(function () { if (!Object.keys(active).length) { burst = null; if (!starting) box.hidden = true; } }, 600);
	}
	if (window.BroadcastChannel) {
		var channel = new BroadcastChannel('pt-downloads');
		channel.onmessage = function (e) {
			var m = e.data;
			if (!m || !m.t) return;
			if (m.rel == 'data_startup.pack') {			// the start-up panel shows this one
				if (m.t == 'start') pack = { size: m.size, got: 0, t0: Date.now() };
				else if (m.t == 'bytes' && pack) pack.got = m.got;
				else if (m.t == 'end' && pack) pack.got = pack.size || pack.got;
				drawStart();
				return;
			}
			if (m.t == 'batch') {
				clearTimeout(hideTimer);
				if (!burst) burst = newBurst();
				burst.total += m.size;
				burst.left += m.files;
				later();
			}
			else if (m.t == 'start') {
				clearTimeout(hideTimer);
				if (!burst) burst = newBurst();
				active[m.id] = { rel: m.rel, size: m.size, got: 0, seen: Date.now(), batch: m.batch };
				if (!m.batch) burst.total += m.size;		// a batch's files are counted already
				later();
			}
			else if (active[m.id]) {
				active[m.id].seen = Date.now();
				if (m.t == 'bytes') { active[m.id].got = m.got; if (!showTimer) drawPlay(); }
				else if (m.t == 'end') {
					burst.done += active[m.id].size || active[m.id].got;
					if (active[m.id].batch && burst.left > 0) burst.left--;
					delete active[m.id];
					if (Object.keys(active).length) { if (!showTimer) drawPlay(); } else idle();
				}
			}
		};
		// a download that failed half-way sends no 'end': forget it after 30 s of silence
		setInterval(function () {
			var now = Date.now(), changed = false;
			for (var id in active) if (now - active[id].seen > 30000) { delete active[id]; changed = true; }
			if (changed && !Object.keys(active).length) idle();
			if (pack && starting) drawStart();			// keeps speed and time left moving
		}, 1000);
	}
})();
