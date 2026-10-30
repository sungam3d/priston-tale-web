// pre.js - runs before the game (web build).
//  - creates the game folder in the browser file system
//  - loads the data manifest, so the game can see every data file (each one is
//    downloaded the first time it is opened - ptwin_fs.cpp)
//  - unpacks data_startup.pack (the files needed up to the login screen, in one
//    download - made by web\make_pack.py), if there is one
//  - puts back the saves (files the game wrote before) from the browser's storage

// ---------------------------------------------------------------------------
// Saves: every file the game writes (characters, accounts, mail, castle...) is
// kept in IndexedDB - database 'pristontale-saves', store 'files', key = path in
// the game folder ('server/DataServer/userinfo/205/player.dat'), value
// { data: Uint8Array, or null for a deleted file; time }.
// ptwin_fs.cpp calls Module.ptSaveChanged(rel, deleted) when the game closes or
// deletes a file; it is copied a second later. The server worker keeps the files
// under server/, the page all others.
// ---------------------------------------------------------------------------
var PtSaves = (function () {
	var DB = 'pristontale-saves', STORE = 'files';
	var dbPromise = null;
	function db() {
		if (!dbPromise) dbPromise = new Promise(function (resolve, reject) {
			var r = indexedDB.open(DB, 1);
			r.onupgradeneeded = function () { r.result.createObjectStore(STORE); };
			r.onsuccess = function () { resolve(r.result); };
			r.onerror = function () { reject(r.error); };
		});
		return dbPromise;
	}
	function all() {			// -> [[rel, { data, time }], ...]
		return db().then(function (d) {
			return new Promise(function (resolve, reject) {
				var out = [], req = d.transaction(STORE, 'readonly').objectStore(STORE).openCursor();
				req.onsuccess = function () {
					var c = req.result;
					if (c) { out.push([c.key, c.value]); c.continue(); } else resolve(out);
				};
				req.onerror = function () { reject(req.error); };
			});
		});
	}
	function write(entries, clearFirst) {
		return db().then(function (d) {
			return new Promise(function (resolve, reject) {
				var tx = d.transaction(STORE, 'readwrite'), st = tx.objectStore(STORE);
				if (clearFirst) st.clear();
				entries.forEach(function (e) { st.put(e[1], e[0]); });
				tx.oncomplete = function () { resolve(); };
				tx.onerror = tx.onabort = function () { reject(tx.error); };
			});
		});
	}

	var pending = {}, timer = 0;
	function changed(rel, deleted) {
		pending[rel] = deleted;
		if (!timer) timer = setTimeout(flush, 1000);
	}
	function flush() {
		if (timer) { clearTimeout(timer); timer = 0; }
		var rels = Object.keys(pending);
		if (!rels.length) return Promise.resolve(0);
		var entries = rels.map(function (rel) {
			var data = null;
			if (!pending[rel]) { try { data = FS.readFile('/game/' + rel); } catch (e) {} }
			return [rel, { data: data, time: Date.now() }];
		});
		pending = {};
		entries.forEach(function (e) {			// deletions are rare and final: say so in the log
			if (!e[1].data) Module['print']('[saves] deleted ' + e[0]);
		});
		return write(entries).then(function () {
			if (Module['ptSaveDone']) Module['ptSaveDone'](entries.length);
			return entries.length;
		}).catch(function (e) {
			Module['printErr']('[saves] could not store ' + entries.length + ' file(s): ' + e);
			return 0;
		});
	}

	// Export file (.ptsave): "PTSV", u32 count, then per file: u16 path length, path
	// (UTF-8), u8 deleted, u32 size, data. Little endian.
	function pack(entries) {
		var enc = new TextEncoder(), parts = [], size = 8;
		entries.forEach(function (e) {
			var p = enc.encode(e[0]), d = e[1].data;
			parts.push([p, d]);
			size += 2 + p.length + 1 + 4 + (d ? d.length : 0);
		});
		var buf = new Uint8Array(size), dv = new DataView(buf.buffer), o = 8;
		buf.set([0x50, 0x54, 0x53, 0x56]);
		dv.setUint32(4, parts.length, true);
		parts.forEach(function (x) {
			dv.setUint16(o, x[0].length, true); o += 2;
			buf.set(x[0], o); o += x[0].length;
			buf[o++] = x[1] ? 0 : 1;
			dv.setUint32(o, x[1] ? x[1].length : 0, true); o += 4;
			if (x[1]) { buf.set(x[1], o); o += x[1].length; }
		});
		return buf;
	}
	function unpack(buf) {
		var dv = new DataView(buf.buffer, buf.byteOffset, buf.byteLength), dec = new TextDecoder();
		if (buf.length < 8 || dec.decode(buf.subarray(0, 4)) !== 'PTSV') throw new Error('not a Priston Tale save file');
		var n = dv.getUint32(4, true), o = 8, out = [], now = Date.now();
		for (var i = 0; i < n; i++) {
			var pl = dv.getUint16(o, true); o += 2;
			var rel = dec.decode(buf.subarray(o, o + pl)); o += pl;
			var deleted = buf[o++];
			var size = dv.getUint32(o, true); o += 4;
			if (o + size > buf.length) throw new Error('save file is cut off');
			out.push([rel, { data: deleted ? null : buf.slice(o, o + size), time: now }]);
			o += size;
		}
		return out;
	}

	return { all: all, write: write, changed: changed, flush: flush, pack: pack, unpack: unpack };
})();
Module['ptSaves'] = PtSaves;
Module['ptSaveChanged'] = PtSaves.changed;

