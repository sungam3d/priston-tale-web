// sw.js - service worker: keeps the game's downloads in the browser (Cache
// Storage), so a visit only downloads what changed on the web server since the
// last one. Registered by the page (web\shell.html); copied next to it by
// web\make_site.py.
//  - data/<file>: data_manifest.txt has a content hash per file (third column,
//    web\make_manifest.py / make_site.py). A stored copy with the same hash is used
//    without asking the server; a changed hash downloads the file again.
//  - data_startup.pack, data_manifest.txt: the stored copy is checked with the
//    server (ETag / Last-Modified) - a few hundred bytes - and downloaded again
//    only when it changed.
// Everything else (page, game.js, game.wasm, ws.ashx) goes to the network as before.
// Downloads from the network are announced on BroadcastChannel 'pt-downloads' for
// the page's progress bar (web\progress.js): { t: 'start', id, rel, size, batch },
// { t: 'bytes', id, got }, { t: 'end', id, ok }, and for a batch (below)
// { t: 'batch', files, size } before its downloads start.
// Batches: the game opens files one at a time, but when it loads a map or a model
// it knows every texture that comes next. It posts that list (one path per line)
// to __pt_prefetch (smTexture.cpp ReadTextures; a request, so a sleeping service
// worker wakes up); they download here several at once, and the game's own request
// for a file still downloading waits for that download (one download per file).
var CACHE = 'pt-data-1';				// change to drop every stored file
var CHECKED = ['data_startup.pack', 'data_manifest.txt'];
var MANIFEST = 'data_manifest.txt';
var scopePath = new URL(self.registration.scope).pathname;

var channel = self.BroadcastChannel ? new BroadcastChannel('pt-downloads') : null;
var nextId = 0;
// res.body, counted as it passes; announced as a download of rel
function counted(res, rel, size, batch) {
	if (!channel || !res.body || !self.TransformStream) return res.body;
	var id = ++nextId, got = 0, last = 0;
	channel.postMessage({ t: 'start', id: id, rel: rel, size: size || +res.headers.get('Content-Length') || 0, batch: !!batch });
	return res.body.pipeThrough(new TransformStream({
		transform: function (chunk, ctl) {
			got += chunk.length;
			var now = Date.now();
			if (now - last > 150) { last = now; channel.postMessage({ t: 'bytes', id: id, got: got }); }
			ctl.enqueue(chunk);
		},
		flush: function () { channel.postMessage({ t: 'end', id: id, ok: true }); }
	}));
}

self.addEventListener('install', function () { self.skipWaiting(); });
self.addEventListener('activate', function (e) {
	e.waitUntil(caches.keys().then(function (names) {
		return Promise.all(names.filter(function (n) { return n.indexOf('pt-data-') == 0 && n != CACHE; })
			.map(function (n) { return caches.delete(n); }));
	}).then(function () { return self.clients.claim(); }));
});

self.addEventListener('fetch', function (e) {
	var req = e.request;
	var url = new URL(req.url);
	if (url.origin != location.origin || url.search || url.pathname.indexOf(scopePath) != 0) return;
	var rel = url.pathname.slice(scopePath.length);
	if (rel == '__pt_prefetch' && req.method == 'POST') {
		// answered at once; the downloads keep the worker awake until they are done
		e.respondWith(req.text().then(function (t) {
			e.waitUntil(prefetch(t.split('\n').filter(Boolean)).catch(function () {}));
			return new Response(null, { status: 204 });
		}));
		return;
	}
	if (req.method != 'GET' || req.headers.has('Range')) return;
	if (rel.indexOf('data/') == 0) e.respondWith(dataFile(e, decodeURIComponent(rel.slice(5))));
	else if (CHECKED.indexOf(rel) >= 0) e.respondWith(checked(e, rel));
});

// rel path -> { size, hash, rel }, from the stored manifest (read again after the
// worker restarts); also under the lower-case path (the game's names differ in case)
var versions = null;
function parseManifest(text) {
	var m = new Map();
	text.split('\n').forEach(function (l) {
		var a = l.split('\t');
		if (a.length > 2) {
			var v = { size: +a[1], hash: a[2].trim(), rel: a[0] };
			m.set(a[0], v);
			if (!m.has(a[0].toLowerCase())) m.set(a[0].toLowerCase(), v);
		}
	});
	return m;
}
function getVersions() {
	if (!versions) versions = caches.open(CACHE)
		.then(function (c) { return c.match(new URL(MANIFEST, self.registration.scope).href); })
		.then(function (r) { return r ? r.text() : ''; })
		.then(parseManifest)
		.catch(function () { return new Map(); });
	return versions;
}

function dataUrl(rel) {
	return new URL('data/' + rel.split('/').map(encodeURIComponent).join('/'), self.registration.scope).href;
}

// Downloads still running, by URL: a second request for one waits for it.
var inflight = new Map();

