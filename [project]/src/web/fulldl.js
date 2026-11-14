// fulldl.js - how the game's files arrive, chosen before the game starts:
//  - "Play now": the game downloads each file the first time it needs it (as before).
//  - "Download the full game": every file in data_manifest.txt is downloaded once, then a
//    visit downloads only the files whose hash changed on the web server.
// The files go through the service worker (web\sw.js), which keeps them in Cache Storage
// ('pt-data-1') and serves them to the game; this page only asks for the missing ones
// (header X-PT-Full: the worker then keeps them off the in-game download bar).
// What is stored is noted in a record (path -> hash) kept in the same cache, so a visit
// knows what changed without opening 27,000 stored files.
// Loaded by the page (web\shell.html) after progress.js; the page starts the game once
// window.ptDownloadGate() resolves. The choice is kept in localStorage and can be changed
// with the Downloads button under the game.
(function () {
	var MODE_KEY = 'ptDownloadMode';				// 'stream' | 'full'
	var CONC_KEY = 'ptDownloadConcurrency';		// how many files at once (many are tiny: more can be faster)
	var CONC_OPTS = [4, 8, 16, 24, 32];
	var CONC_DEFAULT = 8;
	var CACHE = 'pt-data-1';						// web\sw.js CACHE
	var RECORD_URL = new URL('__pt_full_record', location.href).href;
	var DATA_BASE = new URL('data/', location.href).href;
	var sw = navigator.serviceWorker;
	var canFull = !!(sw && window.caches && window.fetch);

	function getMode() { try { return localStorage.getItem(MODE_KEY) || ''; } catch (e) { return ''; } }
	function setMode(m) { try { localStorage.setItem(MODE_KEY, m); } catch (e) {} }
	function getConcurrency() {
		try { var n = +localStorage.getItem(CONC_KEY); return CONC_OPTS.indexOf(n) >= 0 ? n : CONC_DEFAULT; }
		catch (e) { return CONC_DEFAULT; }
	}
	function setConcurrency(n) { try { localStorage.setItem(CONC_KEY, String(n)); } catch (e) {} }

	function gb(n) { return n >= 1e9 ? (n / 1e9).toFixed(1) + ' GB' : n >= 1e7 ? Math.round(n / 1e6) + ' MB' : (n / 1e6).toFixed(1) + ' MB'; }
	function num(n) { return n.toLocaleString('en'); }
	function dataUrl(rel) { return DATA_BASE + rel.split('/').map(encodeURIComponent).join('/'); }

	// ---- the panel (over the game picture, like progress.js) ----
	var css = document.createElement('style');
	css.textContent =
		'.pt-dl{position:absolute;inset:0;z-index:6;display:flex;align-items:center;justify-content:center;' +
			'background:rgba(8,8,12,.9);font:13px/1.45 system-ui,sans-serif;color:#d8d4c8}' +
		'.pt-dl[hidden]{display:none}' +
		'.pt-dl .card{width:min(520px,92%);box-sizing:border-box;background:#121218;border:1px solid #3a3426;border-radius:10px;' +
			'padding:20px 22px;display:flex;flex-direction:column;gap:12px}' +
		'.pt-dl h2{margin:0;font:600 17px/1.3 Georgia,"Times New Roman",serif;color:#e8cf8e;letter-spacing:.02em}' +
		'.pt-dl .opt{display:block;width:100%;text-align:left;background:#1b1a21;color:#d8d4c8;border:1px solid #34323c;' +
			'border-radius:8px;padding:12px 14px;cursor:pointer;font:inherit}' +
		'.pt-dl .opt:hover,.pt-dl .opt:focus-visible{border-color:#9a7430;outline:none}' +
		'.pt-dl .opt b{display:block;color:#e8cf8e;font-size:14px;margin-bottom:2px}' +
		'.pt-dl .opt span{color:#a8a294}' +
		'.pt-dl .opt.cur{border-color:#6b5a32}' +
		'.pt-dl .n{color:#807a6c;font-size:12px}' +
		'.pt-dl .conc{display:flex;align-items:center;gap:8px;flex-wrap:wrap;color:#a8a294;font-size:12px}' +
		'.pt-dl .conc select{background:#1b1a21;color:#d8d4c8;border:1px solid #34323c;border-radius:4px;padding:3px 8px;font:inherit}' +
		'.pt-dl .warn{color:#e0a060;font-size:12px}' +
		'.pt-dl .bar{height:6px;border-radius:3px;background:#2a2620;overflow:hidden;position:relative}' +
		'.pt-dl .bar i{position:absolute;left:0;top:0;bottom:0;width:0;background:linear-gradient(90deg,#9a7430,#e2bb62);border-radius:3px;transition:width .3s}' +
		'.pt-dl .d{color:#a8a294;font-variant-numeric:tabular-nums}' +
		'.pt-dl .row{display:flex;gap:8px;justify-content:flex-end;flex-wrap:wrap}' +
		'.pt-dl .btn{background:#2a2620;color:#e8cf8e;border:1px solid #9a7430;border-radius:4px;padding:5px 12px;cursor:pointer;font:inherit}' +
		'.pt-dl .btn.q{background:none;color:#a8a294;border-color:#34323c}' +
		'@media (prefers-reduced-motion:reduce){.pt-dl .bar i{transition:none}}';
	document.head.appendChild(css);

	var panel = document.createElement('div');
	panel.className = 'pt-dl';
	panel.hidden = true;
	panel.innerHTML = '<div class="card" role="dialog" aria-modal="true" aria-labelledby="pt-dl-t"></div>';
	var card = panel.firstChild;
	function mountPanel() {
		var stage = document.querySelector('.pt-stage') || document.getElementById('canvas').parentNode;
		if (panel.parentNode != stage) stage.appendChild(panel);
	}

	// ---- the manifest and what is stored ----
	var plan = null;				// { entries, total, todo, todoBytes, record }
	function parseManifest(text) {
		var out = [];
		text.split('\n').forEach(function (l) {
			var a = l.split('\t');
			if (a.length > 2) out.push({ rel: a[0], size: +a[1], hash: a[2].trim() });
		});
		return out;
	}
	function loadRecord(cache) {
		return cache.match(RECORD_URL).then(function (r) { return r ? r.json() : []; })
			.then(function (a) { return new Map(a); }, function () { return new Map(); });
	}
	var recordDirty = false, recordSaving = null;
	function saveRecord() {
		if (!plan || !recordDirty || recordSaving) return recordSaving || Promise.resolve();
		recordDirty = false;
		var body = JSON.stringify(Array.from(plan.record.entries()));
		recordSaving = caches.open(CACHE).then(function (c) {
			return c.put(RECORD_URL, new Response(body, { headers: { 'Content-Type': 'application/json' } }));
		}).catch(function () {}).then(function () { recordSaving = null; });
		return recordSaving;
	}
	async function makePlan() {
		var res = await fetch('data_manifest.txt');		// through the worker: fresh when it changed
		if (!res.ok) throw new Error('data_manifest.txt: HTTP ' + res.status);
		var entries = parseManifest(await res.text());
		var record = await loadRecord(await caches.open(CACHE));
		// Not listed with this hash: new, changed, or not asked for yet. One the game already
		// downloaded while playing is answered by the worker from its stored copy (no network).
		// (cache.keys() is no help here: with this many files it fails - "Operation too large".)
		var todo = [], total = 0;
		entries.forEach(function (e) {
			total += e.size;
			if (record.get(e.rel) !== e.hash) todo.push(e);
		});
		var names = new Set(entries.map(function (e) { return e.rel; }));
		record.forEach(function (v, k) { if (!names.has(k)) { record.delete(k); recordDirty = true; } });
		var todoBytes = 0;
		todo.forEach(function (e) { todoBytes += e.size; });
		plan = { entries: entries, total: total, todo: todo, todoBytes: todoBytes, record: record };
		saveRecord();
		return plan;
	}
	function storageLeft() {
		if (!navigator.storage || !navigator.storage.estimate) return Promise.resolve(null);
		return navigator.storage.estimate().then(function (s) { return s.quota ? s.quota - (s.usage || 0) : null; }, function () { return null; });
	}

	// ---- the download ----
	var sync = null;				// { done, doneBytes, failed, files, bytes, t0, stop, playing }
	function runSync(playing) {
		if (sync) { sync.playing = sync.playing || playing; return sync.promise; }
		var s = sync = { done: 0, doneBytes: 0, failed: 0, files: plan.todo.length, bytes: plan.todoBytes, t0: Date.now(), stop: false, playing: playing };
		var queue = plan.todo.slice(), running = 0;
		var saver = setInterval(saveRecord, 15000);
		s.promise = new Promise(function (finish) {
			function one(e, tries) {
				return fetch(dataUrl(e.rel), { headers: { 'X-PT-Full': '1' } }).then(function (r) {
					if (r.body && r.body.cancel) r.body.cancel().catch(function () {});
					if (!r.ok) throw new Error('HTTP ' + r.status);
					plan.record.set(e.rel, e.hash);
					recordDirty = true;
				}).catch(function (err) {
					if (tries < 2 && !s.stop) return new Promise(function (ok) { setTimeout(ok, 1000 * (tries + 1)); }).then(function () { return one(e, tries + 1); });
					s.failed++;
				});
			}
			function pump() {
				// fewer at once while the game plays, so its own downloads come first;
				// read live so changing the setting (below) takes effect without restarting
				var limit = s.playing ? Math.min(4, getConcurrency()) : getConcurrency();
				while (!s.stop && running < limit && queue.length) {
					var e = queue.shift();
					running++;
					one(e, 0).then((function (e) { return function () {
						running--;
						s.done++;
						s.doneBytes += e.size;
						draw();
						pump();
					}; })(e));
				}
				if (!running && (s.stop || !queue.length)) {
					clearInterval(saver);
					saveRecord();
					s.finished = !s.stop;
					sync = null;
					draw();
					finish(s);
				}
			}
			pump();
		});
		return s.promise;
	}

	// ---- what is on screen ----
	var btn = null, gateDone = null, lastSync = null;
	function draw() {
		var s = sync || lastSync;
		if (btn) {
			var mode = getMode();
			btn.textContent = sync ? 'Downloading game ' + Math.floor(100 * (sync.bytes ? sync.doneBytes / sync.bytes : 1)) + '%' :
				mode == 'full' ? 'Downloads: full game' : 'Downloads: as you play';
		}
		if (panel.hidden || panel.dataset.view != 'sync' || !s) return;
		var frac = s.bytes ? s.doneBytes / s.bytes : 1;
		card.querySelector('.bar i').style.width = Math.max(2, Math.min(100, frac * 100)) + '%';
		var secs = (Date.now() - s.t0) / 1000, rate = s.doneBytes / Math.max(secs, 0.1), left = rate ? (s.bytes - s.doneBytes) / rate : 0;
		card.querySelector('.d').textContent = num(s.done) + ' of ' + num(s.files) + ' files · ' + gb(s.doneBytes) + ' of ' + gb(s.bytes) +
			(secs > 3 && rate ? ' · ' + gb(rate) + '/s' + (left > 5 ? ' · about ' + (left < 90 ? Math.round(left) + ' s' : Math.round(left / 60) + ' min') + ' left' : '') : '');
		if (!sync && s.failed) card.querySelector('.n').textContent = num(s.failed) + ' file(s) could not be downloaded; the game downloads them when it needs them.';
	}
	function el(tag, cls, text) { var e = document.createElement(tag); if (cls) e.className = cls; if (text != null) e.textContent = text; return e; }

	// "Files downloaded at once": most of them are tiny, so more at a time can finish sooner
	// on a fast connection. Shown on the choice panel and while a download is running; pump()
	// (above) reads the setting live, so a change here applies right away.
	function concRow() {
		var row = el('div', 'conc');
		row.appendChild(el('span', null, 'Files downloaded at once:'));
		var sel = document.createElement('select');
		CONC_OPTS.forEach(function (n) {
			var o = document.createElement('option');
			o.value = n; o.textContent = n;
			sel.appendChild(o);
		});
		sel.value = getConcurrency();
		sel.onchange = function () { setConcurrency(+sel.value); };
		row.appendChild(sel);
		row.appendChild(el('span', null, 'Higher can be faster when there are many small files, on a fast connection.'));
		return row;
	}

	// "How should the game download?" -> resolves 'stream' or 'full' (or null: closed)
	function showChoice(running) {
		return new Promise(function (answer) {
			mountPanel();
			card.innerHTML = '';
			panel.dataset.view = 'choice';
			var h = el('h2', null, running ? 'Downloads' : 'How should the game download?');
			h.id = 'pt-dl-t';
			card.appendChild(h);
			var mode = getMode();
			var a = el('button', 'opt' + (mode == 'stream' ? ' cur' : ''));
			a.appendChild(el('b', null, 'Play now'));
			a.appendChild(el('span', null, 'Files download as you go. Quick to start; a new area or monster can take a moment the first time you meet it.'));
			var b = el('button', 'opt' + (mode == 'full' ? ' cur' : ''));
			b.appendChild(el('b', null, 'Download the full game' + (plan ? ' (' + gb(plan.total) + ')' : '')));
			b.appendChild(el('span', null, plan && !plan.todo.length ? 'All downloaded. From now on only the files that change on the server download.' :
				plan && plan.todo.length < plan.entries.length ?
				'Gets the ' + gb(plan.todoBytes) + ' not downloaded yet, then only the files that change. No loading waits while playing.' :
				'Everything once, then only the files that change on the server. No loading waits while playing.'));
			card.appendChild(a);
			card.appendChild(b);
			var warn = el('div', 'warn');
			warn.hidden = true;
			card.appendChild(warn);
			if (plan) storageLeft().then(function (left) {
				if (left != null && left < plan.todoBytes * 1.05) {
					warn.hidden = false;
					warn.textContent = 'Your browser has about ' + gb(Math.max(0, left)) + ' of room for this site; the full game may not fit.';
				}
			});
			card.appendChild(el('div', 'n', running ? 'Files you already have are kept either way.' :
				'You can change this later with the Downloads button under the game.'));
			card.appendChild(concRow());
			if (running) {
				var row = el('div', 'row'), c = el('button', 'btn q', 'Close');
				c.onclick = function () { close(); answer(null); };
				row.appendChild(c);
				card.appendChild(row);
			}
			function close() { panel.hidden = true; document.removeEventListener('keydown', key); }
			function key(e) { if (e.key == 'Escape' && running) { close(); answer(null); } }
			document.addEventListener('keydown', key);
			a.onclick = function () { close(); answer('stream'); };
			b.onclick = function () { close(); answer('full'); };
			panel.hidden = false;
			(mode == 'full' ? b : a).focus();
		});
	}
	// the download's progress; resolves when it is done or the player chose to play
	function showSync(update) {
		return new Promise(function (play) {
			mountPanel();
			card.innerHTML = '';
			panel.dataset.view = 'sync';
			var h = el('h2', null, update ? 'Updating the game' : 'Downloading the full game');
			h.id = 'pt-dl-t';
			card.appendChild(h);
			var bar = el('div', 'bar');
			bar.appendChild(el('i'));
			card.appendChild(bar);
			card.appendChild(el('div', 'd', ''));
			card.appendChild(el('div', 'n', update ? 'Only the files that changed since your last visit.' :
				'Only this once: later visits download just the files that change. You can play while it downloads.'));
			card.appendChild(concRow());
			var row = el('div', 'row'), p = el('button', 'btn', 'Play while it downloads');
			p.onclick = function () { panel.hidden = true; play(); };
			row.appendChild(p);
			card.appendChild(row);
			panel.hidden = false;
			p.focus();
			draw();
		});
	}

	// background: keep going while the game runs
	function startSync(playing) {
		var p = runSync(playing);
		p.then(function (s) { lastSync = s; draw(); });
		return p;
	}

	function addButton() {
		var bar = document.getElementById('bar');
		if (!bar || btn) return;
		btn = el('button');
		btn.title = 'Download the whole game, or only what you play';
		btn.onclick = function () {
			var ready = plan ? Promise.resolve() : makePlan().catch(function () {});
			ready.then(function () { return showChoice(true); }).then(function (m) {
				if (!m) return;
				setMode(m);
				if (m == 'stream') { if (sync) sync.stop = true; }
				else if (plan && sw && sw.controller) {
					if (sync) return;
					makePlan().then(function () { if (plan.todo.length) startSync(true); }).catch(function () {});
				}
				draw();
			});
		};
		var info = document.getElementById('saveInfo');
		bar.insertBefore(btn, info);
		draw();
	}

	// ---- the page's gate (web\shell.html starts the game when it resolves) ----
	window.ptDownloadGate = function () {
		if (gateDone) return gateDone;
		gateDone = (async function () {
			addButton();
			// no service worker (private window, old browser) or not handling this page yet:
			// the game downloads as it plays, as before
			if (!canFull || !sw.controller) return;
			var mode = getMode();
			try {
				if (!mode) {
					await makePlan().catch(function () {});
					mode = await showChoice(false);
					setMode(mode);
					draw();
				}
				if (mode != 'full') return;
				if (!plan) await makePlan();
				if (!plan.todo.length) return;
				var first = plan.todo.length > plan.entries.length / 2;
				var p = startSync(false);
				await Promise.race([p, showSync(!first)]);
				panel.hidden = true;
				if (sync) sync.playing = true;		// still going: the game starts alongside it
			} catch (e) {
				panel.hidden = true;
				if (window.Module && Module.printErr) Module.printErr('[web] full download: ' + (e && e.message || e));
			}
		})();
		return gateDone;
	};
	window.addEventListener('pagehide', function () { saveRecord(); });
})();