Module['preRun'] = Module['preRun'] || [];
Module['preRun'].push(function () {
	var isServer = !!Module['ptServer'];
	FS.mkdir('/game');

	function mkdirs(path) {
		var k = 0;
		while ((k = path.indexOf('/', k + 1)) > 0) { try { FS.mkdir(path.slice(0, k)); } catch (e) {} }
	}

	var manifest = fetch('data_manifest.txt')
		.then(function (r) { if (!r.ok) throw new Error('HTTP ' + r.status); return r.arrayBuffer(); })
		.catch(function (e) { console.error('data manifest missing - run web\\make_manifest.py', e); return null; });

	var startupPack = fetch(isServer ? 'data_server.pack' : 'data_startup.pack')
		.then(function (r) {
			if (!r.ok) return null;
			var total = +r.headers.get('Content-Length') || 0, got = 0, chunks = [];
			var reader = r.body.getReader(), lastPaint = Date.now();
			// the page's loading panel (web\progress.js) shows these figures
			Module['ptPack'] = { got: 0, size: total, t0: Date.now() };
			function pump() {
				return reader.read().then(function (res) {
					if (res.done) return;
					chunks.push(res.value);
					got += res.value.length;
					Module['ptPack'].got = got;
					if (Module.setStatus) Module.setStatus('Downloading game data... ' + (total ? Math.floor(got * 100 / total) + '%' : (got >> 20) + ' MB'));
					// already-buffered data resolves at once: without a pause now and then the
					// page would not repaint (or handle anything else) until the download ends
					if (Date.now() - lastPaint > 100) {
						lastPaint = Date.now();
						return new Promise(function (ok) { setTimeout(ok, 0); }).then(pump);
					}
					return pump();
				});
			}
			return pump().then(function () {
				var buf = new Uint8Array(got), o = 0;
				chunks.forEach(function (c) { buf.set(c, o); o += c.length; });
				return buf;
			});
		})
		.catch(function (e) { console.warn('start-up pack not used', e); return null; });

	var saves = PtSaves.all().catch(function (e) {
		Module['printErr']('[saves] browser storage not available - progress will not be kept (' + e + ')');
		return [];
	});

	// In this order: start-up pack, then the saves over it, then the manifest
	// without the files the game deleted.
	addRunDependency('game-data');
	Promise.all([manifest, startupPack, saves]).then(function (r) {
		var man = r[0], buf = r[1], saved = r[2];

		if (buf) {
			try {
				var dv = new DataView(buf.buffer), o = 8;
				if (String.fromCharCode(buf[0], buf[1], buf[2], buf[3]) !== 'PTPK') throw new Error('not a pack');
				var count = dv.getUint32(4, true);
				var dec = new TextDecoder();
				for (var i = 0; i < count; i++) {
					var n = dv.getUint16(o, true); o += 2;
					var rel = dec.decode(buf.subarray(o, o + n)); o += n;
					var size = dv.getUint32(o, true); o += 4;
					mkdirs('/game/' + rel);
					FS.writeFile('/game/' + rel, buf.subarray(o, o + size));
					o += size;
				}
				console.log('[pre] start-up pack: ' + count + ' files');
			} catch (e) { console.warn('start-up pack not used', e); }
		}

		var deleted = {}, restored = [];
		saved.forEach(function (e) {
			var rel = e[0], v = e[1];
			var mine = (rel.slice(0, 7).toLowerCase() === 'server/') === isServer;
			if (!v.data) {
				deleted[rel.toLowerCase()] = 1;
				if (mine) { try { FS.unlink('/game/' + rel); } catch (x) {} }
				return;
			}
			if (!mine) return;
			mkdirs('/game/' + rel);
			FS.writeFile('/game/' + rel, v.data);
			restored.push(rel);
		});
		FS.writeFile('/game_saves_restored.txt', restored.join('\n'));
		if (restored.length) Module['print']('[saves] ' + restored.length + ' saved file(s) restored');

		if (man) {
			var bytes = new Uint8Array(man);
			if (Object.keys(deleted).length) {
				var lines = new TextDecoder().decode(bytes).split('\n').filter(function (l) {
					var t = l.indexOf('\t');
					return t < 0 || !deleted[l.slice(0, t).toLowerCase()];
				});
				bytes = new TextEncoder().encode(lines.join('\n'));
			}
			FS.writeFile('/game_manifest.txt', bytes);
		}
	}).catch(function (e) {
		Module['printErr']('[pre] start-up failed: ' + e);
	}).finally(function () { removeRunDependency('game-data'); });
});