// A data file from the network into the cache: { body, headers } or a Response
// that is not 200 (passed on as it is).
function download(url, rel, entry, batch) {
	var p = inflight.get(url);
	if (p) return p;
	p = (async function () {
		// 'no-cache': the browser's own cache may still hold the old copy (data\web.config max-age)
		var res = await fetch(url, { cache: 'no-cache' });
		if (res.status != 200) return res;
		var body = await new Response(counted(res, rel, entry.size, batch)).arrayBuffer();
		var h = new Headers(res.headers);
		h.delete('Content-Encoding');
		h.set('Content-Length', String(body.byteLength));
		h.set('X-PT-Version', entry.hash);
		var cache = await caches.open(CACHE);
		await cache.put(url, new Response(body, { status: 200, statusText: 'OK', headers: h })).catch(function () {});	// storage full: still played
		return { body: body, headers: h };
	})();
	inflight.set(url, p);
	p.then(function () { inflight.delete(url); }, function () { inflight.delete(url); });
	return p;
}

async function dataFile(e, rel) {
	var entry = (await getVersions()).get(rel);
	if (!entry) return fetch(e.request);			// not listed (or old manifest without hashes)
	var cache = await caches.open(CACHE);
	var hit = await cache.match(e.request.url);
	if (hit && hit.headers.get('X-PT-Version') == entry.hash) return hit;
	var r = await download(e.request.url, rel, entry, false);
	if (r instanceof Response) return r;
	return new Response(r.body, { status: 200, statusText: 'OK', headers: r.headers });
}

// ---- batches (see the top) ----
var BATCH_PARALLEL = 6;
var queue = [], running = 0;
function pump() {
	while (running < BATCH_PARALLEL && queue.length) {
		var job = queue.shift();
		running++;
		download(job.url, job.rel, job.entry, true).catch(function () {}).then(job.done).then(function () { running--; pump(); });
	}
}
async function prefetch(rels) {
	var man = await getVersions();
	var cache = await caches.open(CACHE);
	var seen = new Set(), jobs = [], size = 0;
	async function add(name) {
		var entry = man.get(name) || man.get(name.toLowerCase());
		if (!entry || seen.has(entry.rel)) return;
		seen.add(entry.rel);
		var url = dataUrl(entry.rel);
		if (inflight.has(url)) return;
		var hit = await cache.match(url);
		if (hit && hit.headers.get('X-PT-Version') == entry.hash) return;
		jobs.push({ url: url, rel: entry.rel, entry: entry });
		size += entry.size;
	}
	for (var i = 0; i < rels.length; i++) {
		var name = String(rels[i]).replace(/\\/g, '/').replace(/^\.?\//, '');
		await add(name);
		// smaller copies of the texture (mipmaps: name_mm1.ext ...) when there are any
		var dot = name.lastIndexOf('.');
		if (dot > 0) for (var k = 1; k <= 4; k++) await add(name.slice(0, dot) + '_mm' + k + name.slice(dot));
	}
	if (!jobs.length) return;
	if (channel) channel.postMessage({ t: 'batch', files: jobs.length, size: size });
	var all = jobs.map(function (j) { return new Promise(function (ok) { j.done = ok; }); });
	queue.push.apply(queue, jobs);
	pump();
	return Promise.all(all);
}

async function checked(e, rel) {
	var cache = await caches.open(CACHE);
	var hit = await cache.match(e.request.url);
	var cond = {};
	if (hit) {
		if (hit.headers.get('ETag')) cond['If-None-Match'] = hit.headers.get('ETag');
		if (hit.headers.get('Last-Modified')) cond['If-Modified-Since'] = hit.headers.get('Last-Modified');
	}
	var res;
	try { res = await fetch(e.request.url, { cache: 'no-store', headers: cond }); }
	catch (err) { if (hit) return hit; throw err; }		// server unreachable: use the stored copy
	if (res.status == 304 && hit) return hit;
	if (res.status != 200) return res;
	if (rel == MANIFEST) {
		var text = res.clone().text();
		versions = text.then(parseManifest);
		e.waitUntil(cache.put(e.request.url, res.clone()).then(function () { return versions; }).then(prune).catch(function () {}));
	}
	else {
		e.waitUntil(cache.put(e.request.url, res.clone()).catch(function () {}));
		return new Response(counted(res, rel), { status: res.status, statusText: res.statusText, headers: res.headers });
	}
	return res;
}

// data files no longer in the manifest
async function prune(man) {
	var cache = await caches.open(CACHE);
	var base = new URL('data/', self.registration.scope).href;
	var keys = await cache.keys();
	for (var i = 0; i < keys.length; i++) {
		var u = keys[i].url;
		if (u.indexOf(base) == 0 && !man.has(decodeURIComponent(u.slice(base.length)))) await cache.delete(keys[i]);
	}
}