// ---------------------------------------------------------------------------
// Network between the game (page) and the offline server (Web Worker running
// the same game.js). ptwin_net.cpp calls Module.ptNetPost(kind, link, port,
// bytes); the other side gets it in Module._PtNet_Receive.
// ---------------------------------------------------------------------------
(function () {
	var inbox = [];				// messages that arrive before the runtime is ready
	var outbox = [];			// page: messages before the server worker exists
	var ready = false;

	function deliver(m) {
		var bytes = m[3], p = _malloc(bytes.length || 1);
		HEAPU8.set(bytes, p);
		Module['_PtNet_Receive'](m[0], m[1], m[2], p, bytes.length);
		_free(p);
	}
	function receive(m) { if (ready) deliver(m); else inbox.push(m); }

	Module['onRuntimeInitialized'] = function () {
		ready = true;
		inbox.splice(0).forEach(deliver);
	};

	if (Module['ptServer']) {
		// inside the server worker: talk to the page
		self.addEventListener('message', function (e) {
			if (!e.data) return;
			if (e.data.ptnet) receive(e.data.ptnet);
			else if (e.data.saveFlush !== undefined)		// the page wants the saves up to date (export)
				PtSaves.flush().then(function (n) { postMessage({ saveFlushed: e.data.saveFlush, count: n }); });
		});
		Module['ptNetPost'] = function (kind, link, port, bytes) {
			postMessage({ ptnet: [kind, link, port, bytes] }, [bytes.buffer]);
		};
		Module['ptSaveDone'] = function (n) { postMessage({ saveDone: n }); };
		return;
	}

	// on the page: start the server worker when the game asks for it (StartOfflineServer)
	var flushWaits = {}, flushId = 0;
	Module['ptStartServer'] = function () {
		if (Module['ptServerWorker']) return;
		var w = new Worker('server_worker.js' + location.search, { name: 'pt-server' });
		w.onmessage = function (e) {
			if (!e.data) return;
			if (e.data.ptnet) receive(e.data.ptnet);
			else if (e.data.log !== undefined) Module['print'](e.data.log);
			else if (e.data.saveDone !== undefined) { if (Module['ptSaveDone']) Module['ptSaveDone'](e.data.saveDone); }
			else if (e.data.saveFlushed !== undefined && flushWaits[e.data.saveFlushed]) {
				flushWaits[e.data.saveFlushed]();
				delete flushWaits[e.data.saveFlushed];
			}
		};
		w.onerror = function (e) { Module['printErr']('[server] ' + (e.message || e)); };
		Module['ptServerWorker'] = w;
		outbox.splice(0).forEach(function (m) { w.postMessage({ ptnet: m }, [m[3].buffer]); });
		Module['print']('[web] offline server starting in a worker');
	};
	Module['ptNetPost'] = function (kind, link, port, bytes) {
		if (Module['ptServerUrl']) return remotePost(kind, link, port, bytes);
		var m = [kind, link, port, bytes];
		if (Module['ptServerWorker']) Module['ptServerWorker'].postMessage({ ptnet: m }, [bytes.buffer]);
		else outbox.push(m);
	};

	// Shared server (config.js sets Module.ptServerUrl): every game connection is a
	// WebSocket to the bridge (ws.ashx on IIS, serve.py locally), which connects to
	// server.exe on that port and passes the bytes both ways. Same messages as the
	// worker link: 1 connect 2 accepted 3 refused 4 data 5 closed.
	var sockets = {};
	function bridgeUrl(port) {
		var u = new URL(Module['ptServerUrl'], location.href);
		u.protocol = u.protocol === 'https:' ? 'wss:' : 'ws:';
		u.searchParams.set('port', port);
		return u.href;
	}
	function remotePost(kind, link, port, bytes) {
		var s = sockets[link];
		if (kind === 1) {
			var ws;
			try { ws = new WebSocket(bridgeUrl(port)); } catch (e) { receive([3, link, port, new Uint8Array(0)]); return; }
			ws.binaryType = 'arraybuffer';
			s = sockets[link] = { ws: ws, open: false, queue: [] };
			ws.onopen = function () {
				s.open = true;
				s.queue.splice(0).forEach(function (b) { ws.send(b); });
				receive([2, link, port, new Uint8Array(0)]);
			};
			ws.onmessage = function (e) { receive([4, link, 0, new Uint8Array(e.data)]); };
			ws.onclose = function () {
				var wasOpen = s.open;
				s.open = false;
				delete sockets[link];
				receive([wasOpen ? 5 : 3, link, port, new Uint8Array(0)]);
				if (!wasOpen) Module['printErr']('[web] cannot reach the game server (' + bridgeUrl(port) + ')');
			};
		}
		else if (kind === 4 && s) {
			if (s.open) s.ws.send(bytes); else s.queue.push(bytes);
		}
		else if (kind === 5 && s) {
			s.ws.onclose = null;
			delete sockets[link];
			s.ws.close();
		}
	}

	// Saves of both the page and the server written to storage now (before an export)
	Module['ptFlushSaves'] = function () {
		var waits = [PtSaves.flush()];
		var w = Module['ptServerWorker'];
		if (w) waits.push(new Promise(function (resolve) {
			var id = ++flushId;
			flushWaits[id] = resolve;
			w.postMessage({ saveFlush: id });
			setTimeout(resolve, 5000);			// server busy or gone: export what is stored
		}));
		return Promise.all(waits);
	};
})();
