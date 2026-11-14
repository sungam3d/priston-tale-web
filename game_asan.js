// include: shell.js
// include: minimum_runtime_check.js
// end include: minimum_runtime_check.js
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(moduleArg) => Promise<Module>
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module = typeof Module != "undefined" ? Module : {};

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).
// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = !!globalThis.window;

var ENVIRONMENT_IS_WORKER = !!globalThis.WorkerGlobalScope;

// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = globalThis.process?.versions?.node && globalThis.process?.type != "renderer";

var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

// Three configurations we can be running in:
// 1) We could be the application main() thread running in the main JS UI thread. (ENVIRONMENT_IS_WORKER == false and ENVIRONMENT_IS_PTHREAD == false)
// 2) We could be the application main() running directly in a worker. (ENVIRONMENT_IS_WORKER == true, ENVIRONMENT_IS_PTHREAD == false)
// 3) We could be an application pthread running in a worker. (ENVIRONMENT_IS_WORKER == true and ENVIRONMENT_IS_PTHREAD == true)
// The way we signal to a worker that it is hosting a pthread is to construct
// it with a specific name.
var ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && globalThis.name == "em-pthread";

if (ENVIRONMENT_IS_NODE) {
  var worker_threads = require("node:worker_threads");
  globalThis.Worker = worker_threads.Worker;
  ENVIRONMENT_IS_WORKER = !worker_threads.isMainThread;
  // Under node we set `workerData` to `em-pthread` to signal that the worker
  // is hosting a pthread.
  ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && worker_threads.workerData == "em-pthread";
}

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
// include: C:/SunnyZ/PristonBuild/src/web/pre.js
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
var PtSaves = (function() {
  var DB = "pristontale-saves", STORE = "files";
  var dbPromise = null;
  function db() {
    if (!dbPromise) dbPromise = new Promise(function(resolve, reject) {
      var r = indexedDB.open(DB, 1);
      r.onupgradeneeded = function() {
        r.result.createObjectStore(STORE);
      };
      r.onsuccess = function() {
        resolve(r.result);
      };
      r.onerror = function() {
        reject(r.error);
      };
    });
    return dbPromise;
  }
  function all() {
    // -> [[rel, { data, time }], ...]
    return db().then(function(d) {
      return new Promise(function(resolve, reject) {
        var out = [], req = d.transaction(STORE, "readonly").objectStore(STORE).openCursor();
        req.onsuccess = function() {
          var c = req.result;
          if (c) {
            out.push([ c.key, c.value ]);
            c.continue();
          } else resolve(out);
        };
        req.onerror = function() {
          reject(req.error);
        };
      });
    });
  }
  function write(entries, clearFirst) {
    return db().then(function(d) {
      return new Promise(function(resolve, reject) {
        var tx = d.transaction(STORE, "readwrite"), st = tx.objectStore(STORE);
        if (clearFirst) st.clear();
        entries.forEach(function(e) {
          st.put(e[1], e[0]);
        });
        tx.oncomplete = function() {
          resolve();
        };
        tx.onerror = tx.onabort = function() {
          reject(tx.error);
        };
      });
    });
  }
  var pending = {}, timer = 0;
  function changed(rel, deleted) {
    pending[rel] = deleted;
    if (!timer) timer = setTimeout(flush, 1e3);
  }
  function flush() {
    if (timer) {
      clearTimeout(timer);
      timer = 0;
    }
    var rels = Object.keys(pending);
    if (!rels.length) return Promise.resolve(0);
    var entries = rels.map(function(rel) {
      var data = null;
      if (!pending[rel]) {
        try {
          data = FS.readFile("/game/" + rel);
        } catch (e) {}
      }
      return [ rel, {
        data,
        time: Date.now()
      } ];
    });
    pending = {};
    entries.forEach(function(e) {
      // deletions are rare and final: say so in the log
      if (!e[1].data) Module["print"]("[saves] deleted " + e[0]);
    });
    return write(entries).then(function() {
      if (Module["ptSaveDone"]) Module["ptSaveDone"](entries.length);
      return entries.length;
    }).catch(function(e) {
      Module["printErr"]("[saves] could not store " + entries.length + " file(s): " + e);
      return 0;
    });
  }
  // Export file (.ptsave): "PTSV", u32 count, then per file: u16 path length, path
  // (UTF-8), u8 deleted, u32 size, data. Little endian.
  function pack(entries) {
    var enc = new TextEncoder, parts = [], size = 8;
    entries.forEach(function(e) {
      var p = enc.encode(e[0]), d = e[1].data;
      parts.push([ p, d ]);
      size += 2 + p.length + 1 + 4 + (d ? d.length : 0);
    });
    var buf = new Uint8Array(size), dv = new DataView(buf.buffer), o = 8;
    buf.set([ 80, 84, 83, 86 ]);
    dv.setUint32(4, parts.length, true);
    parts.forEach(function(x) {
      dv.setUint16(o, x[0].length, true);
      o += 2;
      buf.set(x[0], o);
      o += x[0].length;
      buf[o++] = x[1] ? 0 : 1;
      dv.setUint32(o, x[1] ? x[1].length : 0, true);
      o += 4;
      if (x[1]) {
        buf.set(x[1], o);
        o += x[1].length;
      }
    });
    return buf;
  }
  function unpack(buf) {
    var dv = new DataView(buf.buffer, buf.byteOffset, buf.byteLength), dec = new TextDecoder;
    if (buf.length < 8 || dec.decode(buf.subarray(0, 4)) !== "PTSV") throw new Error("not a Priston Tale save file");
    var n = dv.getUint32(4, true), o = 8, out = [], now = Date.now();
    for (var i = 0; i < n; i++) {
      var pl = dv.getUint16(o, true);
      o += 2;
      var rel = dec.decode(buf.subarray(o, o + pl));
      o += pl;
      var deleted = buf[o++];
      var size = dv.getUint32(o, true);
      o += 4;
      if (o + size > buf.length) throw new Error("save file is cut off");
      out.push([ rel, {
        data: deleted ? null : buf.slice(o, o + size),
        time: now
      } ]);
      o += size;
    }
    return out;
  }
  return {
    all,
    write,
    changed,
    flush,
    pack,
    unpack
  };
})();

Module["ptSaves"] = PtSaves;

Module["ptSaveChanged"] = PtSaves.changed;

Module["preRun"] = Module["preRun"] || [];

Module["preRun"].push(function() {
  var isServer = !!Module["ptServer"];
  FS.mkdir("/game");
  function mkdirs(path) {
    var k = 0;
    while ((k = path.indexOf("/", k + 1)) > 0) {
      try {
        FS.mkdir(path.slice(0, k));
      } catch (e) {}
    }
  }
  var manifest = fetch("data_manifest.txt").then(function(r) {
    if (!r.ok) throw new Error("HTTP " + r.status);
    return r.arrayBuffer();
  }).catch(function(e) {
    console.error("data manifest missing - run web\\make_manifest.py", e);
    return null;
  });
  var startupPack = fetch(isServer ? "data_server.pack" : "data_startup.pack").then(function(r) {
    if (!r.ok) return null;
    var total = +r.headers.get("Content-Length") || 0, got = 0, chunks = [];
    var reader = r.body.getReader(), lastPaint = Date.now();
    // the page's loading panel (web\progress.js) shows these figures
    Module["ptPack"] = {
      got: 0,
      size: total,
      t0: Date.now()
    };
    function pump() {
      return reader.read().then(function(res) {
        if (res.done) return;
        chunks.push(res.value);
        got += res.value.length;
        Module["ptPack"].got = got;
        if (Module.setStatus) Module.setStatus("Downloading game data... " + (total ? Math.floor(got * 100 / total) + "%" : (got >> 20) + " MB"));
        // already-buffered data resolves at once: without a pause now and then the
        // page would not repaint (or handle anything else) until the download ends
        if (Date.now() - lastPaint > 100) {
          lastPaint = Date.now();
          return new Promise(function(ok) {
            setTimeout(ok, 0);
          }).then(pump);
        }
        return pump();
      });
    }
    return pump().then(function() {
      var buf = new Uint8Array(got), o = 0;
      chunks.forEach(function(c) {
        buf.set(c, o);
        o += c.length;
      });
      return buf;
    });
  }).catch(function(e) {
    console.warn("start-up pack not used", e);
    return null;
  });
  var saves = PtSaves.all().catch(function(e) {
    Module["printErr"]("[saves] browser storage not available - progress will not be kept (" + e + ")");
    return [];
  });
  // In this order: start-up pack, then the saves over it, then the manifest
  // without the files the game deleted.
  addRunDependency("game-data");
  Promise.all([ manifest, startupPack, saves ]).then(function(r) {
    var man = r[0], buf = r[1], saved = r[2];
    if (buf) {
      try {
        var dv = new DataView(buf.buffer), o = 8;
        if (String.fromCharCode(buf[0], buf[1], buf[2], buf[3]) !== "PTPK") throw new Error("not a pack");
        var count = dv.getUint32(4, true);
        var dec = new TextDecoder;
        for (var i = 0; i < count; i++) {
          var n = dv.getUint16(o, true);
          o += 2;
          var rel = dec.decode(buf.subarray(o, o + n));
          o += n;
          var size = dv.getUint32(o, true);
          o += 4;
          mkdirs("/game/" + rel);
          FS.writeFile("/game/" + rel, buf.subarray(o, o + size));
          o += size;
        }
        console.log("[pre] start-up pack: " + count + " files");
      } catch (e) {
        console.warn("start-up pack not used", e);
      }
    }
    var deleted = {}, restored = [];
    saved.forEach(function(e) {
      var rel = e[0], v = e[1];
      var mine = (rel.slice(0, 7).toLowerCase() === "server/") === isServer;
      if (!v.data) {
        deleted[rel.toLowerCase()] = 1;
        if (mine) {
          try {
            FS.unlink("/game/" + rel);
          } catch (x) {}
        }
        return;
      }
      if (!mine) return;
      mkdirs("/game/" + rel);
      FS.writeFile("/game/" + rel, v.data);
      restored.push(rel);
    });
    FS.writeFile("/game_saves_restored.txt", restored.join("\n"));
    if (restored.length) Module["print"]("[saves] " + restored.length + " saved file(s) restored");
    if (man) {
      var bytes = new Uint8Array(man);
      if (Object.keys(deleted).length) {
        var lines = (new TextDecoder).decode(bytes).split("\n").filter(function(l) {
          var t = l.indexOf("\t");
          return t < 0 || !deleted[l.slice(0, t).toLowerCase()];
        });
        bytes = (new TextEncoder).encode(lines.join("\n"));
      }
      FS.writeFile("/game_manifest.txt", bytes);
    }
  }).catch(function(e) {
    Module["printErr"]("[pre] start-up failed: " + e);
  }).finally(function() {
    removeRunDependency("game-data");
  });
});

// ---------------------------------------------------------------------------
// Network between the game (page) and the offline server (Web Worker running
// the same game.js). ptwin_net.cpp calls Module.ptNetPost(kind, link, port,
// bytes); the other side gets it in Module._PtNet_Receive.
// ---------------------------------------------------------------------------
(function() {
  var inbox = [];
  // messages that arrive before the runtime is ready
  var outbox = [];
  // page: messages before the server worker exists
  var ready = false;
  function deliver(m) {
    var bytes = m[3], p = _malloc(bytes.length || 1);
    (growMemViews(), HEAPU8).set(bytes, p >>> 0);
    Module["_PtNet_Receive"](m[0], m[1], m[2], p, bytes.length);
    _free(p);
  }
  function receive(m) {
    if (ready) deliver(m); else inbox.push(m);
  }
  Module["onRuntimeInitialized"] = function() {
    ready = true;
    inbox.splice(0).forEach(deliver);
  };
  if (Module["ptServer"]) {
    // inside the server worker: talk to the page
    self.addEventListener("message", function(e) {
      if (!e.data) return;
      if (e.data.ptnet) receive(e.data.ptnet); else if (e.data.saveFlush !== undefined) // the page wants the saves up to date (export)
      PtSaves.flush().then(function(n) {
        postMessage({
          saveFlushed: e.data.saveFlush,
          count: n
        });
      });
    });
    Module["ptNetPost"] = function(kind, link, port, bytes) {
      postMessage({
        ptnet: [ kind, link, port, bytes ]
      }, [ bytes.buffer ]);
    };
    Module["ptSaveDone"] = function(n) {
      postMessage({
        saveDone: n
      });
    };
    return;
  }
  // on the page: start the server worker when the game asks for it (StartOfflineServer)
  var flushWaits = {}, flushId = 0;
  Module["ptStartServer"] = function() {
    if (Module["ptServerWorker"]) return;
    var w = new Worker("server_worker.js" + location.search, {
      name: "pt-server"
    });
    w.onmessage = function(e) {
      if (!e.data) return;
      if (e.data.ptnet) receive(e.data.ptnet); else if (e.data.log !== undefined) Module["print"](e.data.log); else if (e.data.saveDone !== undefined) {
        if (Module["ptSaveDone"]) Module["ptSaveDone"](e.data.saveDone);
      } else if (e.data.saveFlushed !== undefined && flushWaits[e.data.saveFlushed]) {
        flushWaits[e.data.saveFlushed]();
        delete flushWaits[e.data.saveFlushed];
      }
    };
    w.onerror = function(e) {
      Module["printErr"]("[server] " + (e.message || e));
    };
    Module["ptServerWorker"] = w;
    outbox.splice(0).forEach(function(m) {
      w.postMessage({
        ptnet: m
      }, [ m[3].buffer ]);
    });
    Module["print"]("[web] offline server starting in a worker");
  };
  Module["ptNetPost"] = function(kind, link, port, bytes) {
    if (Module["ptServerUrl"]) return remotePost(kind, link, port, bytes);
    var m = [ kind, link, port, bytes ];
    if (Module["ptServerWorker"]) Module["ptServerWorker"].postMessage({
      ptnet: m
    }, [ bytes.buffer ]); else outbox.push(m);
  };
  // Shared server (config.js sets Module.ptServerUrl): every game connection is a
  // WebSocket to the bridge (ws.ashx on IIS, serve.py locally), which connects to
  // server.exe on that port and passes the bytes both ways. Same messages as the
  // worker link: 1 connect 2 accepted 3 refused 4 data 5 closed.
  var sockets = {};
  function bridgeUrl(port) {
    var u = new URL(Module["ptServerUrl"], location.href);
    u.protocol = u.protocol === "https:" ? "wss:" : "ws:";
    u.searchParams.set("port", port);
    return u.href;
  }
  function remotePost(kind, link, port, bytes) {
    var s = sockets[link];
    if (kind === 1) {
      var ws;
      try {
        ws = new WebSocket(bridgeUrl(port));
      } catch (e) {
        receive([ 3, link, port, new Uint8Array(0) ]);
        return;
      }
      ws.binaryType = "arraybuffer";
      s = sockets[link] = {
        ws,
        open: false,
        queue: []
      };
      ws.onopen = function() {
        s.open = true;
        s.queue.splice(0).forEach(function(b) {
          ws.send(b);
        });
        receive([ 2, link, port, new Uint8Array(0) ]);
      };
      ws.onmessage = function(e) {
        receive([ 4, link, 0, new Uint8Array(e.data) ]);
      };
      ws.onclose = function() {
        var wasOpen = s.open;
        s.open = false;
        delete sockets[link];
        receive([ wasOpen ? 5 : 3, link, port, new Uint8Array(0) ]);
        if (!wasOpen) Module["printErr"]("[web] cannot reach the game server (" + bridgeUrl(port) + ")");
      };
    } else if (kind === 4 && s) {
      if (s.open) s.ws.send(bytes); else s.queue.push(bytes);
    } else if (kind === 5 && s) {
      s.ws.onclose = null;
      delete sockets[link];
      s.ws.close();
    }
  }
  // Saves of both the page and the server written to storage now (before an export)
  Module["ptFlushSaves"] = function() {
    var waits = [ PtSaves.flush() ];
    var w = Module["ptServerWorker"];
    if (w) waits.push(new Promise(function(resolve) {
      var id = ++flushId;
      flushWaits[id] = resolve;
      w.postMessage({
        saveFlush: id
      });
      setTimeout(resolve, 5e3);
    }));
    return Promise.all(waits);
  };
})();

// end include: C:/SunnyZ/PristonBuild/src/web/pre.js
var programArgs = [];

var thisProgram = "./this.program";

var quit_ = (status, toThrow) => {
  throw toThrow;
};

// In MODULARIZE mode _scriptName needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
var _scriptName = globalThis.document?.currentScript?.src;

if (typeof __filename != "undefined") {
  // Node
  _scriptName = __filename;
} else if (ENVIRONMENT_IS_WORKER) {
  _scriptName = self.location.href;
}

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = "";

function locateFile(path) {
  if (Module["locateFile"]) {
    return Module["locateFile"](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var readAsync, readBinary;

if (ENVIRONMENT_IS_NODE) {
  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require("node:fs");
  scriptDirectory = __dirname + "/";
  // include: node_shell_read.js
  readBinary = filename => {
    // We need to re-wrap `file://` strings to URLs.
    filename = isFileURI(filename) ? new URL(filename) : filename;
    var ret = fs.readFileSync(filename);
    return ret;
  };
  readAsync = async (filename, binary = true) => {
    // See the comment in the `readBinary` function.
    filename = isFileURI(filename) ? new URL(filename) : filename;
    var ret = fs.readFileSync(filename, binary ? undefined : "utf8");
    return ret;
  };
  // end include: node_shell_read.js
  if (process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, "/");
  }
  programArgs = process.argv.slice(2);
  // MODULARIZE will export the module in the proper place outside, we don't need to export here
  if (typeof module != "undefined") {
    module["exports"] = Module;
  }
  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };
} else // Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  try {
    scriptDirectory = new URL(".", _scriptName).href;
  } catch {}
  // Differentiate the Web Worker from the Node Worker case, as reading must
  // be done differently.
  if (!ENVIRONMENT_IS_NODE) {
    // include: web_or_worker_shell_read.js
    if (ENVIRONMENT_IS_WORKER) {
      readBinary = url => {
        var xhr = new XMLHttpRequest;
        xhr.open("GET", url, false);
        xhr.responseType = "arraybuffer";
        xhr.send(null);
        return new Uint8Array(/** @type{!ArrayBuffer} */ (xhr.response));
      };
    }
    readAsync = async url => {
      // Fetch has some additional restrictions over XHR, like it can't be used on a file:// url.
      // See https://github.com/github/fetch/pull/92#issuecomment-140665932
      // Cordova or Electron apps are typically loaded from a file:// url.
      // So use XHR on webview if URL is a file URL.
      if (isFileURI(url)) {
        return new Promise((resolve, reject) => {
          var xhr = new XMLHttpRequest;
          xhr.open("GET", url, true);
          xhr.responseType = "arraybuffer";
          xhr.onload = () => {
            if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) {
              // file URLs can return 0
              resolve(xhr.response);
              return;
            }
            reject(xhr.status);
          };
          xhr.onerror = reject;
          xhr.send(null);
        });
      }
      var response = await fetch(url, {
        credentials: "same-origin"
      });
      if (response.ok) {
        return response.arrayBuffer();
      }
      throw new Error(response.status + " : " + response.url);
    };
  }
} else {}

// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
// Normally just binding console.log/console.error here works fine, but
// under node (with workers) we see missing/out-of-order messages so route
// directly to stdout and stderr.
// See https://github.com/emscripten-core/emscripten/issues/14804
var defaultPrint = console.log.bind(console);

var defaultPrintErr = console.error.bind(console);

if (ENVIRONMENT_IS_NODE) {
  var utils = require("node:util");
  var stringify = a => typeof a == "object" ? utils.inspect(a) : a;
  defaultPrint = (...args) => fs.writeSync(1, args.map(stringify).join(" ") + "\n");
  defaultPrintErr = (...args) => fs.writeSync(2, args.map(stringify).join(" ") + "\n");
}

var out = defaultPrint;

var err = defaultPrintErr;

// end include: shell.js
// include: preamble.js
// === Preamble library stuff ===
// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html
var wasmBinary;

// Wasm globals
// For sending to workers.
var wasmModule;

//========================================
// Runtime essentials
//========================================
// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

// In STRICT mode, we only define assert() when ASSERTIONS is set.  i.e. we
// don't define it at all in release modes.  This matches the behaviour of
// MINIMAL_RUNTIME.
// TODO(sbc): Make this the default even without STRICT enabled.
/** @type {function(*, string=)} */ function assert(condition, text) {
  if (!condition) {
    // This build was created without ASSERTIONS defined.  `assert()` should not
    // ever be called in this configuration but in case there are callers in
    // the wild leave this simple abort() implementation here for now.
    abort(text);
  }
}

/**
 * Indicates whether filename is delivered via file protocol (as opposed to http/https)
 * @noinline
 */ var isFileURI = filename => filename.startsWith("file://");

// include: runtime_common.js
// include: runtime_exceptions.js
// Base Emscripten EH error class
class EmscriptenEH {}

class EmscriptenSjLj extends EmscriptenEH {}

// end include: runtime_exceptions.js
// include: runtime_debug.js
// end include: runtime_debug.js
// include: runtime_stack_check.js
const stackCookie1 = 34821223;

const stackCookie2 = 2310721022;

// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
  // If the stack ends at address zero we write our cookies 4 bytes into the
  // stack.  This prevents interference with SAFE_HEAP and ASAN which also
  // monitor writes to address zero.
  if (max == 0) {
    max += 4;
  }
  // The stack grow downwards towards _emscripten_stack_get_end.
  // We write cookies to the final two words in the stack and detect if they are
  // ever overwritten.
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((max) >>> 2) >>> 0, ___asan_storeN)] = stackCookie1;
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((max) + (4)) >>> 2) >>> 0, ___asan_storeN)] = stackCookie2;
}

function u32ToHexString(num) {
  return "0x" + (num >>> 0).toString(16).padStart(8, "0");
}

function checkStackCookie() {
  if (ABORT) return;
  var max = _emscripten_stack_get_end();
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var val1 = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((max) >>> 2) >>> 0, ___asan_loadN)];
  var val2 = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((max) + (4)) >>> 2) >>> 0, ___asan_loadN)];
  if (val1 != stackCookie1 || val2 != stackCookie2) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords ${u32ToHexString(stackCookie2)} and ${u32ToHexString(stackCookie1)}, but received ${u32ToHexString(val2)} ${u32ToHexString(val1)}`);
  }
}

// end include: runtime_stack_check.js
// include: runtime_asan.js
// C versions of asan_js_{load|store} will be used from compiled code, which have
// ASan instrumentation on them. However, until the wasm module is ready, we
// must access things directly.
function _asan_js_check_index(arr, index, asanFn) {
  if (runtimeInitialized && !runtimeExited) {
    const elemSize = arr.BYTES_PER_ELEMENT;
    asanFn(index * elemSize, elemSize);
  }
  return index;
}

// end include: runtime_asan.js
// Support for growable heap + pthreads, where the buffer may change, so JS views
// must be updated.
function growMemViews() {
  // `updateMemoryViews` updates all the views simultaneously, so it's enough to check any of them.
  if (wasmMemory.buffer != HEAP8.buffer) {
    updateMemoryViews();
  }
}

if (ENVIRONMENT_IS_NODE && (ENVIRONMENT_IS_PTHREAD)) {
  // Create as web-worker-like an environment as we can.
  globalThis.self = globalThis;
  var parentPort = worker_threads.parentPort;
  // Deno and Bun already have `postMessage` defined on the global scope and
  // deliver messages to `globalThis.onmessage`, so we must not duplicate that
  // behavior here if `postMessage` is already present.
  if (!globalThis.postMessage) {
    parentPort.on("message", msg => globalThis.onmessage?.({
      data: msg
    }));
    globalThis.postMessage = msg => parentPort.postMessage(msg);
  }
  // Node.js Workers do not pass postMessage()s and uncaught exception events to the parent
  // thread necessarily in the same order where they were generated in sequential program order.
  // See https://github.com/nodejs/node/issues/59617
  // To remedy this, capture all uncaughtExceptions in the Worker, and sequentialize those over
  // to the same postMessage pipe that other messages use.
  process.on("uncaughtException", err => {
    postMessage({
      cmd: 8,
      error: err
    });
    // Also shut down the Worker to match the same semantics as if this uncaughtException
    // handler was not registered.
    // (n.b. this will not shut down the whole Node.js app process, but just the Worker)
    process.exit(1);
  });
}

// include: runtime_pthread.js
// Pthread Web Worker handling code.
// This code runs only on pthread web workers and handles pthread setup
// and communication with the main thread via postMessage.
var startWorker;

if (ENVIRONMENT_IS_PTHREAD) {
  // Thread-local guard variable for one-time init of the JS state
  var initializedJS = false;
  // Turn unhandled rejected promises into errors so that the main thread will be
  // notified about them.
  self.onunhandledrejection = e => {
    throw e.reason || e;
  };
  function handleMessage(e) {
    try {
      var msgData = e.data;
      //dbg('msgData: ' + Object.keys(msgData));
      var cmd = msgData.cmd;
      if (cmd == 1) {
        // Preload command that is called once per worker to parse and load the Emscripten code.
        // Until we initialize the runtime, queue up any further incoming messages.
        let messageQueue = [];
        self.onmessage = e => messageQueue.push(e);
        // And add a callback for when the runtime is initialized.
        startWorker = () => {
          // Notify the main thread that this thread has loaded.
          postMessage({
            cmd: 3
          });
          // Process any messages that were queued before the thread was ready.
          for (let msg of messageQueue) {
            handleMessage(msg);
          }
          // Restore the real message handler.
          self.onmessage = handleMessage;
        };
        // Use `const` here to ensure that the variable is scoped only to
        // that iteration, allowing safe reference from a closure.
        for (const handler of msgData.handlers) {
          // If the main module has a handler for a certain event, but no
          // handler exists on the pthread worker, then proxy that handler
          // back to the main thread.
          if (!Module[handler] || Module[handler].proxy) {
            Module[handler] = (...args) => {
              postMessage({
                cmd: 9,
                handler,
                args
              });
            };
            // Rebind the out / err handlers if needed
            if (handler == "print") out = Module[handler];
            if (handler == "printErr") err = Module[handler];
          }
        }
        wasmMemory = msgData.wasmMemory;
        updateMemoryViews();
        wasmModule = msgData.wasmModule;
        createWasm();
        run();
        startWorker();
      } else if (cmd == 2) {
        // Call inside JS module to set up the stack frame for this pthread in JS module scope.
        // This needs to be the first thing that we do, as we cannot call to any C/C++ functions
        // until the thread stack is initialized.
        establishStackSpace(msgData.pthread_ptr);
        // Pass the thread address to wasm to store it for fast access.
        __emscripten_thread_init(msgData.pthread_ptr, /*is_main=*/ 0, /*is_runtime=*/ 0, /*can_block=*/ 1, 0, 0);
        PThread.threadInitTLS();
        // Await mailbox notifications with `Atomics.waitAsync` so we can start
        // using the fast `Atomics.notify` notification path.
        __emscripten_thread_mailbox_await(msgData.pthread_ptr);
        if (!initializedJS) {
          initializedJS = true;
        }
        try {
          invokeEntryPoint(msgData.start_routine, msgData.arg);
        } catch (ex) {
          if (ex != "unwind") {
            // The pthread "crashed".  Do not call `_emscripten_thread_exit` (which
            // would make this thread joinable).  Instead, re-throw the exception
            // and let the top level handler propagate it back to the main thread.
            throw ex;
          }
        }
      } else if (cmd == 4) {
        if (initializedJS) {
          checkMailbox();
        }
      } else if (cmd) {
        // The received message looks like something that should be handled by this message
        // handler, (since there is a cmd field present), but is not one of the
        // recognized commands:
        err(`worker: received unknown command ${cmd}`);
        err(msgData);
      }
    } catch (ex) {
      if (runtimeInitialized) __emscripten_thread_crashed();
      throw ex;
    }
  }
  self.onmessage = handleMessage;
}

// ENVIRONMENT_IS_PTHREAD
// end include: runtime_pthread.js
// Memory management
var runtimeInitialized = false;

var runtimeExited = false;

// When ALLOW_MEMORY_GROWTH is enabled, the conversion from Wasm
// memory to ArrayBuffer requires some additional logic.
function getMemoryBuffer() {
  return wasmMemory.buffer;
}

function updateMemoryViews() {
  // If we already have a heap that is resizeable/growable buffer we don't
  // need to do anything in updateMemoryViews.
  if (HEAP8?.buffer?.growable) return;
  var b = getMemoryBuffer();
  HEAP8 = new Int8Array(b);
  HEAP16 = new Int16Array(b);
  HEAPU8 = new Uint8Array(b);
  HEAPU16 = new Uint16Array(b);
  HEAP32 = new Int32Array(b);
  HEAPU32 = new Uint32Array(b);
  HEAPF32 = new Float32Array(b);
  HEAPF64 = new Float64Array(b);
  HEAP64 = new BigInt64Array(b);
}

// In non-standalone/normal mode, we create the memory here.
// include: runtime_init_memory.js
// Create the wasm memory. (Note: this only applies if IMPORTED_MEMORY is defined)
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
function initMemory() {
  if ((ENVIRONMENT_IS_PTHREAD)) {
    return;
  }
  {
    var INITIAL_MEMORY = 882049024;
    /** @suppress {checkTypes} */ wasmMemory = new WebAssembly.Memory({
      "initial": INITIAL_MEMORY / 65536,
      // In theory we should not need to emit the maximum if we want "unlimited"
      // or 4GB of memory, but VMs error on that atm, see
      // https://github.com/emscripten-core/emscripten/issues/14130
      // And in the pthreads case we definitely need to emit a maximum. So
      // always emit one.
      "maximum": 65536,
      "shared": true
    });
  }
  updateMemoryViews();
}

// end include: runtime_init_memory.js
// include: memoryprofiler.js
// end include: memoryprofiler.js
// end include: runtime_common.js
function preRun() {
  var preRun = Module["preRun"];
  if (preRun) {
    if (typeof preRun == "function") preRun = [ preRun ];
    onPreRuns.push(...preRun);
  }
  // Begin ATPRERUNS hooks
  callRuntimeCallbacks(onPreRuns);
}

function initRuntime() {
  runtimeInitialized = true;
  if (ENVIRONMENT_IS_PTHREAD) return;
  setStackLimits();
  checkStackCookie();
  // Begin ATINITS hooks
  if (!Module["noFSInit"] && !FS.initialized) FS.init();
  TTY.init();
  // End ATINITS hooks
  wasmExports["__wasm_call_ctors"]();
  // Begin ATPOSTCTORS hooks
  FS.ignorePermissions = false;
  // End ATPOSTCTORS hooks
  checkStackCookie();
}

function exitRuntime() {
  checkStackCookie();
  ___funcs_on_exit();
  // Native atexit() functions
  // Begin ATEXITS hooks
  callRuntimeCallbacks(onExits);
  FS.quit();
  TTY.shutdown();
  // End ATEXITS hooks
  PThread.terminateRuntime();
  runtimeExited = true;
}

function postRun() {
  checkStackCookie();
  var postRun = Module["postRun"];
  if (postRun) {
    if (typeof postRun == "function") postRun = [ postRun ];
    onPostRuns.push(...postRun);
  }
  // Begin ATPOSTRUNS hooks
  callRuntimeCallbacks(onPostRuns);
}

/**
 * @param {string|number=} what
 */ function abort(what) {
  Module["onAbort"]?.(what);
  what = `Aborted(${what})`;
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);
  ABORT = true;
  what += ". Build with -sASSERTIONS for more info.";
  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  // FIXME This approach does not work in Wasm EH because it currently does not assume
  // all RuntimeErrors are from traps; it decides whether a RuntimeError is from
  // a trap or not based on a hidden field within the object. So at the moment
  // we don't have a way of throwing a wasm trap from JS. TODO Make a JS API that
  // allows this in the wasm spec.
  // Suppress closure compiler warning here. Closure compiler's builtin extern
  // definition for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
  /** @suppress {checkTypes} */ var e = new WebAssembly.RuntimeError(what);
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

var wasmBinaryFile;

function findWasmBinary() {
  return locateFile("game_asan.wasm");
}

function getBinarySync(file) {
  if (readBinary) {
    return readBinary(file);
  }
  // Throwing a plain string here, even though it not normally advisable since
  // this gets turning into an `abort` in instantiateArrayBuffer.
  throw "both async and sync fetching of the wasm failed";
}

async function getWasmBinary(binaryFile) {
  // If we don't have the binary yet, load it asynchronously using readAsync.
  if (!wasmBinary) {
    // Fetch the binary using readAsync
    try {
      var response = await readAsync(binaryFile);
      return new Uint8Array(response);
    } catch {}
  }
  // Otherwise, getBinarySync should be able to get it synchronously
  return getBinarySync(binaryFile);
}

async function instantiateArrayBuffer(binaryFile, imports) {
  try {
    var binary = await getWasmBinary(binaryFile);
    var instance = await WebAssembly.instantiate(binary, imports);
    return instance;
  } catch (reason) {
    err(`failed to asynchronously prepare wasm: ${reason}`);
    abort(reason);
  }
}

async function instantiateAsync(binary, binaryFile, imports) {
  if (!binary && !isFileURI(binaryFile) && !ENVIRONMENT_IS_NODE) {
    try {
      var response = fetch(binaryFile, {
        credentials: "same-origin"
      });
      var instantiationResult = await WebAssembly.instantiateStreaming(response, imports);
      return instantiationResult;
    } catch (reason) {
      // We expect the most common failure cause to be a bad MIME type for the binary,
      // in which case falling back to ArrayBuffer instantiation should work.
      err(`wasm streaming compile failed: ${reason}`);
      err("falling back to ArrayBuffer instantiation");
    }
  }
  return instantiateArrayBuffer(binaryFile, imports);
}

function getWasmImports() {
  assignWasmImports();
  // prepare imports
  var imports = {
    "env": wasmImports,
    "wasi_snapshot_preview1": wasmImports
  };
  return imports;
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
async function createWasm() {
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  function receiveInstance(instance, module) {
    wasmExports = instance.exports;
    wasmExports = Asyncify.instrumentWasmExports(wasmExports);
    wasmExports = applySignatureConversions(wasmExports);
    registerTLSInit(wasmExports["_emscripten_tls_init"]);
    assignWasmExports(wasmExports);
    // We now have the Wasm module loaded up, keep a reference to the compiled module so we can post it to the workers.
    wasmModule = module;
    return wasmExports;
  }
  // Prefer streaming instantiation if available.
  function receiveInstantiationResult(result) {
    // 'result' is a ResultObject object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    return receiveInstance(result["instance"], result["module"]);
  }
  var info = getWasmImports();
  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  var instantiateWasm = Module["instantiateWasm"];
  if (instantiateWasm) {
    return new Promise(resolve => {
      instantiateWasm(info, (inst, mod) => resolve(receiveInstance(inst, mod)));
    });
  }
  if ((ENVIRONMENT_IS_PTHREAD)) {
    // Instantiate from the module that was received via postMessage from
    // the main thread. We can just use sync instantiation in the worker.
    var instance = new WebAssembly.Instance(wasmModule, getWasmImports());
    return receiveInstance(instance, wasmModule);
  }
  wasmBinaryFile ??= findWasmBinary();
  var result = await instantiateAsync(wasmBinary, wasmBinaryFile, info);
  var exports = receiveInstantiationResult(result);
  return exports;
}

// end include: preamble.js
// Begin JS library code
class ExitStatus {
  name="ExitStatus";
  constructor(status) {
    this.message = `Program terminated with exit(${status})`;
    this.status = status;
  }
}

/** @type {!Int32Array} */ var HEAP32;

/** @type {!Int8Array} */ var HEAP8;

/** @type {!Uint32Array} */ var HEAPU32;

var terminateWorker = worker => {
  worker.terminate();
  // terminate() can be asynchronous, so in theory the worker can continue
  // to run for some amount of time after termination.  However from our POV
  // the worker is now dead and we don't want to hear from it again, so we stub
  // out its message handler here.  This avoids having to check in each of
  // the onmessage handlers if the message was coming from a valid worker.
  worker.onmessage = e => {};
};

var cleanupThread = pthread_ptr => {
  var worker = PThread.pthreads[pthread_ptr];
  PThread.returnWorkerToPool(worker);
};

var callRuntimeCallbacks = callbacks => {
  while (callbacks.length > 0) {
    // Pass the module as the first argument.
    callbacks.shift()(Module);
  }
};

var onPreRuns = [];

var addOnPreRun = cb => onPreRuns.push(cb);

var dependenciesPromise = null;

var resolveRunDependencies = async () => dependenciesPromise;

var runDependencies = 0;

var dependenciesPromiseResolve = null;

var removeRunDependency = id => {
  runDependencies--;
  Module["monitorRunDependencies"]?.(runDependencies);
  if (!runDependencies) {
    dependenciesPromiseResolve();
  }
};

var addRunDependency = id => {
  if (!runDependencies) {
    dependenciesPromise = new Promise(resolve => dependenciesPromiseResolve = resolve);
  }
  runDependencies++;
  Module["monitorRunDependencies"]?.(runDependencies);
};

var spawnThread = threadParams => {
  var worker = PThread.getNewWorker();
  if (!worker) {
    // No available workers in the PThread pool.
    return 6;
  }
  // Add to pthreads map
  PThread.pthreads[threadParams.pthread_ptr] = worker;
  worker.pthread_ptr = threadParams.pthread_ptr;
  var msg = {
    cmd: 2,
    start_routine: threadParams.startRoutine,
    arg: threadParams.arg,
    pthread_ptr: threadParams.pthread_ptr
  };
  // Ask the worker to start executing its pthread entry point function.
  worker.postMessage(msg, threadParams.transferList);
  return 0;
};

var runtimeKeepaliveCounter = 0;

var keepRuntimeAlive = () => noExitRuntime || runtimeKeepaliveCounter > 0;

var stackSave = () => _emscripten_stack_get_current();

var stackRestore = val => __emscripten_stack_restore(val);

var stackAlloc = sz => __emscripten_stack_alloc(sz);

/** @type {!Float64Array} */ var HEAPF64;

/** not-@type {!BigInt64Array} */ var HEAP64;

/** @type{function(number, (number|boolean), ...number)} */ var proxyToMainThread = (funcIndex, emAsmAddr, proxyMode, ...callArgs) => {
  // EM_ASM proxying is done by passing a pointer to the address of the EM_ASM
  // content as `emAsmAddr`.  JS library proxying is done by passing an index
  // into `proxiedJSCallArgs` as `funcIndex`. If `emAsmAddr` is non-zero then
  // `funcIndex` will be ignored.
  // Additional arguments are passed after the first three are the actual
  // function arguments.
  // The serialization buffer contains the number of call params, and then
  // all the args here.
  // We also pass 'proxyMode' to C separately, since C needs to look at it.
  // Allocate a buffer (on the stack), which will be copied if necessary by
  // the C code.
  // First passed parameter specifies the number of arguments to the function.
  // When BigInt support is enabled, we must handle types in a more complex
  // way, detecting at runtime if a value is a BigInt or not (as we have no
  // type info here). To do that, add a "prefix" before each value that
  // indicates if it is a BigInt, which effectively doubles the number of
  // values we serialize for proxying. TODO: pack this?
  var bufSize = 8 * callArgs.length * 2;
  var sp = stackSave();
  var args = stackAlloc(bufSize);
  var b = ((args) >>> 3);
  for (var arg of callArgs) {
    if (typeof arg == "bigint") {
      // The prefix is non-zero to indicate a bigint.
      (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), b++ >>> 0, ___asan_storeN)] = 1n;
      (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), b++ >>> 0, ___asan_storeN)] = arg;
    } else {
      // The prefix is zero to indicate a JS Number.
      (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), b++ >>> 0, ___asan_storeN)] = 0n;
      (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), b++ >>> 0, ___asan_storeN)] = arg;
    }
  }
  var rtn = __emscripten_run_js_on_main_thread(funcIndex, emAsmAddr, bufSize, args, proxyMode);
  stackRestore(sp);
  return rtn;
};

function _proc_exit(code) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(0, 0, 1, code);
  EXITSTATUS = code;
  if (!keepRuntimeAlive()) {
    PThread.terminateAllThreads();
    Module["onExit"]?.(code);
    ABORT = true;
  }
  quit_(code, new ExitStatus(code));
}

function exitOnMainThread(returnCode) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(1, 0, 0, returnCode);
  _exit(returnCode);
}

/** @param {boolean|number=} implicit */ var exitJS = (status, implicit) => {
  EXITSTATUS = status;
  if (ENVIRONMENT_IS_PTHREAD) {
    // implicit exit can never happen on a pthread
    // When running in a pthread we propagate the exit back to the main thread
    // where it can decide if the whole process should be shut down or not.
    // The pthread may have decided not to exit its own runtime, for example
    // because it runs a main loop, but that doesn't affect the main thread.
    exitOnMainThread(status);
    throw "unwind";
  }
  if (!keepRuntimeAlive()) {
    exitRuntime();
  }
  _proc_exit(status);
};

var _exit = exitJS;

var waitAsyncPolyfilled = (!Atomics.waitAsync || (globalThis.navigator?.userAgent && Number((navigator.userAgent.match(/Chrom(e|ium)\/([0-9]+)\./) || [])[2]) < 91));

var PThread = {
  unusedWorkers: [],
  tlsInitFunctions: [],
  pthreads: {},
  init() {
    if ((!(ENVIRONMENT_IS_PTHREAD))) {
      PThread.initMainThread();
    }
  },
  initMainThread() {
    var pthreadPoolSize = 8;
    // Start loading up the Worker pool, if requested.
    while (pthreadPoolSize--) {
      PThread.allocateUnusedWorker();
    }
    // MINIMAL_RUNTIME takes care of calling loadWasmModuleToAllWorkers
    // in postamble_minimal.js
    addOnPreRun(async () => {
      var pthreadPoolReady = PThread.loadWasmModuleToAllWorkers();
      addRunDependency("loading-workers");
      await pthreadPoolReady;
      removeRunDependency("loading-workers");
    });
  },
  terminateAllThreads: () => {
    // Attempt to kill all workers.  Sadly (at least on the web) there is no
    // way to terminate a worker synchronously, or to be notified when a
    // worker is actually terminated.  This means there is some risk that
    // pthreads will continue to be executing after `worker.terminate` has
    // returned.  For this reason, we don't call `returnWorkerToPool` here or
    // free the underlying pthread data structures.
    for (var worker of Object.values(PThread.pthreads)) {
      terminateWorker(worker);
    }
    for (var worker of PThread.unusedWorkers) {
      terminateWorker(worker);
    }
    PThread.unusedWorkers = [];
    PThread.pthreads = {};
  },
  clearMailboxAwait: pthread_ptr => {
    if (!waitAsyncPolyfilled) {
      Atomics.notify((growMemViews(), HEAP32), ((pthread_ptr) >>> 2));
    }
  },
  terminateRuntime: () => {
    PThread.terminateAllThreads();
    var pthread_ptr = _pthread_self();
    ___set_thread_state(0, 0, 0, 1);
    PThread.clearMailboxAwait(pthread_ptr);
  },
  returnWorkerToPool: worker => {
    // We don't want to run main thread queued calls here, since we are doing
    // some operations that leave the worker queue in an invalid state until
    // we are completely done (it would be bad if free() ends up calling a
    // queued pthread_create which looks at the global data structures we are
    // modifying). To achieve that, defer the free() until the very end, when
    // we are all done.
    var pthread_ptr = worker.pthread_ptr;
    delete PThread.pthreads[pthread_ptr];
    // Note: worker is intentionally not terminated so the pool can
    // dynamically grow.
    PThread.unusedWorkers.push(worker);
    // Not a running Worker anymore
    // Detach the worker from the pthread object, and return it to the
    // worker pool as an unused worker.
    worker.pthread_ptr = 0;
    // Clear any pending waitAsync waiter armed on this thread's struct
    // BEFORE freeing the memory so that memory recycled by malloc in another
    // thread will not have a window where a stale async waiter is still active.
    PThread.clearMailboxAwait(pthread_ptr);
    // Finally, free the underlying (and now-unused) pthread structure in
    // linear memory.
    __emscripten_thread_free_data(pthread_ptr);
  },
  threadInitTLS() {
    // Call thread init functions (these are the _emscripten_tls_init for each
    // module loaded.
    PThread.tlsInitFunctions.forEach(f => f());
  },
  loadWasmModuleToWorker: worker => new Promise(onFinishedLoading => {
    worker.onmessage = e => {
      var d = e.data;
      var cmd = d.cmd;
      // If this message is intended to a recipient that is not the main
      // thread, forward it to the target thread. This is currently only
      // used by `CMD_CHECK_MAILBOX`.
      if (d.targetThread && d.targetThread != _pthread_self()) {
        var targetWorker = PThread.pthreads[d.targetThread];
        targetWorker?.postMessage(d);
        return;
      }
      if (d === "setimmediate" || d === "_si") {
        // Worker wants to postMessage() to itself to implement setImmediate()
        // emulation.
        worker.postMessage(d);
        return;
      }
      switch (cmd) {
       case 4:
        checkMailbox();
        break;

       case 5:
        spawnThread(d);
        break;

       case 6:
        // cleanupThread needs to be run via callUserCallback since it calls
        // back into user code to free thread data. Without this it's possible
        // the unwind or ExitStatus exception could escape here.
        callUserCallback(() => cleanupThread(d.thread));
        break;

       case 3:
        if (ENVIRONMENT_IS_NODE && !worker.strongref) {
          // Once worker is loaded & idle, mark it as weakly referenced,
          // so that mere existence of a Worker in the pool does not prevent
          // Node.js from exiting the app.
          worker.unref();
        }
        onFinishedLoading(worker);
        break;

       case 8:
        // Message handler for Node.js specific out-of-order behavior:
        // https://github.com/nodejs/node/issues/59617
        // A pthread sent an uncaught exception event. Re-raise it on the main thread.
        worker.onerror(d.error);
        break;

       case 9:
        Module[d.handler](...d.args);
        break;

       default:
        // The received message looks like something that should be handled by this message
        // handler, (since there is a e.data.cmd field present), but is not one of the
        // recognized commands:
        if (cmd) err(`worker sent an unknown command ${cmd}`);
      }
    };
    worker.onerror = e => {
      var message = "worker sent an error!";
      err(`${message} ${e.filename}:${e.lineno}: ${e.message}`);
      throw e;
    };
    if (ENVIRONMENT_IS_NODE) {
      worker.on("message", data => worker.onmessage({
        data
      }));
      worker.on("error", e => worker.onerror(e));
    }
    // When running on a pthread, none of the incoming parameters on the module
    // object are present. Proxy known handlers back to the main thread if specified.
    var handlers = [];
    var knownHandlers = [ "onExit", "onAbort", "print", "printErr" ];
    for (var handler of knownHandlers) {
      if (Module.propertyIsEnumerable(handler)) {
        handlers.push(handler);
      }
    }
    // Ask the new worker to load up the Emscripten-compiled page. This is a heavy operation.
    worker.postMessage({
      cmd: 1,
      handlers,
      wasmMemory,
      wasmModule
    });
  }),
  async loadWasmModuleToAllWorkers() {
    // Instantiation is synchronous in pthreads.
    if (ENVIRONMENT_IS_PTHREAD) {
      return;
    }
    let pthreadPoolReady = Promise.all(PThread.unusedWorkers.map(PThread.loadWasmModuleToWorker));
    return pthreadPoolReady;
  },
  allocateUnusedWorker() {
    var worker;
    var pthreadMainJs = _scriptName;
    worker = new Worker(pthreadMainJs, {
      // This is the way that we signal to the node worker that it is hosting
      // a pthread.
      "workerData": "em-pthread",
      // This is the way that we signal to the Web Worker that it is hosting
      // a pthread.
      "name": "em-pthread"
    });
    PThread.unusedWorkers.push(worker);
    return worker;
  },
  getNewWorker() {
    if (PThread.unusedWorkers.length == 0) {
      // PTHREAD_POOL_SIZE_STRICT should show a warning and, if set to level `2`, return from the function.
      var newWorker = PThread.allocateUnusedWorker();
      PThread.loadWasmModuleToWorker(newWorker);
    }
    return PThread.unusedWorkers.pop();
  }
};

var onPostRuns = [];

var addOnPostRun = cb => onPostRuns.push(cb);

var dynCalls = {};

var dynCallLegacy = (sig, ptr, args) => {
  sig = sig.replace(/p/g, "i");
  var f = dynCalls[sig];
  return f(ptr, ...args);
};

var dynCall = (sig, ptr, args = [], promising = false) => {
  var rtn = dynCallLegacy(sig, ptr, args);
  function convert(rtn) {
    return sig[0] == "p" ? rtn >>> 0 : rtn;
  }
  return convert(rtn);
};

function establishStackSpace(pthread_ptr) {
  var stackHigh = (growMemViews(), HEAPU32)[(((pthread_ptr) + (48)) >>> 2) >>> 0];
  var stackSize = (growMemViews(), HEAPU32)[(((pthread_ptr) + (52)) >>> 2) >>> 0];
  var stackLow = stackHigh - stackSize;
  // Set stack limits used by `emscripten/stack.h` function.  These limits are
  // cached in wasm-side globals to make checks as fast as possible.
  _emscripten_stack_set_limits(stackHigh, stackLow);
  setStackLimits();
  // Call inside wasm module to set up the stack frame for this pthread in wasm module scope
  stackRestore(stackHigh);
  // Write the stack cookie last, after we have set up the proper bounds and
  // current position of the stack.
  writeStackCookie();
}

var invokeEntryPoint = (ptr, arg) => {
  // An old thread on this worker may have been canceled without returning the
  // `runtimeKeepaliveCounter` to zero. Reset it now so the new thread won't
  // be affected.
  runtimeKeepaliveCounter = 0;
  // Same for noExitRuntime.  The default for pthreads should always be false
  // otherwise pthreads would never complete and attempts to pthread_join to
  // them would block forever.
  // pthreads can still choose to set `noExitRuntime` explicitly, or
  // call emscripten_unwind_to_js_event_loop to extend their lifetime beyond
  // their main function.  See comment in src/runtime_pthread.js for more.
  noExitRuntime = 0;
  // pthread entry points are always of signature 'void *ThreadMain(void *arg)'
  // Native codebases sometimes spawn threads with other thread entry point
  // signatures, such as void ThreadMain(void *arg), void *ThreadMain(), or
  // void ThreadMain().  That is not acceptable per C/C++ specification, but
  // x86 compiler ABI extensions enable that to work. If you find the
  // following line to crash, either change the signature to "proper" void
  // *ThreadMain(void *arg) form, or try linking with the Emscripten linker
  // flag -sEMULATE_FUNCTION_POINTER_CASTS to add in emulation for this x86
  // ABI extension.
  var result = ((a1 => dynCall_ii(ptr, a1)))(arg);
  checkStackCookie();
  function finish(result) {
    // In MINIMAL_RUNTIME the noExitRuntime concept does not apply to
    // pthreads. To exit a pthread with live runtime, use the function
    // emscripten_unwind_to_js_event_loop() in the pthread body.
    if (keepRuntimeAlive()) {
      EXITSTATUS = result;
      return;
    }
    __emscripten_thread_exit(result);
  }
  finish(result);
};

var noExitRuntime = false;

function ptrToString(ptr) {
  // Convert to 32-bit unsigned value
  ptr >>>= 0;
  return "0x" + ptr.toString(16).padStart(8, "0");
}

var registerTLSInit = tlsInitFunc => PThread.tlsInitFunctions.push(tlsInitFunc);

var setStackLimits = () => {
  var stackLow = _emscripten_stack_get_base();
  var stackHigh = _emscripten_stack_get_end();
  ___set_stack_limits(stackLow, stackHigh);
};

var wasmMemory;

var INT53_MAX = 9007199254740992;

var INT53_MIN = -9007199254740992;

var bigintToI53Checked = num => (num < INT53_MIN || num > INT53_MAX) ? NaN : Number(num);

var UTF8Decoder = globalThis.TextDecoder && new TextDecoder;

/**
   * heapOrArray is either a regular array, or a JavaScript typed array view.
   * @param {number} idx
   * @param {number=} maxBytesToRead
   * @param {boolean=} ignoreNul
   * @return {number}
   */ var findStringEnd = (heapOrArray, idx, maxBytesToRead, ignoreNul) => {
  var maxIdx = idx + maxBytesToRead;
  if (ignoreNul) return maxIdx;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on
  // null terminator by itself.
  // As a tiny code save trick, compare idx against maxIdx using a negation,
  // so that maxBytesToRead=undefined/NaN means Infinity.
  while (heapOrArray[idx] && !(idx >= maxIdx)) ++idx;
  return idx;
};

/**
   * Given a pointer 'idx' to a null-terminated UTF8-encoded string in the given
   * array that contains uint8 values, returns a copy of that string as a
   * Javascript String object.
   * heapOrArray is either a regular array, or a JavaScript typed array view.
   * @param {number=} idx
   * @param {number=} maxBytesToRead
   * @param {boolean=} ignoreNul - If true, the function will not stop on a NUL character.
   * @return {string}
   */ var UTF8ArrayToString = (heapOrArray, idx = 0, maxBytesToRead, ignoreNul) => {
  idx >>>= 0;
  var endPtr = findStringEnd(heapOrArray, idx, maxBytesToRead, ignoreNul);
  // When using conditional TextDecoder, skip it for short strings as the overhead of the native call is not worth it.
  if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
    return UTF8Decoder.decode(heapOrArray.buffer instanceof ArrayBuffer ? heapOrArray.subarray(idx, endPtr) : heapOrArray.slice(idx, endPtr));
  }
  var str = "";
  while (idx < endPtr) {
    // For UTF8 byte structure, see:
    // http://en.wikipedia.org/wiki/UTF-8#Description
    // https://www.ietf.org/rfc/rfc2279.txt
    // https://tools.ietf.org/html/rfc3629
    var u0 = heapOrArray[idx++];
    if (!(u0 & 128)) {
      str += String.fromCharCode(u0);
      continue;
    }
    var u1 = heapOrArray[idx++] & 63;
    if ((u0 & 224) == 192) {
      str += String.fromCharCode(((u0 & 31) << 6) | u1);
      continue;
    }
    var u2 = heapOrArray[idx++] & 63;
    if ((u0 & 240) == 224) {
      u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
    } else {
      u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
    }
    if (u0 < 65536) {
      str += String.fromCharCode(u0);
    } else {
      var ch = u0 - 65536;
      str += String.fromCharCode(55296 | (ch >> 10), 56320 | (ch & 1023));
    }
  }
  return str;
};

/** @type {!Uint8Array} */ var HEAPU8;

/**
   * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
   * emscripten HEAP, returns a copy of that string as a Javascript String object.
   *
   * @param {number} ptr
   * @param {number=} maxBytesToRead - An optional length that specifies the
   *   maximum number of bytes to read. You can omit this parameter to scan the
   *   string until the first 0 byte. If maxBytesToRead is passed, and the string
   *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
   *   string will cut short at that byte index.
   * @param {boolean=} ignoreNul - If true, the function will not stop on a NUL character.
   * @return {string}
   */ var UTF8ToString = (ptr, maxBytesToRead, ignoreNul) => {
  ptr >>>= 0;
  return ptr ? UTF8ArrayToString((growMemViews(), HEAPU8), ptr, maxBytesToRead, ignoreNul) : "";
};

function ___assert_fail(condition, filename, line, func) {
  condition >>>= 0;
  filename >>>= 0;
  func >>>= 0;
  return abort(`Assertion failed: ${UTF8ToString(condition)}, at: ` + [ filename ? UTF8ToString(filename) : "unknown filename", line, func ? UTF8ToString(func) : "unknown function" ]);
}

var ___call_sighandler = function(fp, sig) {
  fp >>>= 0;
  return ((a1 => dynCall_vi(fp, a1)))(sig);
};

class ExceptionInfo {
  // excPtr - Thrown object pointer to wrap. Metadata pointer is calculated from it.
  constructor(excPtr) {
    this.excPtr = excPtr;
    this.ptr = excPtr - 24;
  }
  set_type(type) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((this.ptr) + (4)) >>> 2) >>> 0, ___asan_storeN)] = type;
  }
  get_type() {
    return (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((this.ptr) + (4)) >>> 2) >>> 0, ___asan_loadN)];
  }
  set_destructor(destructor) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((this.ptr) + (8)) >>> 2) >>> 0, ___asan_storeN)] = destructor;
  }
  get_destructor() {
    return (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((this.ptr) + (8)) >>> 2) >>> 0, ___asan_loadN)];
  }
  set_caught(caught) {
    caught = caught ? 1 : 0;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (this.ptr) + (12) >>> 0, ___asan_storeN)] = caught;
  }
  get_caught() {
    return (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (this.ptr) + (12) >>> 0, ___asan_loadN)] != 0;
  }
  set_rethrown(rethrown) {
    rethrown = rethrown ? 1 : 0;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (this.ptr) + (13) >>> 0, ___asan_storeN)] = rethrown;
  }
  get_rethrown() {
    return (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (this.ptr) + (13) >>> 0, ___asan_loadN)] != 0;
  }
  // Initialize native structure fields. Should be called once after allocated.
  init(type, destructor) {
    this.set_adjusted_ptr(0);
    this.set_type(type);
    this.set_destructor(destructor);
  }
  set_adjusted_ptr(adjustedPtr) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((this.ptr) + (16)) >>> 2) >>> 0, ___asan_storeN)] = adjustedPtr;
  }
  get_adjusted_ptr() {
    return (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((this.ptr) + (16)) >>> 2) >>> 0, ___asan_loadN)];
  }
}

var uncaughtExceptionCount = 0;

var __Unwind_RaiseException = ex => {
  abort();
};

function ___cxa_throw(ptr, type, destructor) {
  ptr >>>= 0;
  type >>>= 0;
  destructor >>>= 0;
  var info = new ExceptionInfo(ptr);
  // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
  info.init(type, destructor);
  uncaughtExceptionCount++;
  __Unwind_RaiseException(ptr);
}

function ___handle_stack_overflow(requested) {
  requested >>>= 0;
  var base = _emscripten_stack_get_base();
  var end = _emscripten_stack_get_end();
  abort(`stack overflow (Attempt to set SP to ${ptrToString(requested)}` + `, with stack limits [${ptrToString(end)} - ${ptrToString(base)}` + "]). If you require more stack space build with -sSTACK_SIZE=<bytes>");
}

function pthreadCreateProxied(pthread_ptr, attr, startRoutine, arg) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(2, 0, 1, pthread_ptr, attr, startRoutine, arg);
  return ___pthread_create_js(pthread_ptr, attr, startRoutine, arg);
}

var _emscripten_has_threading_support = () => !!globalThis.SharedArrayBuffer;

function ___pthread_create_js(pthread_ptr, attr, startRoutine, arg) {
  pthread_ptr >>>= 0;
  attr >>>= 0;
  startRoutine >>>= 0;
  arg >>>= 0;
  if (!_emscripten_has_threading_support()) {
    return 6;
  }
  // List of JS objects that will transfer ownership to the Worker hosting the thread
  var transferList = [];
  var error = 0;
  // Synchronously proxy the thread creation to main thread if possible. If we
  // need to transfer ownership of objects, then proxy asynchronously via
  // postMessage.
  if (ENVIRONMENT_IS_PTHREAD && (!transferList.length || error)) {
    return pthreadCreateProxied(pthread_ptr, attr, startRoutine, arg);
  }
  // If on the main thread, and accessing Canvas/OffscreenCanvas failed, abort
  // with the detected error.
  if (error) return error;
  var threadParams = {
    startRoutine,
    pthread_ptr,
    arg,
    transferList
  };
  if (ENVIRONMENT_IS_PTHREAD) {
    // The prepopulated pool of web workers that can host pthreads is stored
    // in the main JS thread. Therefore if a pthread is attempting to spawn a
    // new thread, the thread creation must be deferred to the main JS thread.
    threadParams.cmd = 5;
    postMessage(threadParams, transferList);
    // When we defer thread creation this way, we have no way to detect thread
    // creation synchronously today, so we have to assume success and return 0.
    return 0;
  }
  // We are the main thread, so we have the pthread warmup pool in this
  // thread and can fire off JS thread creation directly ourselves.
  return spawnThread(threadParams);
}

var PATH = {
  isAbs: path => path.charAt(0) === "/",
  splitPath: filename => {
    var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
    return splitPathRe.exec(filename).slice(1);
  },
  normalizeArray: (parts, allowAboveRoot) => {
    // if the path tries to go above the root, `up` ends up > 0
    var up = 0;
    for (var i = parts.length - 1; i >= 0; i--) {
      var last = parts[i];
      if (last === ".") {
        parts.splice(i, 1);
      } else if (last === "..") {
        parts.splice(i, 1);
        up++;
      } else if (up) {
        parts.splice(i, 1);
        up--;
      }
    }
    // if the path is allowed to go above the root, restore leading ..s
    if (allowAboveRoot) {
      for (;up; up--) {
        parts.unshift("..");
      }
    }
    return parts;
  },
  normalize: path => {
    var isAbsolute = PATH.isAbs(path), trailingSlash = path.slice(-1) === "/";
    // Normalize the path
    path = PATH.normalizeArray(path.split("/").filter(p => !!p), !isAbsolute).join("/");
    if (!path && !isAbsolute) {
      path = ".";
    }
    if (path && trailingSlash) {
      path += "/";
    }
    return (isAbsolute ? "/" : "") + path;
  },
  dirname: path => {
    var result = PATH.splitPath(path), root = result[0], dir = result[1];
    if (!root && !dir) {
      // No dirname whatsoever
      return ".";
    }
    if (dir) {
      // It has a dirname, strip trailing slash
      dir = dir.slice(0, -1);
    }
    return root + dir;
  },
  basename: path => path && path.match(/([^\/]+|\/)\/*$/)[1],
  join: (...paths) => PATH.normalize(paths.join("/")),
  join2: (l, r) => PATH.normalize(l + "/" + r)
};

var initRandomFill = () => {
  // This block is not needed on v19+ since crypto.getRandomValues is builtin
  if (ENVIRONMENT_IS_NODE) {
    var nodeCrypto = require("node:crypto");
    return view => (nodeCrypto.randomFillSync(view), 0);
  }
  // like with most Web APIs, we can't use Web Crypto API directly on shared memory,
  // so we need to create an intermediate buffer and copy it to the destination
  return view => (view.set(crypto.getRandomValues(new Uint8Array(view.byteLength))), 
  0);
};

var randomFill = view => (randomFill = initRandomFill())(view);

var PATH_FS = {
  resolve: (...args) => {
    var resolvedPath = "", resolvedAbsolute = false;
    for (var i = args.length - 1; i >= -1 && !resolvedAbsolute; i--) {
      var path = (i >= 0) ? args[i] : FS.cwd();
      // Skip empty and invalid entries
      if (typeof path != "string") {
        throw new TypeError("Arguments to path.resolve must be strings");
      } else if (!path) {
        return "";
      }
      resolvedPath = path + "/" + resolvedPath;
      resolvedAbsolute = PATH.isAbs(path);
    }
    // At this point the path should be resolved to a full absolute path, but
    // handle relative paths to be safe (might happen when process.cwd() fails)
    resolvedPath = PATH.normalizeArray(resolvedPath.split("/").filter(p => !!p), !resolvedAbsolute).join("/");
    return ((resolvedAbsolute ? "/" : "") + resolvedPath) || ".";
  },
  relative: (from, to) => {
    from = PATH_FS.resolve(from).slice(1);
    to = PATH_FS.resolve(to).slice(1);
    function trim(arr) {
      var start = 0;
      for (;start < arr.length; start++) {
        if (arr[start] !== "") break;
      }
      var end = arr.length - 1;
      for (;end >= 0; end--) {
        if (arr[end] !== "") break;
      }
      if (start > end) return [];
      return arr.slice(start, end - start + 1);
    }
    var fromParts = trim(from.split("/"));
    var toParts = trim(to.split("/"));
    var length = Math.min(fromParts.length, toParts.length);
    var samePartsLength = length;
    for (var i = 0; i < length; i++) {
      if (fromParts[i] !== toParts[i]) {
        samePartsLength = i;
        break;
      }
    }
    var outputParts = [];
    for (var i = samePartsLength; i < fromParts.length; i++) {
      outputParts.push("..");
    }
    outputParts = outputParts.concat(toParts.slice(samePartsLength));
    return outputParts.join("/");
  }
};

var FS_stdin_getChar_buffer = [];

var lengthBytesUTF8 = str => {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
    // unit, not a Unicode code point of the character! So decode
    // UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var c = str.charCodeAt(i);
    // possibly a lead surrogate
    if (c <= 127) {
      len++;
    } else if (c <= 2047) {
      len += 2;
    } else if (c >= 55296 && c <= 57343) {
      len += 4;
      ++i;
    } else {
      len += 3;
    }
  }
  return len;
};

var stringToUTF8Array = (str, heap, outIdx, maxBytesToWrite) => {
  outIdx >>>= 0;
  // Parameter maxBytesToWrite is not optional. Negative values, 0, null,
  // undefined and false each don't write out any bytes.
  if (!(maxBytesToWrite > 0)) return 0;
  var startIdx = outIdx;
  var endIdx = outIdx + maxBytesToWrite - 1;
  // -1 for string null terminator.
  for (var i = 0; i < str.length; ++i) {
    // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description
    // and https://www.ietf.org/rfc/rfc2279.txt
    // and https://tools.ietf.org/html/rfc3629
    var u = str.codePointAt(i);
    if (u <= 127) {
      if (outIdx >= endIdx) break;
      heap[outIdx++ >>> 0] = u;
    } else if (u <= 2047) {
      if (outIdx + 1 >= endIdx) break;
      heap[outIdx++ >>> 0] = 192 | (u >> 6);
      heap[outIdx++ >>> 0] = 128 | (u & 63);
    } else if (u <= 65535) {
      if (outIdx + 2 >= endIdx) break;
      heap[outIdx++ >>> 0] = 224 | (u >> 12);
      heap[outIdx++ >>> 0] = 128 | ((u >> 6) & 63);
      heap[outIdx++ >>> 0] = 128 | (u & 63);
    } else {
      if (outIdx + 3 >= endIdx) break;
      heap[outIdx++ >>> 0] = 240 | (u >> 18);
      heap[outIdx++ >>> 0] = 128 | ((u >> 12) & 63);
      heap[outIdx++ >>> 0] = 128 | ((u >> 6) & 63);
      heap[outIdx++ >>> 0] = 128 | (u & 63);
      // Gotcha: if codePoint is over 0xFFFF, it is represented as a surrogate pair in UTF-16.
      // We need to manually skip over the second code unit for correct iteration.
      i++;
    }
  }
  // Null-terminate the pointer to the buffer.
  heap[outIdx >>> 0] = 0;
  return outIdx - startIdx;
};

/** @type {function(string, boolean=, number=)} */ var intArrayFromString = (stringy, dontAddNull, length) => {
  var len = length > 0 ? length : lengthBytesUTF8(stringy) + 1;
  var u8array = new Array(len);
  var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
  if (dontAddNull) u8array.length = numBytesWritten;
  return u8array;
};

var FS_stdin_getChar = () => {
  if (!FS_stdin_getChar_buffer.length) {
    var result = null;
    if (ENVIRONMENT_IS_NODE) {
      // we will read data by chunks of BUFSIZE
      var BUFSIZE = 256;
      var buf = Buffer.alloc(BUFSIZE);
      var bytesRead = 0;
      try {
        bytesRead = fs.readSync(process.stdin.fd, buf, 0, BUFSIZE);
      } catch (e) {
        // Cross-platform differences: on Windows, reading EOF throws an
        // exception, but on other OSes, reading EOF returns 0. Uniformize
        // behavior by treating the EOF exception to return 0.
        if (e.toString().includes("EOF")) bytesRead = 0; else throw e;
      }
      if (bytesRead > 0) {
        result = buf.toString("utf-8", 0, bytesRead);
      }
    } else if (globalThis.window?.prompt) {
      // Browser.
      result = window.prompt("Input: ");
      // returns null on cancel
      if (result !== null) {
        result += "\n";
      }
    } else {}
    if (!result) {
      return null;
    }
    FS_stdin_getChar_buffer = intArrayFromString(result, true);
  }
  return FS_stdin_getChar_buffer.shift();
};

var TTY = {
  ttys: [],
  init() {},
  shutdown() {},
  register(dev, ops) {
    TTY.ttys[dev] = {
      input: [],
      output: [],
      ops
    };
    FS.registerDevice(dev, TTY.stream_ops);
  },
  stream_ops: {
    open(stream) {
      var tty = TTY.ttys[stream.node.rdev];
      if (!tty) {
        throw new FS.ErrnoError(43);
      }
      stream.tty = tty;
      stream.seekable = false;
    },
    close(stream) {
      // flush any pending line data
      stream.tty.ops.fsync(stream.tty);
    },
    fsync(stream) {
      stream.tty.ops.fsync(stream.tty);
    },
    read(stream, buffer, offset, length, pos) {
      if (!stream.tty || !stream.tty.ops.get_char) {
        throw new FS.ErrnoError(60);
      }
      var bytesRead = 0;
      for (var i = 0; i < length; i++) {
        var result;
        try {
          result = stream.tty.ops.get_char(stream.tty);
        } catch (e) {
          throw new FS.ErrnoError(29);
        }
        if (result === undefined && !bytesRead) {
          throw new FS.ErrnoError(6);
        }
        if (result === null || result === undefined) break;
        bytesRead++;
        buffer[offset + i] = result;
        // We currently only support canonical mode (ICANON), where
        // read(2) returns as soon as a line delimiter is read.
        if (result === 10) break;
      }
      if (bytesRead) {
        stream.node.atime = Date.now();
      }
      return bytesRead;
    },
    write(stream, buffer, offset, length, pos) {
      if (!stream.tty || !stream.tty.ops.put_char) {
        throw new FS.ErrnoError(60);
      }
      try {
        for (var i = 0; i < length; i++) {
          stream.tty.ops.put_char(stream.tty, buffer[offset + i]);
        }
      } catch (e) {
        throw new FS.ErrnoError(29);
      }
      if (length) {
        stream.node.mtime = stream.node.ctime = Date.now();
      }
      return i;
    }
  },
  default_tty_ops: {
    get_char(tty) {
      return FS_stdin_getChar();
    },
    put_char(tty, val) {
      if (val === null || val === 10) {
        out(UTF8ArrayToString(tty.output));
        tty.output = [];
      } else {
        if (val != 0) tty.output.push(val);
      }
    },
    fsync(tty) {
      if (tty.output?.length > 0) {
        out(UTF8ArrayToString(tty.output));
        tty.output = [];
      }
    },
    ioctl_tcgets(tty) {
      // typical setting
      return {
        c_iflag: 25856,
        c_oflag: 5,
        c_cflag: 191,
        c_lflag: 35387,
        c_cc: [ 3, 28, 127, 21, 4, 0, 1, 0, 17, 19, 26, 0, 18, 15, 23, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
      };
    },
    ioctl_tcsets(tty, optional_actions, data) {
      // currently just ignore
      return 0;
    },
    ioctl_tiocgwinsz(tty) {
      return [ 24, 80 ];
    }
  },
  default_tty1_ops: {
    put_char(tty, val) {
      if (val === null || val === 10) {
        err(UTF8ArrayToString(tty.output));
        tty.output = [];
      } else {
        if (val != 0) tty.output.push(val);
      }
    },
    fsync(tty) {
      if (tty.output?.length > 0) {
        err(UTF8ArrayToString(tty.output));
        tty.output = [];
      }
    }
  }
};

var zeroMemory = (ptr, size) => (growMemViews(), HEAPU8).fill(0, ptr, ptr + size);

var alignMemory = (size, alignment) => Math.ceil(size / alignment) * alignment;

var mmapAlloc = size => {
  size = alignMemory(size, 65536);
  var ptr = _emscripten_builtin_memalign(65536, size);
  if (ptr) zeroMemory(ptr, size);
  return ptr;
};

var MEMFS = {
  ops_table: null,
  mount(mount) {
    return MEMFS.createNode(null, "/", 16895, 0);
  },
  createNode(parent, name, mode, dev = undefined) {
    if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
      // not supported
      throw new FS.ErrnoError(63);
    }
    MEMFS.ops_table ||= {
      dir: {
        node: {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr,
          lookup: MEMFS.node_ops.lookup,
          mknod: MEMFS.node_ops.mknod,
          rename: MEMFS.node_ops.rename,
          unlink: MEMFS.node_ops.unlink,
          rmdir: MEMFS.node_ops.rmdir,
          readdir: MEMFS.node_ops.readdir,
          symlink: MEMFS.node_ops.symlink
        },
        stream: {
          llseek: MEMFS.stream_ops.llseek
        }
      },
      file: {
        node: {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr
        },
        stream: {
          llseek: MEMFS.stream_ops.llseek,
          read: MEMFS.stream_ops.read,
          write: MEMFS.stream_ops.write,
          mmap: MEMFS.stream_ops.mmap,
          msync: MEMFS.stream_ops.msync
        }
      },
      link: {
        node: {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr,
          readlink: MEMFS.node_ops.readlink
        },
        stream: {}
      },
      chrdev: {
        node: {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr
        },
        stream: FS.chrdev_stream_ops
      }
    };
    var node = FS.createNode(parent, name, mode, dev);
    if (FS.isDir(node.mode)) {
      node.node_ops = MEMFS.ops_table.dir.node;
      node.stream_ops = MEMFS.ops_table.dir.stream;
      node.contents = {};
    } else if (FS.isFile(node.mode)) {
      node.node_ops = MEMFS.ops_table.file.node;
      node.stream_ops = MEMFS.ops_table.file.stream;
      // The actual number of bytes used in the typed array, as opposed to
      // contents.length which gives the whole capacity.
      node.usedBytes = 0;
      // The byte data of the file is stored in a typed array.
      // Note: typed arrays are not resizable like normal JS arrays are, so
      // there is a small penalty involved for appending file writes that
      // continuously grow a file similar to std::vector capacity vs used.
      node.contents = MEMFS.emptyFileContents ??= new Uint8Array(0);
    } else if (FS.isLink(node.mode)) {
      node.node_ops = MEMFS.ops_table.link.node;
      node.stream_ops = MEMFS.ops_table.link.stream;
    } else if (FS.isChrdev(node.mode)) {
      node.node_ops = MEMFS.ops_table.chrdev.node;
      node.stream_ops = MEMFS.ops_table.chrdev.stream;
    }
    node.atime = node.mtime = node.ctime = Date.now();
    // add the new node to the parent
    if (parent) {
      parent.contents[name] = node;
      parent.atime = parent.mtime = parent.ctime = node.atime;
    }
    return node;
  },
  getFileDataAsTypedArray(node) {
    return node.contents.subarray(0, node.usedBytes);
  },
  expandFileStorage(node, newCapacity) {
    var prevCapacity = node.contents.length;
    if (prevCapacity >= newCapacity) return;
    // No need to expand, the storage was already large enough.
    // Don't expand strictly to the given requested limit if it's only a very
    // small increase, but instead geometrically grow capacity.
    // For small filesizes (<1MB), perform size*2 geometric increase, but for
    // large sizes, do a much more conservative size*1.125 increase to avoid
    // overshooting the allocation cap by a very large margin.
    var CAPACITY_DOUBLING_MAX = 1024 * 1024;
    newCapacity = Math.max(newCapacity, (prevCapacity * (prevCapacity < CAPACITY_DOUBLING_MAX ? 2 : 1.125)) >>> 0);
    if (prevCapacity) newCapacity = Math.max(newCapacity, 256);
    // At minimum allocate 256b for each file when expanding.
    var oldContents = MEMFS.getFileDataAsTypedArray(node);
    node.contents = new Uint8Array(newCapacity);
    // Allocate new storage.
    node.contents.set(oldContents);
  },
  resizeFileStorage(node, newSize) {
    if (node.usedBytes == newSize) return;
    var oldContents = node.contents;
    node.contents = new Uint8Array(newSize);
    // Allocate new storage.
    node.contents.set(oldContents.subarray(0, Math.min(newSize, node.usedBytes)));
    // Copy old data over to the new storage.
    node.usedBytes = newSize;
  },
  node_ops: {
    getattr(node) {
      var attr = {};
      // device numbers reuse inode numbers.
      attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
      attr.ino = node.id;
      attr.mode = node.mode;
      attr.nlink = 1;
      attr.uid = 0;
      attr.gid = 0;
      attr.rdev = node.rdev;
      if (FS.isDir(node.mode)) {
        attr.size = 4096;
      } else if (FS.isFile(node.mode)) {
        attr.size = node.usedBytes;
      } else if (FS.isLink(node.mode)) {
        attr.size = node.link.length;
      } else {
        attr.size = 0;
      }
      attr.atime = new Date(node.atime);
      attr.mtime = new Date(node.mtime);
      attr.ctime = new Date(node.ctime);
      // A Date only holds whole milliseconds: also return the exact times
      // (e.g. as set by utimensat), which SYSCALLS.writeStat prefers.
      attr.atimeMs = node.atime;
      attr.mtimeMs = node.mtime;
      attr.ctimeMs = node.ctime;
      // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
      //       but this is not required by the standard.
      attr.blksize = 4096;
      attr.blocks = Math.ceil(attr.size / attr.blksize);
      return attr;
    },
    setattr(node, attr) {
      for (const key of [ "mode", "atime", "mtime", "ctime" ]) {
        if (attr[key] != null) {
          node[key] = attr[key];
        }
      }
      if (attr.size !== undefined) {
        MEMFS.resizeFileStorage(node, attr.size);
      }
    },
    lookup(parent, name) {
      // This error may happen quite a bit. To avoid overhead we reuse it (and
      // suffer a lack of stack info).
      if (!MEMFS.doesNotExistError) {
        MEMFS.doesNotExistError = new FS.ErrnoError(44);
        /** @suppress {checkTypes} */ MEMFS.doesNotExistError.stack = "<generic error, no stack>";
      }
      throw MEMFS.doesNotExistError;
    },
    mknod(parent, name, mode, dev = undefined) {
      return MEMFS.createNode(parent, name, mode, dev);
    },
    rename(old_node, new_dir, new_name) {
      var new_node;
      try {
        new_node = FS.lookupNode(new_dir, new_name);
      } catch (e) {}
      if (new_node && FS.isDir(old_node.mode)) {
        // if we're overwriting a directory at new_name, make sure it's empty.
        for (var i in new_node.contents) {
          throw new FS.ErrnoError(55);
        }
      }
      // do the internal rewiring
      delete old_node.parent.contents[old_node.name];
      new_dir.contents[new_name] = old_node;
      old_node.name = new_name;
      new_dir.ctime = new_dir.mtime = old_node.parent.ctime = old_node.parent.mtime = Date.now();
    },
    unlink(parent, name) {
      delete parent.contents[name];
      parent.ctime = parent.mtime = Date.now();
    },
    rmdir(parent, name) {
      var node = FS.lookupNode(parent, name);
      for (var i in node.contents) {
        throw new FS.ErrnoError(55);
      }
      delete parent.contents[name];
      parent.ctime = parent.mtime = Date.now();
    },
    readdir(node) {
      return [ ".", "..", ...Object.keys(node.contents) ];
    },
    symlink(parent, newname, oldpath) {
      var node = MEMFS.createNode(parent, newname, 511 | 40960, 0);
      node.link = oldpath;
      return node;
    },
    readlink(node) {
      if (!FS.isLink(node.mode)) {
        throw new FS.ErrnoError(28);
      }
      return node.link;
    }
  },
  stream_ops: {
    read(stream, buffer, offset, length, position) {
      var contents = stream.node.contents;
      if (position >= stream.node.usedBytes) return 0;
      var size = Math.min(stream.node.usedBytes - position, length);
      buffer.set(contents.subarray(position, position + size), offset);
      return size;
    },
    write(stream, buffer, offset, length, position, canOwn) {
      // If the buffer is located in main memory (HEAP), and if
      // memory can grow, we can't hold on to references of the
      // memory buffer, as they may get invalidated. That means we
      // need to copy its contents.
      if (buffer.buffer === (growMemViews(), HEAP8).buffer) {
        canOwn = false;
      }
      if (!length) return 0;
      var node = stream.node;
      node.mtime = node.ctime = Date.now();
      if (canOwn) {
        node.contents = buffer.subarray(offset, offset + length);
        node.usedBytes = length;
      } else if (!node.usedBytes && !position) {
        // If this is a simple first write to an empty file, do a fast set since we don't need to care about old data.
        node.contents = buffer.slice(offset, offset + length);
        node.usedBytes = length;
      } else {
        MEMFS.expandFileStorage(node, position + length);
        // Use typed array write which is available.
        node.contents.set(buffer.subarray(offset, offset + length), position);
        node.usedBytes = Math.max(node.usedBytes, position + length);
      }
      return length;
    },
    llseek(stream, offset, whence) {
      var position = offset;
      if (whence === 1) {
        position += stream.position;
      } else if (whence === 2) {
        if (FS.isFile(stream.node.mode)) {
          position += stream.node.usedBytes;
        }
      }
      if (position < 0) {
        throw new FS.ErrnoError(28);
      }
      return position;
    },
    mmap(stream, length, position, prot, flags) {
      if (!FS.isFile(stream.node.mode)) {
        throw new FS.ErrnoError(43);
      }
      var ptr;
      var allocated;
      var contents = stream.node.contents;
      // Only make a new copy when MAP_PRIVATE is specified.
      if (!(flags & 2) && contents.buffer === (growMemViews(), HEAP8).buffer) {
        // We can't emulate MAP_SHARED when the file is not backed by the
        // buffer we're mapping to (e.g. the HEAP buffer).
        allocated = false;
        ptr = contents.byteOffset;
      } else {
        allocated = true;
        ptr = mmapAlloc(length);
        if (!ptr) {
          throw new FS.ErrnoError(48);
        }
        if (contents) {
          // Try to avoid unnecessary slices.
          if (position > 0 || position + length < contents.length) {
            if (contents.subarray) {
              contents = contents.subarray(position, position + length);
            } else {
              contents = Array.prototype.slice.call(contents, position, position + length);
            }
          }
          (growMemViews(), HEAP8).set(contents, ptr >>> 0);
        }
      }
      return {
        ptr,
        allocated
      };
    },
    msync(stream, buffer, offset, length, mmapFlags) {
      MEMFS.stream_ops.write(stream, buffer, 0, length, offset, false);
      // should we check if bytesWritten and length are the same?
      return 0;
    }
  }
};

var FS_modeStringToFlags = str => {
  if (typeof str != "string") return str;
  var flagModes = {
    "r": 0,
    "r+": 2,
    "w": 512 | 64 | 1,
    "w+": 512 | 64 | 2,
    "a": 1024 | 64 | 1,
    "a+": 1024 | 64 | 2
  };
  var flags = flagModes[str];
  if (typeof flags == "undefined") {
    throw new Error(`Unknown file open mode: ${str}`);
  }
  return flags;
};

var FS_fileDataToTypedArray = data => {
  if (typeof data == "string") {
    data = intArrayFromString(data, true);
  }
  if (!data.subarray) {
    data = new Uint8Array(data);
  }
  return data;
};

var FS_getMode = (canRead, canWrite) => {
  var mode = 0;
  if (canRead) mode |= 292 | 73;
  if (canWrite) mode |= 146;
  return mode;
};

var asyncLoad = async url => {
  var arrayBuffer = await readAsync(url);
  return new Uint8Array(arrayBuffer);
};

var FS_createDataFile = (...args) => FS.createDataFile(...args);

var getUniqueRunDependency = id => id;

var preloadPlugins = [];

var FS_handledByPreloadPlugin = async (byteArray, fullname) => {
  // Ensure plugins are ready.
  if (typeof Browser != "undefined") Browser.init();
  for (var plugin of preloadPlugins) {
    if (plugin["canHandle"](fullname)) {
      return plugin["handle"](byteArray, fullname);
    }
  }
  // If no plugin handled this file then return the original/unmodified
  // byteArray.
  return byteArray;
};

var FS_preloadFile = async (parent, name, url, canRead, canWrite, dontCreateFile, canOwn, preFinish) => {
  // TODO we should allow people to just pass in a complete filename instead
  // of parent and name being that we just join them anyways
  var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
  var dep = getUniqueRunDependency(`cp ${fullname}`);
  // might have several active requests for the same fullname
  addRunDependency(dep);
  try {
    var byteArray = url;
    if (typeof url == "string") {
      byteArray = await asyncLoad(url);
    }
    byteArray = await FS_handledByPreloadPlugin(byteArray, fullname);
    preFinish?.();
    if (!dontCreateFile) {
      FS_createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
    }
  } finally {
    removeRunDependency(dep);
  }
};

var FS_createPreloadedFile = (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
  FS_preloadFile(parent, name, url, canRead, canWrite, dontCreateFile, canOwn, preFinish).then(onload).catch(onerror);
};

var FS = {
  root: null,
  mounts: [],
  devices: {},
  streams: [],
  nextInode: 1,
  nameTable: [],
  currentPath: "/",
  initialized: false,
  ignorePermissions: true,
  filesystems: null,
  syncFSRequests: 0,
  ErrnoError: class {
    name="ErrnoError";
    // We set the `name` property to be able to identify `FS.ErrnoError`
    // - the `name` is a standard ECMA-262 property of error objects. Kind of good to have it anyway.
    // - when using PROXYFS, an error can come from an underlying FS
    // as different FS objects have their own FS.ErrnoError each,
    // the test `err instanceof FS.ErrnoError` won't detect an error coming from another filesystem, causing bugs.
    // we'll use the reliable test `err.name == "ErrnoError"` instead
    constructor(errno) {
      this.errno = errno;
    }
  },
  FSStream: class {
    shared={};
    get object() {
      return this.node;
    }
    set object(val) {
      this.node = val;
    }
    get isRead() {
      return (this.flags & 2097155) !== 1;
    }
    get isWrite() {
      return (this.flags & 2097155) !== 0;
    }
    get isAppend() {
      return (this.flags & 1024);
    }
    get flags() {
      return this.shared.flags;
    }
    set flags(val) {
      this.shared.flags = val;
    }
    get position() {
      return this.shared.position;
    }
    set position(val) {
      this.shared.position = val;
    }
  },
  FSNode: class {
    node_ops={};
    stream_ops={};
    readMode=292 | 73;
    writeMode=146;
    mounted=null;
    constructor(parent, name, mode, rdev) {
      if (!parent) {
        parent = this;
      }
      this.parent = parent;
      this.mount = parent.mount;
      this.id = FS.nextInode++;
      this.name = name;
      this.mode = mode;
      this.rdev = rdev;
      this.atime = this.mtime = this.ctime = Date.now();
    }
    get read() {
      return (this.mode & this.readMode) === this.readMode;
    }
    set read(val) {
      val ? this.mode |= this.readMode : this.mode &= ~this.readMode;
    }
    get write() {
      return (this.mode & this.writeMode) === this.writeMode;
    }
    set write(val) {
      val ? this.mode |= this.writeMode : this.mode &= ~this.writeMode;
    }
    get isFolder() {
      return FS.isDir(this.mode);
    }
    get isDevice() {
      return FS.isChrdev(this.mode);
    }
    // The per-inode readiness wait-queue. The node carries a Set of listener
    // entries {cb}; producers (SOCKFS, PIPEFS) call notifyListeners on a
    // readiness transition, and poll()/epoll consume it. It lives on the node
    // (not the fd) so dup'd fds share one queue. Only nodes that derive real
    // readiness (sockets, pipes, and an epoll's own node) ever use this -
    // always-ready types (regular files, ttys) never register or notify.
    addListener(cb, exclusive = false) {
      var entry = {
        cb,
        exclusive
      };
      var listeners = (this.listeners ??= new Set);
      listeners.add(entry);
      return {
        listeners,
        entry
      };
    }
    notifyListeners(flags) {
      // Iterates the set without copying, which is safe ONLY under a
      // load-bearing contract that every internal listener must honour:
      //   1. A listener must not run user code synchronously (a poll waiter only
      //      resolves a Promise; an epoll registration only re-lists +
      //      re-notifies; the epoll callback only schedules a tick). User code
      //      runs on a later tick, never inside this loop.
      //   2. A listener may delete entries only from ITS OWN waiter, never from
      //      a sibling node's set that may be mid-iteration. (Deleting an entry
      //      of the set being iterated here is fine - a Set tolerates removal of
      //      a not-yet-visited entry mid-iteration; mutating a *different* node's
      //      set is fine because that set is not being iterated.)
      // Violating either gives silently skipped wakeups that are near-impossible
      // to reproduce. Any new producer/listener must preserve it.
      if (!this.listeners) return;
      // Fire every non-exclusive listener. Among EPOLLEXCLUSIVE registrations
      // (one fd watched by several epolls) wake only one, rotating round-robin
      // per node, to avoid a thundering herd. (Only epoll registrations are ever
      // exclusive; poll waiters and a node's own consumers are not.)
      var excl;
      for (var entry of this.listeners) {
        if (entry.exclusive) (excl ||= []).push(entry); else entry.cb(flags);
      }
      if (excl) {
        var i = (this.exclTurn || 0) % excl.length;
        this.exclTurn = i + 1;
        excl[i].cb(flags);
      }
    }
  },
  lookupPath(path, opts = {}) {
    if (!path) {
      throw new FS.ErrnoError(44);
    }
    opts.follow_mount ??= true;
    if (!PATH.isAbs(path)) {
      path = FS.cwd() + "/" + path;
    }
    // limit max consecutive symlinks to SYMLOOP_MAX.
    linkloop: for (var nlinks = 0; nlinks < 40; nlinks++) {
      // split the absolute path
      var parts = path.split("/").filter(p => !!p);
      // start at the root
      var current = FS.root;
      var current_path = "/";
      for (var i = 0; i < parts.length; i++) {
        var islast = (i === parts.length - 1);
        if (islast && opts.parent) {
          // stop resolving
          break;
        }
        if (parts[i] === ".") {
          continue;
        }
        if (parts[i] === "..") {
          current_path = PATH.dirname(current_path);
          if (FS.isRoot(current)) {
            path = current_path + "/" + parts.slice(i + 1).join("/");
            // We're making progress here, don't let many consecutive ..'s
            // lead to ELOOP
            nlinks--;
            continue linkloop;
          } else {
            current = current.parent;
          }
          continue;
        }
        current_path = PATH.join2(current_path, parts[i]);
        try {
          current = FS.lookupNode(current, parts[i]);
        } catch (e) {
          // if noent_okay is true, suppress a ENOENT in the last component
          // and return an object with an undefined node. This is needed for
          // resolving symlinks in the path when creating a file.
          if ((e?.errno === 44) && islast && opts.noent_okay) {
            return {
              path: current_path
            };
          }
          throw e;
        }
        // jump to the mount's root node if this is a mountpoint
        if (FS.isMountpoint(current) && (!islast || opts.follow_mount)) {
          current = current.mounted.root;
        }
        // by default, lookupPath will not follow a symlink if it is the final path component.
        // setting opts.follow = true will override this behavior.
        if (FS.isLink(current.mode) && (!islast || opts.follow)) {
          if (!current.node_ops.readlink) {
            throw new FS.ErrnoError(52);
          }
          var link = current.node_ops.readlink(current);
          if (!PATH.isAbs(link)) {
            link = PATH.dirname(current_path) + "/" + link;
          }
          path = link + "/" + parts.slice(i + 1).join("/");
          continue linkloop;
        }
      }
      return {
        path: current_path,
        node: current
      };
    }
    throw new FS.ErrnoError(32);
  },
  getPath(node) {
    var path;
    while (true) {
      if (FS.isRoot(node)) {
        var mount = node.mount.mountpoint;
        if (!path) return mount;
        return mount[mount.length - 1] !== "/" ? `${mount}/${path}` : mount + path;
      }
      path = path ? `${node.name}/${path}` : node.name;
      node = node.parent;
    }
  },
  hashName(parentid, name) {
    var hash = 0;
    for (var i = 0; i < name.length; i++) {
      hash = ((hash << 5) - hash + name.charCodeAt(i)) | 0;
    }
    return ((parentid + hash) >>> 0) % FS.nameTable.length;
  },
  hashAddNode(node) {
    var hash = FS.hashName(node.parent.id, node.name);
    node.name_next = FS.nameTable[hash];
    FS.nameTable[hash] = node;
  },
  hashRemoveNode(node) {
    var hash = FS.hashName(node.parent.id, node.name);
    if (FS.nameTable[hash] === node) {
      FS.nameTable[hash] = node.name_next;
    } else {
      var current = FS.nameTable[hash];
      while (current) {
        if (current.name_next === node) {
          current.name_next = node.name_next;
          break;
        }
        current = current.name_next;
      }
    }
  },
  lookupNode(parent, name) {
    var errCode = FS.mayLookup(parent);
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    var hash = FS.hashName(parent.id, name);
    for (var node = FS.nameTable[hash]; node; node = node.name_next) {
      var nodeName = node.name;
      if (node.parent.id === parent.id && nodeName === name) {
        return node;
      }
    }
    // if we failed to find it in the cache, call into the VFS
    return FS.lookup(parent, name);
  },
  createNode(parent, name, mode, rdev = undefined) {
    var node = new FS.FSNode(parent, name, mode, rdev);
    FS.hashAddNode(node);
    return node;
  },
  destroyNode(node) {
    FS.hashRemoveNode(node);
  },
  isRoot(node) {
    return node === node.parent;
  },
  isMountpoint(node) {
    return !!node.mounted;
  },
  isFile(mode) {
    return (mode & 61440) === 32768;
  },
  isDir(mode) {
    return (mode & 61440) === 16384;
  },
  isLink(mode) {
    return (mode & 61440) === 40960;
  },
  isChrdev(mode) {
    return (mode & 61440) === 8192;
  },
  isBlkdev(mode) {
    return (mode & 61440) === 24576;
  },
  isFIFO(mode) {
    return (mode & 61440) === 4096;
  },
  isSocket(mode) {
    return (mode & 49152) === 49152;
  },
  flagsToPermissionString(flag) {
    var perms = [ "r", "w", "rw" ][flag & 3];
    if ((flag & 512)) {
      perms += "w";
    }
    return perms;
  },
  nodePermissions(node, perms) {
    if (FS.ignorePermissions) {
      return 0;
    }
    // return 0 if any user, group or owner bits are set.
    if (perms.includes("r") && !(node.mode & 292)) {
      return 2;
    }
    if (perms.includes("w") && !(node.mode & 146)) {
      return 2;
    }
    if (perms.includes("x") && !(node.mode & 73)) {
      return 2;
    }
    return 0;
  },
  mayLookup(dir) {
    if (!FS.isDir(dir.mode)) return 54;
    var errCode = FS.nodePermissions(dir, "x");
    if (errCode) return errCode;
    if (!dir.node_ops.lookup) return 2;
    return 0;
  },
  mayCreate(dir, name) {
    if (!FS.isDir(dir.mode)) {
      return 54;
    }
    try {
      var node = FS.lookupNode(dir, name);
      return 20;
    } catch (e) {}
    return FS.nodePermissions(dir, "wx");
  },
  mayDelete(dir, name, isdir) {
    var node;
    try {
      node = FS.lookupNode(dir, name);
    } catch (e) {
      return e.errno;
    }
    var errCode = FS.nodePermissions(dir, "wx");
    if (errCode) {
      return errCode;
    }
    if (isdir) {
      if (!FS.isDir(node.mode)) {
        return 54;
      }
      if (FS.isRoot(node) || FS.getPath(node) === FS.cwd()) {
        return 10;
      }
    } else if (FS.isDir(node.mode)) {
      return 31;
    }
    return 0;
  },
  mayOpen(node, flags) {
    if (!node) {
      return 44;
    }
    if (FS.isLink(node.mode)) {
      return 32;
    }
    var mode = FS.flagsToPermissionString(flags);
    if (FS.isDir(node.mode)) {
      // opening for write
      // TODO: check for O_SEARCH? (== search for dir only)
      if (mode !== "r" || (flags & (512 | 64))) {
        return 31;
      }
    }
    return FS.nodePermissions(node, mode);
  },
  checkOpExists(op, err) {
    if (!op) {
      throw new FS.ErrnoError(err);
    }
    return op;
  },
  MAX_OPEN_FDS: 4096,
  nextfd() {
    for (var fd = 0; fd <= FS.MAX_OPEN_FDS; fd++) {
      if (!FS.streams[fd]) {
        return fd;
      }
    }
    throw new FS.ErrnoError(33);
  },
  getStreamChecked(fd) {
    var stream = FS.getStream(fd);
    if (!stream) {
      throw new FS.ErrnoError(8);
    }
    return stream;
  },
  getStream: fd => FS.streams[fd],
  createStream(stream, fd = -1) {
    // clone it, so we can return an instance of FSStream
    stream = Object.assign(new FS.FSStream, stream);
    if (fd == -1) {
      fd = FS.nextfd();
    }
    stream.fd = fd;
    FS.streams[fd] = stream;
    return stream;
  },
  closeStream(fd) {
    FS.streams[fd] = null;
  },
  dupStream(origStream, fd = -1) {
    var stream = FS.createStream(origStream, fd);
    stream.stream_ops?.dup?.(stream);
    return stream;
  },
  doSetAttr(stream, node, attr) {
    var setattr = stream?.stream_ops.setattr;
    var arg = setattr ? stream : node;
    setattr ??= node.node_ops.setattr;
    FS.checkOpExists(setattr, 63);
    try {
      setattr(arg, attr);
    } catch (e) {
      if (e instanceof RangeError) {
        throw new FS.ErrnoError(22);
      }
      throw e;
    }
  },
  chrdev_stream_ops: {
    open(stream) {
      var device = FS.getDevice(stream.node.rdev);
      // override node's stream ops with the device's
      stream.stream_ops = device.stream_ops;
      // forward the open call
      stream.stream_ops.open?.(stream);
    },
    llseek() {
      throw new FS.ErrnoError(70);
    }
  },
  major: dev => ((dev) >> 8),
  minor: dev => ((dev) & 255),
  makedev: (ma, mi) => ((ma) << 8 | (mi)),
  registerDevice(dev, ops) {
    FS.devices[dev] = {
      stream_ops: ops
    };
  },
  getDevice: dev => FS.devices[dev],
  getMounts(mount) {
    var mounts = [];
    var check = [ mount ];
    while (check.length) {
      var m = check.pop();
      mounts.push(m);
      check.push(...m.mounts);
    }
    return mounts;
  },
  syncfs(populate, callback) {
    if (typeof populate == "function") {
      callback = populate;
      populate = false;
    }
    FS.syncFSRequests++;
    if (FS.syncFSRequests > 1) {
      err(`warning: ${FS.syncFSRequests} FS.syncfs operations in flight at once, probably just doing extra work`);
    }
    var mounts = FS.getMounts(FS.root.mount);
    var completed = 0;
    function doCallback(errCode) {
      FS.syncFSRequests--;
      return callback(errCode);
    }
    function done(errCode) {
      if (errCode) {
        if (!done.errored) {
          done.errored = true;
          return doCallback(errCode);
        }
        return;
      }
      if (++completed >= mounts.length) {
        doCallback(null);
      }
    }
    // sync all mounts
    for (var mount of mounts) {
      if (mount.type.syncfs) {
        mount.type.syncfs(mount, populate, done);
      } else {
        done(null);
      }
    }
  },
  mount(type, opts, mountpoint) {
    var root = mountpoint === "/";
    var pseudo = !mountpoint;
    var node;
    if (root && FS.root) {
      throw new FS.ErrnoError(10);
    } else if (!root && !pseudo) {
      var lookup = FS.lookupPath(mountpoint, {
        follow_mount: false
      });
      mountpoint = lookup.path;
      // use the absolute path
      node = lookup.node;
      if (FS.isMountpoint(node)) {
        throw new FS.ErrnoError(10);
      }
      if (!FS.isDir(node.mode)) {
        throw new FS.ErrnoError(54);
      }
    }
    var mount = {
      type,
      opts,
      mountpoint,
      mounts: []
    };
    // create a root node for the fs
    var mountRoot = type.mount(mount);
    mountRoot.mount = mount;
    mount.root = mountRoot;
    if (root) {
      FS.root = mountRoot;
    } else if (node) {
      // set as a mountpoint
      node.mounted = mount;
      // add the new mount to the current mount's children
      if (node.mount) {
        node.mount.mounts.push(mount);
      }
    }
    return mountRoot;
  },
  unmount(mountpoint) {
    var lookup = FS.lookupPath(mountpoint, {
      follow_mount: false
    });
    if (!FS.isMountpoint(lookup.node)) {
      throw new FS.ErrnoError(28);
    }
    // destroy the nodes for this mount, and all its child mounts
    var node = lookup.node;
    var mount = node.mounted;
    var mounts = FS.getMounts(mount);
    for (var [hash, current] of Object.entries(FS.nameTable)) {
      while (current) {
        var next = current.name_next;
        if (mounts.includes(current.mount)) {
          FS.destroyNode(current);
        }
        current = next;
      }
    }
    // no longer a mountpoint
    node.mounted = null;
    // remove this mount from the child mounts
    var idx = node.mount.mounts.indexOf(mount);
    node.mount.mounts.splice(idx, 1);
  },
  lookup(parent, name) {
    return parent.node_ops.lookup(parent, name);
  },
  mknod(path, mode, dev) {
    var lookup = FS.lookupPath(path, {
      parent: true
    });
    var parent = lookup.node;
    var name = PATH.basename(path);
    if (!name) {
      throw new FS.ErrnoError(28);
    }
    if (name === "." || name === "..") {
      throw new FS.ErrnoError(20);
    }
    var errCode = FS.mayCreate(parent, name);
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    if (!parent.node_ops.mknod) {
      throw new FS.ErrnoError(63);
    }
    return parent.node_ops.mknod(parent, name, mode, dev);
  },
  statfs(path) {
    return FS.statfsNode(FS.lookupPath(path, {
      follow: true
    }).node);
  },
  statfsStream(stream) {
    // We keep a separate statfsStream function because noderawfs overrides
    // it. In noderawfs, stream.node is sometimes null. Instead, we need to
    // look at stream.path.
    return FS.statfsNode(stream.node);
  },
  statfsNode(node) {
    // NOTE: None of the defaults here are true. We're just returning safe and
    //       sane values. Currently nodefs and rawfs replace these defaults,
    //       other file systems leave them alone.
    var rtn = {
      bsize: 4096,
      frsize: 4096,
      blocks: 1e6,
      bfree: 5e5,
      bavail: 5e5,
      files: FS.nextInode,
      ffree: FS.nextInode - 1,
      fsid: 42,
      flags: 2,
      namelen: 255
    };
    if (node.node_ops.statfs) {
      Object.assign(rtn, node.node_ops.statfs(node.mount.opts.root));
    }
    return rtn;
  },
  create(path, mode = 438) {
    mode &= 4095;
    mode |= 32768;
    return FS.mknod(path, mode, 0);
  },
  mkdir(path, mode = 511) {
    mode &= 511 | 512;
    mode |= 16384;
    return FS.mknod(path, mode, 0);
  },
  mkdirTree(path, mode = 511) {
    var dirs = path.split("/");
    var d = "";
    for (var dir of dirs) {
      if (!dir) continue;
      if (d || PATH.isAbs(path)) d += "/";
      d += dir;
      try {
        FS.mkdir(d, mode);
      } catch (e) {
        if (e.errno != 20) throw e;
      }
    }
  },
  mkdev(path, mode, dev = undefined) {
    if (typeof dev == "undefined") {
      dev = mode;
      mode = 438;
    }
    mode |= 8192;
    return FS.mknod(path, mode, dev);
  },
  symlink(oldpath, newpath) {
    if (!PATH_FS.resolve(oldpath)) {
      throw new FS.ErrnoError(44);
    }
    var lookup = FS.lookupPath(newpath, {
      parent: true
    });
    var parent = lookup.node;
    if (!parent) {
      throw new FS.ErrnoError(44);
    }
    var newname = PATH.basename(newpath);
    var errCode = FS.mayCreate(parent, newname);
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    if (!parent.node_ops.symlink) {
      throw new FS.ErrnoError(63);
    }
    return parent.node_ops.symlink(parent, newname, oldpath);
  },
  link(oldpath, newpath, flags) {
    var lookup = FS.lookupPath(newpath, {
      parent: true
    });
    var parent = lookup.node;
    if (!parent) {
      throw new FS.ErrnoError(44);
    }
    var newname = PATH.basename(newpath);
    var errCode = FS.mayCreate(parent, newname);
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    // Hardlinks are only supported by filesystem backends that provide a
    // `link` node op (e.g. NODERAWFS backed by the host). NODEFS omits it:
    // a host hardlink cannot be confined to the mount root.
    if (!parent.node_ops.link) {
      throw new FS.ErrnoError(34);
    }
    return parent.node_ops.link(parent, newname, oldpath, flags);
  },
  rename(old_path, new_path) {
    var old_dirname = PATH.dirname(old_path);
    var new_dirname = PATH.dirname(new_path);
    var old_name = PATH.basename(old_path);
    var new_name = PATH.basename(new_path);
    // parents must exist
    var lookup, old_dir, new_dir;
    // let the errors from non existent directories percolate up
    lookup = FS.lookupPath(old_path, {
      parent: true
    });
    old_dir = lookup.node;
    lookup = FS.lookupPath(new_path, {
      parent: true
    });
    new_dir = lookup.node;
    if (!old_dir || !new_dir) throw new FS.ErrnoError(44);
    // need to be part of the same mount
    if (old_dir.mount !== new_dir.mount) {
      throw new FS.ErrnoError(75);
    }
    // source must exist
    var old_node = FS.lookupNode(old_dir, old_name);
    // old path should not be an ancestor of the new path
    var relative = PATH_FS.relative(old_path, new_dirname);
    if (relative.charAt(0) !== ".") {
      throw new FS.ErrnoError(28);
    }
    // new path should not be an ancestor of the old path
    relative = PATH_FS.relative(new_path, old_dirname);
    if (relative.charAt(0) !== ".") {
      throw new FS.ErrnoError(55);
    }
    // see if the new path already exists
    var new_node;
    try {
      new_node = FS.lookupNode(new_dir, new_name);
    } catch (e) {}
    // early out if nothing needs to change
    if (old_node === new_node) {
      return;
    }
    // we'll need to delete the old entry
    var isdir = FS.isDir(old_node.mode);
    var errCode = FS.mayDelete(old_dir, old_name, isdir);
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    // need delete permissions if we'll be overwriting.
    // need create permissions if new doesn't already exist.
    errCode = new_node ? FS.mayDelete(new_dir, new_name, isdir) : FS.mayCreate(new_dir, new_name);
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    if (!old_dir.node_ops.rename) {
      throw new FS.ErrnoError(63);
    }
    if (FS.isMountpoint(old_node) || (new_node && FS.isMountpoint(new_node))) {
      throw new FS.ErrnoError(10);
    }
    // if we are going to change the parent, check write permissions
    if (new_dir !== old_dir) {
      errCode = FS.nodePermissions(old_dir, "w");
      if (errCode) {
        throw new FS.ErrnoError(errCode);
      }
    }
    // remove the node from the lookup hash
    FS.hashRemoveNode(old_node);
    // do the underlying fs rename
    try {
      old_dir.node_ops.rename(old_node, new_dir, new_name);
      // The replaced node is stale now. Evict it only after the rename
      // succeeded: backends like NODEFS report node.id as st_ino.
      if (new_node) {
        FS.hashRemoveNode(new_node);
      }
      // update old node (we do this here to avoid each backend
      // needing to)
      old_node.parent = new_dir;
    } catch (e) {
      throw e;
    } finally {
      // add the node back to the hash (in case node_ops.rename
      // changed its name)
      FS.hashAddNode(old_node);
    }
  },
  rmdir(path) {
    var lookup = FS.lookupPath(path, {
      parent: true
    });
    var parent = lookup.node;
    var name = PATH.basename(path);
    var node = FS.lookupNode(parent, name);
    var errCode = FS.mayDelete(parent, name, true);
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    if (!parent.node_ops.rmdir) {
      throw new FS.ErrnoError(63);
    }
    if (FS.isMountpoint(node)) {
      throw new FS.ErrnoError(10);
    }
    parent.node_ops.rmdir(parent, name);
    FS.destroyNode(node);
  },
  readdir(path) {
    var lookup = FS.lookupPath(path, {
      follow: true
    });
    var node = lookup.node;
    var readdir = FS.checkOpExists(node.node_ops.readdir, 54);
    return readdir(node);
  },
  unlink(path) {
    var lookup = FS.lookupPath(path, {
      parent: true
    });
    var parent = lookup.node;
    if (!parent) {
      throw new FS.ErrnoError(44);
    }
    var name = PATH.basename(path);
    var node = FS.lookupNode(parent, name);
    var errCode = FS.mayDelete(parent, name, false);
    if (errCode) {
      // According to POSIX, we should map EISDIR to EPERM, but
      // we instead do what Linux does (and we must, as we use
      // the musl linux libc).
      throw new FS.ErrnoError(errCode);
    }
    if (!parent.node_ops.unlink) {
      throw new FS.ErrnoError(63);
    }
    if (FS.isMountpoint(node)) {
      throw new FS.ErrnoError(10);
    }
    parent.node_ops.unlink(parent, name);
    FS.destroyNode(node);
  },
  readlink(path) {
    var lookup = FS.lookupPath(path);
    var link = lookup.node;
    if (!link) {
      throw new FS.ErrnoError(44);
    }
    if (!link.node_ops.readlink) {
      throw new FS.ErrnoError(28);
    }
    return link.node_ops.readlink(link);
  },
  stat(path, dontFollow = false) {
    var lookup = FS.lookupPath(path, {
      follow: !dontFollow
    });
    var node = lookup.node;
    var getattr = FS.checkOpExists(node.node_ops.getattr, 63);
    return getattr(node);
  },
  fstat(fd) {
    var stream = FS.getStreamChecked(fd);
    var node = stream.node;
    var getattr = stream.stream_ops.getattr;
    var arg = getattr ? stream : node;
    getattr ??= node.node_ops.getattr;
    FS.checkOpExists(getattr, 63);
    return getattr(arg);
  },
  lstat(path) {
    return FS.stat(path, true);
  },
  doChmod(stream, node, mode, dontFollow = false) {
    FS.doSetAttr(stream, node, {
      mode: (mode & 4095) | (node.mode & ~4095),
      ctime: Date.now(),
      dontFollow
    });
  },
  chmod(path, mode, dontFollow = false) {
    var node;
    if (typeof path == "string") {
      var lookup = FS.lookupPath(path, {
        follow: !dontFollow
      });
      node = lookup.node;
    } else {
      node = path;
    }
    FS.doChmod(null, node, mode, dontFollow);
  },
  lchmod(path, mode) {
    FS.chmod(path, mode, true);
  },
  fchmod(fd, mode) {
    var stream = FS.getStreamChecked(fd);
    FS.doChmod(stream, stream.node, mode, false);
  },
  doChown(stream, node, dontFollow = false) {
    FS.doSetAttr(stream, node, {
      timestamp: Date.now(),
      dontFollow
    });
  },
  chown(path, uid, gid, dontFollow = false) {
    var node;
    if (typeof path == "string") {
      var lookup = FS.lookupPath(path, {
        follow: !dontFollow
      });
      node = lookup.node;
    } else {
      node = path;
    }
    FS.doChown(null, node, dontFollow);
  },
  lchown(path, uid, gid) {
    FS.chown(path, uid, gid, true);
  },
  fchown(fd, uid, gid) {
    var stream = FS.getStreamChecked(fd);
    FS.doChown(stream, stream.node, false);
  },
  doTruncate(stream, node, len) {
    if (FS.isDir(node.mode)) {
      throw new FS.ErrnoError(31);
    }
    if (!FS.isFile(node.mode)) {
      throw new FS.ErrnoError(28);
    }
    var errCode = FS.nodePermissions(node, "w");
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    FS.doSetAttr(stream, node, {
      size: len,
      timestamp: Date.now()
    });
  },
  truncate(path, len) {
    if (len < 0) {
      throw new FS.ErrnoError(28);
    }
    var node;
    if (typeof path == "string") {
      var lookup = FS.lookupPath(path, {
        follow: true
      });
      node = lookup.node;
    } else {
      node = path;
    }
    FS.doTruncate(null, node, len);
  },
  ftruncate(fd, len) {
    var stream = FS.getStreamChecked(fd);
    if (len < 0 || (stream.flags & 2097155) === 0) {
      throw new FS.ErrnoError(28);
    }
    FS.doTruncate(stream, stream.node, len);
  },
  utime(path, atime, mtime, dontFollow = false) {
    var lookup = FS.lookupPath(path, {
      follow: !dontFollow
    });
    FS.doSetAttr(null, lookup.node, {
      atime,
      mtime,
      dontFollow
    });
  },
  open(path, flags, mode = 438) {
    if (path === "") {
      throw new FS.ErrnoError(44);
    }
    flags = FS_modeStringToFlags(flags);
    if ((flags & 64)) {
      mode = (mode & 4095) | 32768;
    } else {
      mode = 0;
    }
    var node;
    var isDirPath;
    if (typeof path == "object") {
      node = path;
    } else {
      isDirPath = path.endsWith("/");
      // noent_okay makes it so that if the final component of the path
      // doesn't exist, lookupPath returns `node: undefined`. `path` will be
      // updated to point to the target of all symlinks.
      var lookup = FS.lookupPath(path, {
        follow: !(flags & 131072),
        noent_okay: true
      });
      node = lookup.node;
      path = lookup.path;
    }
    // perhaps we need to create the node
    var created = false;
    if ((flags & 64)) {
      if (node) {
        // if O_CREAT and O_EXCL are set, error out if the node already exists
        if ((flags & 128)) {
          throw new FS.ErrnoError(20);
        }
      } else if (isDirPath) {
        throw new FS.ErrnoError(31);
      } else {
        // node doesn't exist, try to create it
        // Ignore the permission bits here to ensure we can `open` this new
        // file below. We use chmod below to apply the permissions once the
        // file is open.
        node = FS.mknod(path, mode | 511, 0);
        created = true;
      }
    }
    if (!node) {
      throw new FS.ErrnoError(44);
    }
    // can't truncate a device
    if (FS.isChrdev(node.mode)) {
      flags &= ~512;
    }
    // if asked only for a directory, then this must be one
    if ((flags & 65536) && !FS.isDir(node.mode)) {
      throw new FS.ErrnoError(54);
    }
    // check permissions, if this is not a file we just created now (it is ok to
    // create and write to a file with read-only permissions; it is read-only
    // for later use)
    if (!created) {
      var errCode = FS.mayOpen(node, flags);
      if (errCode) {
        throw new FS.ErrnoError(errCode);
      }
    }
    // do truncation if necessary
    if ((flags & 512) && !created) {
      FS.truncate(node, 0);
    }
    // we've already handled these, don't pass down to the underlying vfs
    flags &= ~(128 | 512 | 131072);
    // register the stream with the filesystem
    var stream = FS.createStream({
      node,
      path: FS.getPath(node),
      // we want the absolute path to the node
      flags,
      seekable: true,
      position: 0,
      stream_ops: node.stream_ops,
      // used by the file family libc calls (fopen, fwrite, ferror, etc.)
      ungotten: [],
      error: false
    });
    // call the new stream's open function
    if (stream.stream_ops.open) {
      stream.stream_ops.open(stream);
    }
    if (created) {
      FS.chmod(node, mode & 511);
    }
    return stream;
  },
  close(stream) {
    if (FS.isClosed(stream)) {
      throw new FS.ErrnoError(8);
    }
    if (stream.getdents) stream.getdents = null;
    // free readdir state
    // The fd is going away: wake anything waiting on it (poll/epoll) with
    // POLLNVAL so a blocking wait unblocks and an epoll registration is evicted
    // on its next derive. Only sockets/pipes/epoll ever carry a wait-queue, so
    // for every other stream (incl. nodeless noderawfs stdio) this is a no-op.
    stream.node?.notifyListeners(32);
    try {
      if (stream.stream_ops.close) {
        stream.stream_ops.close(stream);
      }
    } catch (e) {
      throw e;
    } finally {
      FS.closeStream(stream.fd);
    }
    stream.fd = null;
  },
  isClosed(stream) {
    return stream.fd === null;
  },
  llseek(stream, offset, whence) {
    if (FS.isClosed(stream)) {
      throw new FS.ErrnoError(8);
    }
    if (!stream.seekable || !stream.stream_ops.llseek) {
      throw new FS.ErrnoError(70);
    }
    if (whence != 0 && whence != 1 && whence != 2) {
      throw new FS.ErrnoError(28);
    }
    stream.position = stream.stream_ops.llseek(stream, offset, whence);
    stream.ungotten = [];
    return stream.position;
  },
  read(stream, buffer, offset, length, position = undefined) {
    if (length < 0 || position < 0) {
      throw new FS.ErrnoError(28);
    }
    if (FS.isClosed(stream)) {
      throw new FS.ErrnoError(8);
    }
    if ((stream.flags & 2097155) === 1) {
      throw new FS.ErrnoError(8);
    }
    if (FS.isDir(stream.node.mode)) {
      throw new FS.ErrnoError(31);
    }
    if (!stream.stream_ops.read) {
      throw new FS.ErrnoError(28);
    }
    var seeking = typeof position != "undefined";
    if (!seeking) {
      position = stream.position;
    } else if (!stream.seekable) {
      throw new FS.ErrnoError(70);
    }
    var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
    if (!seeking) stream.position += bytesRead;
    return bytesRead;
  },
  write(stream, buffer, offset, length, position = undefined, canOwn = undefined) {
    if (length < 0 || position < 0) {
      throw new FS.ErrnoError(28);
    }
    if (FS.isClosed(stream)) {
      throw new FS.ErrnoError(8);
    }
    if ((stream.flags & 2097155) === 0) {
      throw new FS.ErrnoError(8);
    }
    if (FS.isDir(stream.node.mode)) {
      throw new FS.ErrnoError(31);
    }
    if (!stream.stream_ops.write) {
      throw new FS.ErrnoError(28);
    }
    if (stream.seekable && stream.flags & 1024) {
      // seek to the end before writing in append mode
      FS.llseek(stream, 0, 2);
    }
    var seeking = typeof position != "undefined";
    if (!seeking) {
      position = stream.position;
    } else if (!stream.seekable) {
      throw new FS.ErrnoError(70);
    }
    var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
    if (!seeking) stream.position += bytesWritten;
    return bytesWritten;
  },
  mmap(stream, length, position, prot, flags) {
    // User requests writing to file (prot & PROT_WRITE != 0).
    // Checking if we have permissions to write to the file unless
    // MAP_PRIVATE flag is set. According to POSIX spec it is possible
    // to write to file opened in read-only mode with MAP_PRIVATE flag,
    // as all modifications will be visible only in the memory of
    // the current process.
    if ((prot & 2) && !(flags & 2) && (stream.flags & 2097155) !== 2) {
      throw new FS.ErrnoError(2);
    }
    if ((stream.flags & 2097155) === 1) {
      throw new FS.ErrnoError(2);
    }
    if (!stream.stream_ops.mmap) {
      throw new FS.ErrnoError(43);
    }
    if (!length) {
      throw new FS.ErrnoError(28);
    }
    return stream.stream_ops.mmap(stream, length, position, prot, flags);
  },
  msync(stream, buffer, offset, length, mmapFlags) {
    if (!stream.stream_ops.msync) {
      return 0;
    }
    return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
  },
  ioctl(stream, cmd, arg) {
    if (!stream.stream_ops.ioctl) {
      throw new FS.ErrnoError(59);
    }
    return stream.stream_ops.ioctl(stream, cmd, arg);
  },
  readFile(path, opts = {}) {
    opts.flags ??= 0;
    opts.encoding ??= "binary";
    if (opts.encoding !== "utf8" && opts.encoding !== "binary") {
      abort(`Invalid encoding type "${opts.encoding}"`);
    }
    var stream = FS.open(path, opts.flags);
    var stat = FS.stat(path);
    var length = stat.size;
    var buf = new Uint8Array(length);
    FS.read(stream, buf, 0, length, 0);
    if (opts.encoding === "utf8") {
      buf = UTF8ArrayToString(buf);
    }
    FS.close(stream);
    return buf;
  },
  writeFile(path, data, opts = {}) {
    opts.flags ??= 577;
    var stream = FS.open(path, opts.flags, opts.mode);
    data = FS_fileDataToTypedArray(data);
    FS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn);
    FS.close(stream);
  },
  cwd: () => FS.currentPath,
  chdir(path) {
    var lookup = FS.lookupPath(path, {
      follow: true
    });
    if (lookup.node === null) {
      throw new FS.ErrnoError(44);
    }
    if (!FS.isDir(lookup.node.mode)) {
      throw new FS.ErrnoError(54);
    }
    var errCode = FS.nodePermissions(lookup.node, "x");
    if (errCode) {
      throw new FS.ErrnoError(errCode);
    }
    FS.currentPath = lookup.path;
  },
  createDefaultDirectories() {
    FS.mkdir("/tmp");
    FS.mkdir("/home");
    FS.mkdir("/home/web_user");
  },
  createDefaultDevices() {
    // create /dev
    FS.mkdir("/dev");
    // setup /dev/null
    FS.registerDevice(FS.makedev(1, 3), {
      read: () => 0,
      write: (stream, buffer, offset, length, pos) => length,
      llseek: () => 0
    });
    FS.mkdev("/dev/null", FS.makedev(1, 3));
    // setup /dev/tty and /dev/tty1
    // stderr needs to print output using err() rather than out()
    // so we register a second tty just for it.
    TTY.register(FS.makedev(5, 0), TTY.default_tty_ops);
    TTY.register(FS.makedev(6, 0), TTY.default_tty1_ops);
    FS.mkdev("/dev/tty", FS.makedev(5, 0));
    FS.mkdev("/dev/tty1", FS.makedev(6, 0));
    // setup /dev/[u]random
    // use a buffer to avoid overhead of individual crypto calls per byte
    var randomBuffer = new Uint8Array(1024), randomLeft = 0;
    var randomByte = () => {
      if (!randomLeft) {
        randomFill(randomBuffer);
        randomLeft = randomBuffer.byteLength;
      }
      return randomBuffer[--randomLeft];
    };
    FS.createDevice("/dev", "random", randomByte);
    FS.createDevice("/dev", "urandom", randomByte);
    // we're not going to emulate the actual shm device,
    // just create the tmp dirs that reside in it commonly
    FS.mkdir("/dev/shm");
    FS.mkdir("/dev/shm/tmp");
  },
  createSpecialDirectories() {
    // create /proc/self/fd which allows /proc/self/fd/6 => readlink gives the
    // name of the stream for fd 6 (see test_unistd_ttyname)
    FS.mkdir("/proc");
    var proc_self = FS.mkdir("/proc/self");
    FS.mkdir("/proc/self/fd");
    FS.mount({
      mount() {
        var node = FS.createNode(proc_self, "fd", 16895, 73);
        node.stream_ops = {
          llseek: MEMFS.stream_ops.llseek
        };
        node.node_ops = {
          lookup(parent, name) {
            var fd = +name;
            var stream = FS.getStreamChecked(fd);
            var ret = {
              parent: null,
              mount: {
                mountpoint: "fake"
              },
              node_ops: {
                readlink: () => stream.path
              },
              id: fd + 1
            };
            ret.parent = ret;
            // make it look like a simple root node
            return ret;
          },
          readdir() {
            return Array.from(FS.streams.entries()).filter(([k, v]) => v).map(([k, v]) => k.toString());
          }
        };
        return node;
      }
    }, {}, "/proc/self/fd");
  },
  createStandardStreams(input, output, error) {
    // TODO deprecate the old functionality of a single
    // input / output callback and that utilizes FS.createDevice
    // and instead require a unique set of stream ops
    // by default, we symlink the standard streams to the
    // default tty devices. however, if the standard streams
    // have been overwritten we create a unique device for
    // them instead.
    if (input) {
      FS.createDevice("/dev", "stdin", input);
    } else {
      FS.symlink("/dev/tty", "/dev/stdin");
    }
    if (output) {
      FS.createDevice("/dev", "stdout", null, output);
    } else {
      FS.symlink("/dev/tty", "/dev/stdout");
    }
    if (error) {
      FS.createDevice("/dev", "stderr", null, error);
    } else {
      FS.symlink("/dev/tty1", "/dev/stderr");
    }
    // open default streams for the stdin, stdout and stderr devices
    var stdin = FS.open("/dev/stdin", 0);
    var stdout = FS.open("/dev/stdout", 1);
    var stderr = FS.open("/dev/stderr", 1);
  },
  staticInit() {
    FS.nameTable.length = 4096;
    FS.mount(MEMFS, {}, "/");
    FS.createDefaultDirectories();
    FS.createDefaultDevices();
    FS.createSpecialDirectories();
    FS.filesystems = {
      "MEMFS": MEMFS
    };
  },
  init(input = undefined, output = undefined, error = undefined) {
    FS.initialized = true;
    // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
    input ??= Module["stdin"];
    output ??= Module["stdout"];
    error ??= Module["stderr"];
    FS.createStandardStreams(input, output, error);
  },
  quit() {
    FS.initialized = false;
    // force-flush all streams, so we get musl std streams printed out
    // close all of our streams
    for (var stream of FS.streams) {
      if (stream) {
        FS.close(stream);
      }
    }
  },
  analyzePath(path, dontResolveLastLink = false) {
    // operate from within the context of the symlink's target
    try {
      var lookup = FS.lookupPath(path, {
        follow: !dontResolveLastLink
      });
      path = lookup.path;
    } catch (e) {}
    var ret = {
      isRoot: false,
      exists: false,
      error: 0,
      name: null,
      path: null,
      object: null,
      parentExists: false,
      parentPath: null,
      parentObject: null
    };
    try {
      var lookup = FS.lookupPath(path, {
        parent: true
      });
      ret.parentExists = true;
      ret.parentPath = lookup.path;
      ret.parentObject = lookup.node;
      ret.name = PATH.basename(path);
      lookup = FS.lookupPath(path, {
        follow: !dontResolveLastLink
      });
      ret.exists = true;
      ret.path = lookup.path;
      ret.object = lookup.node;
      ret.name = lookup.node.name;
      ret.isRoot = lookup.path === "/";
    } catch (e) {
      ret.error = e.errno;
    }
    return ret;
  },
  createPath(parent, path, canRead = undefined, canWrite = undefined) {
    parent = typeof parent == "string" ? parent : FS.getPath(parent);
    var parts = path.split("/").reverse();
    while (parts.length) {
      var part = parts.pop();
      if (!part) continue;
      var current = PATH.join2(parent, part);
      try {
        FS.mkdir(current);
      } catch (e) {
        if (e.errno != 20) throw e;
      }
      parent = current;
    }
    return current;
  },
  createFile(parent, name, properties, canRead = undefined, canWrite = undefined) {
    var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
    var mode = FS_getMode(canRead, canWrite);
    return FS.create(path, mode);
  },
  createDataFile(parent, name, data = undefined, canRead = undefined, canWrite = undefined, canOwn = undefined) {
    var path = name;
    if (parent) {
      parent = typeof parent == "string" ? parent : FS.getPath(parent);
      path = name ? PATH.join2(parent, name) : parent;
    }
    var mode = FS_getMode(canRead, canWrite);
    var node = FS.create(path, mode);
    if (data) {
      data = FS_fileDataToTypedArray(data);
      // make sure we can write to the file
      FS.chmod(node, mode | 146);
      var stream = FS.open(node, 577);
      FS.write(stream, data, 0, data.length, 0, canOwn);
      FS.close(stream);
      FS.chmod(node, mode);
    }
  },
  createDevice(parent, name, input = undefined, output = undefined) {
    var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
    var mode = FS_getMode(!!input, !!output);
    FS.createDevice.major ??= 64;
    var dev = FS.makedev(FS.createDevice.major++, 0);
    // Create a fake device that a set of stream ops to emulate
    // the old behavior.
    FS.registerDevice(dev, {
      open(stream) {
        stream.seekable = false;
      },
      close(stream) {
        // flush any pending line data
        if (output?.buffer?.length) {
          output(10);
        }
      },
      read(stream, buffer, offset, length, pos) {
        var bytesRead = 0;
        for (var i = 0; i < length; i++) {
          var result;
          try {
            result = input();
          } catch (e) {
            throw new FS.ErrnoError(29);
          }
          if (result === undefined && !bytesRead) {
            throw new FS.ErrnoError(6);
          }
          if (result === null || result === undefined) break;
          bytesRead++;
          buffer[offset + i] = result;
        }
        if (bytesRead) {
          stream.node.atime = Date.now();
        }
        return bytesRead;
      },
      write(stream, buffer, offset, length, pos) {
        for (var i = 0; i < length; i++) {
          try {
            output(buffer[offset + i]);
          } catch (e) {
            throw new FS.ErrnoError(29);
          }
        }
        if (length) {
          stream.node.mtime = stream.node.ctime = Date.now();
        }
        return i;
      }
    });
    return FS.mkdev(path, mode, dev);
  },
  forceLoadFile(obj) {
    if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
    if (globalThis.XMLHttpRequest) {
      abort("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
    } else {
      // Command-line.
      try {
        obj.contents = readBinary(obj.url);
      } catch (e) {
        throw new FS.ErrnoError(29);
      }
    }
  },
  createLazyFile(parent, name, url, canRead = undefined, canWrite = undefined) {
    // Lazy chunked Uint8Array (implements get and length from Uint8Array).
    // Actual getting is abstracted away for eventual reuse.
    class LazyUint8Array {
      lengthKnown=false;
      chunks=[];
      // Loaded chunks. Index is the chunk number
      get(idx) {
        if (idx > this.length - 1 || idx < 0) {
          return undefined;
        }
        var chunkOffset = idx % this.chunkSize;
        var chunkNum = (idx / this.chunkSize) | 0;
        return this.getter(chunkNum)[chunkOffset];
      }
      setDataGetter(getter) {
        this.getter = getter;
      }
      cacheLength() {
        // Find length
        var xhr = new XMLHttpRequest;
        xhr.open("HEAD", url, false);
        xhr.send(null);
        if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) abort(`Couldn't load ${url}. Status: ${xhr.status}`);
        var datalength = Number(xhr.getResponseHeader("Content-length"));
        var header;
        var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
        var usesGzip = (header = xhr.getResponseHeader("Content-Encoding")) && header === "gzip";
        var chunkSize = 1024 * 1024;
        // Chunk size in bytes
        if (!hasByteServing) chunkSize = datalength;
        // Function to get a range from the remote URL.
        var doXHR = (from, to) => {
          if (from > to) abort(`invalid range (${from}, ${to}) or no bytes requested!`);
          if (to > datalength - 1) abort(`only ${datalength} bytes available! programmer error!`);
          // TODO: Use mozResponseArrayBuffer, responseStream, etc. if available.
          var xhr = new XMLHttpRequest;
          xhr.open("GET", url, false);
          if (datalength !== chunkSize) xhr.setRequestHeader("Range", `bytes=${from}-${to}`);
          // Some hints to the browser that we want binary data.
          xhr.responseType = "arraybuffer";
          if (xhr.overrideMimeType) {
            xhr.overrideMimeType("text/plain; charset=x-user-defined");
          }
          xhr.send(null);
          if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) abort(`Couldn't load ${url}. Status: ${xhr.status}`);
          if (xhr.response !== undefined) {
            return new Uint8Array(/** @type{Array<number>} */ (xhr.response || []));
          }
          return intArrayFromString(xhr.responseText ?? "", true);
        };
        var lazyArray = this;
        lazyArray.setDataGetter(chunkNum => {
          var start = chunkNum * chunkSize;
          var end = (chunkNum + 1) * chunkSize - 1;
          // including this byte
          end = Math.min(end, datalength - 1);
          // if datalength-1 is selected, this is the last block
          if (typeof lazyArray.chunks[chunkNum] == "undefined") {
            lazyArray.chunks[chunkNum] = doXHR(start, end);
          }
          if (typeof lazyArray.chunks[chunkNum] == "undefined") abort("doXHR failed!");
          return lazyArray.chunks[chunkNum];
        });
        if (usesGzip || !datalength) {
          // if the server uses gzip or doesn't supply the length, we have to download the whole file to get the (uncompressed) length
          chunkSize = datalength = 1;
          // this will force getter(0)/doXHR do download the whole file
          datalength = this.getter(0).length;
          chunkSize = datalength;
          out("LazyFiles on gzip forces download of the whole file when length is accessed");
        }
        this._length = datalength;
        this._chunkSize = chunkSize;
        this.lengthKnown = true;
      }
      get length() {
        if (!this.lengthKnown) {
          this.cacheLength();
        }
        return this._length;
      }
      get chunkSize() {
        if (!this.lengthKnown) {
          this.cacheLength();
        }
        return this._chunkSize;
      }
    }
    if (globalThis.XMLHttpRequest) {
      if (!ENVIRONMENT_IS_WORKER) abort("Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc");
      var lazyArray = new LazyUint8Array;
      var properties = {
        isDevice: false,
        contents: lazyArray
      };
    } else {
      var properties = {
        isDevice: false,
        url
      };
    }
    var node = FS.createFile(parent, name, properties, canRead, canWrite);
    // This is a total hack, but I want to get this lazy file code out of the
    // core of MEMFS. If we want to keep this lazy file concept I feel it should
    // be its own thin LAZYFS proxying calls to MEMFS.
    if (properties.contents) {
      node.contents = properties.contents;
    } else if (properties.url) {
      node.contents = null;
      node.url = properties.url;
    }
    // Add a function that defers querying the file size until it is asked the first time.
    Object.defineProperties(node, {
      usedBytes: {
        get: function() {
          return this.contents.length;
        }
      }
    });
    // override each stream op with one that tries to force load the lazy file first
    var stream_ops = {};
    for (const [key, fn] of Object.entries(node.stream_ops)) {
      stream_ops[key] = (...args) => {
        FS.forceLoadFile(node);
        return fn(...args);
      };
    }
    function writeChunks(stream, buffer, offset, length, position) {
      var contents = stream.node.contents;
      if (position >= contents.length) return 0;
      var size = Math.min(contents.length - position, length);
      if (contents.slice) {
        // normal array
        for (var i = 0; i < size; i++) {
          buffer[offset + i] = contents[position + i];
        }
      } else {
        for (var i = 0; i < size; i++) {
          // LazyUint8Array from sync binary XHR
          buffer[offset + i] = contents.get(position + i);
        }
      }
      return size;
    }
    // use a custom read function
    stream_ops.read = (stream, buffer, offset, length, position) => {
      FS.forceLoadFile(node);
      return writeChunks(stream, buffer, offset, length, position);
    };
    // use a custom mmap function
    stream_ops.mmap = (stream, length, position, prot, flags) => {
      FS.forceLoadFile(node);
      var ptr = mmapAlloc(length);
      if (!ptr) {
        throw new FS.ErrnoError(48);
      }
      writeChunks(stream, (growMemViews(), HEAP8), ptr, length, position);
      return {
        ptr,
        allocated: true
      };
    };
    node.stream_ops = stream_ops;
    return node;
  }
};

var SYSCALLS = {
  currentUmask: 18,
  calculateAt(dirfd, path, allowEmpty = false) {
    if (PATH.isAbs(path)) {
      return path;
    }
    // relative path
    var dir;
    if (dirfd === -100) {
      dir = FS.cwd();
    } else {
      var dirstream = SYSCALLS.getStreamFromFD(dirfd);
      dir = dirstream.path;
    }
    if (path.length == 0) {
      if (!allowEmpty) {
        throw new FS.ErrnoError(44);
      }
      return dir;
    }
    return dir + "/" + path;
  },
  writeStat(buf, stat) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((buf) >>> 2) >>> 0, ___asan_storeN)] = stat.dev;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (4)) >>> 2) >>> 0, ___asan_storeN)] = stat.mode;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (8)) >>> 2) >>> 0, ___asan_storeN)] = stat.nlink;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (12)) >>> 2) >>> 0, ___asan_storeN)] = stat.uid;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (16)) >>> 2) >>> 0, ___asan_storeN)] = stat.gid;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (20)) >>> 2) >>> 0, ___asan_storeN)] = stat.rdev;
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (24)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stat.size);
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((buf) + (32)) >>> 2) >>> 0, ___asan_storeN)] = 4096;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((buf) + (36)) >>> 2) >>> 0, ___asan_storeN)] = stat.blocks;
    // Prefer `*Ms` properties if available (e.g. from MEMFS, or NODEFS / host
    // `fs.Stats`) for sub-millisecond precision; fall back to Date#getTime for
    // other filesystems.
    var atime = stat.atimeMs ?? stat.atime.getTime();
    var mtime = stat.mtimeMs ?? stat.mtime.getTime();
    var ctime = stat.ctimeMs ?? stat.ctime.getTime();
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (40)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(Math.floor(atime / 1e3));
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (48)) >>> 2) >>> 0, ___asan_storeN)] = Math.floor((atime % 1e3) * 1e6);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (56)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(Math.floor(mtime / 1e3));
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (64)) >>> 2) >>> 0, ___asan_storeN)] = Math.floor((mtime % 1e3) * 1e6);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (72)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(Math.floor(ctime / 1e3));
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (80)) >>> 2) >>> 0, ___asan_storeN)] = Math.floor((ctime % 1e3) * 1e6);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (88)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stat.ino);
    return 0;
  },
  writeStatFs(buf, stats) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (4)) >>> 2) >>> 0, ___asan_storeN)] = stats.bsize;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (60)) >>> 2) >>> 0, ___asan_storeN)] = stats.bsize;
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (8)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stats.blocks);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (16)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stats.bfree);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (24)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stats.bavail);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (32)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stats.files);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((buf) + (40)) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stats.ffree);
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (48)) >>> 2) >>> 0, ___asan_storeN)] = stats.fsid;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (64)) >>> 2) >>> 0, ___asan_storeN)] = stats.flags;
    // ST_NOSUID
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buf) + (56)) >>> 2) >>> 0, ___asan_storeN)] = stats.namelen;
  },
  doMsync(addr, stream, len, flags, offset) {
    if (!FS.isFile(stream.node.mode)) {
      throw new FS.ErrnoError(43);
    }
    if (flags & 2) {
      // MAP_PRIVATE calls need not to be synced back to underlying fs
      return 0;
    }
    var buffer = (growMemViews(), HEAPU8).subarray(addr >>> 0, addr + len >>> 0);
    FS.msync(stream, buffer, offset, len, flags);
  },
  getStreamFromFD(fd) {
    var stream = FS.getStreamChecked(fd);
    return stream;
  },
  varargs: undefined,
  getStr(ptr) {
    var ret = UTF8ToString(ptr);
    return ret;
  }
};

function ___syscall_chdir(path) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(3, 0, 1, path);
  path >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    FS.chdir(path);
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_dup(fd) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(4, 0, 1, fd);
  try {
    var old = SYSCALLS.getStreamFromFD(fd);
    return FS.dupStream(old).fd;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_faccessat(dirfd, path, amode, flags) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(5, 0, 1, dirfd, path, amode, flags);
  path >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    if (amode & ~7) {
      // need a valid mode
      return -28;
    }
    var lookup = FS.lookupPath(path, {
      follow: true
    });
    var node = lookup.node;
    if (!node) {
      return -44;
    }
    var perms = "";
    if (amode & 4) perms += "r";
    if (amode & 2) perms += "w";
    if (amode & 1) perms += "x";
    if (perms && FS.nodePermissions(node, perms)) {
      return -2;
    }
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

var syscallGetVarargI = () => {
  // the `+` prepended here is necessary to convince the JSCompiler that varargs is indeed a number.
  var ret = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((+SYSCALLS.varargs) >>> 2) >>> 0, ___asan_loadN)];
  SYSCALLS.varargs += 4;
  return ret;
};

var syscallGetVarargP = syscallGetVarargI;

/** @type {!Int16Array} */ var HEAP16;

function ___syscall_fcntl64(fd, cmd, varargs) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(6, 0, 1, fd, cmd, varargs);
  varargs >>>= 0;
  SYSCALLS.varargs = varargs;
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    switch (cmd) {
     case 0:
      {
        var arg = syscallGetVarargI();
        if (arg < 0) {
          return -28;
        }
        while (FS.streams[arg]) {
          arg++;
        }
        var newStream;
        newStream = FS.dupStream(stream, arg);
        return newStream.fd;
      }

     case 1:
     case 2:
      return 0;

     // FD_CLOEXEC makes no sense for a single process.
      case 3:
      return stream.flags;

     case 4:
      {
        var arg = syscallGetVarargI();
        var mask = 289792;
        stream.flags = (stream.flags & ~mask) | (arg & mask);
        return 0;
      }

     case 12:
      {
        var arg = syscallGetVarargP();
        var offset = 0;
        // We're always unlocked.
        (growMemViews(), HEAP16)[_asan_js_check_index((growMemViews(), HEAP16), (((arg) + (offset)) >>> 1) >>> 0, ___asan_storeN)] = 2;
        return 0;
      }

     case 13:
     case 14:
      // Pretend that the locking is successful. These are process-level locks,
      // and Emscripten programs are a single process. If we supported linking a
      // filesystem between programs, we'd need to do more here.
      // See https://github.com/emscripten-core/emscripten/issues/23697
      return 0;
    }
    return -28;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_fstat64(fd, buf) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(7, 0, 1, fd, buf);
  buf >>>= 0;
  try {
    return SYSCALLS.writeStat(buf, FS.fstat(fd));
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

var stringToUTF8 = (str, outPtr, maxBytesToWrite) => stringToUTF8Array(str, (growMemViews(), 
HEAPU8), outPtr, maxBytesToWrite);

function ___syscall_getcwd(buf, size) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(8, 0, 1, buf, size);
  buf >>>= 0;
  size >>>= 0;
  try {
    if (!size) return -28;
    var cwd = FS.cwd();
    var cwdLengthInBytes = lengthBytesUTF8(cwd) + 1;
    if (size < cwdLengthInBytes) return -68;
    stringToUTF8(cwd, buf, size);
    return cwdLengthInBytes;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_getdents64(fd, dirp, count) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(9, 0, 1, fd, dirp, count);
  dirp >>>= 0;
  count >>>= 0;
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    stream.getdents ||= FS.readdir(stream.path);
    var struct_size = 280;
    var pos = 0;
    var off = FS.llseek(stream, 0, 1);
    var startIdx = Math.floor(off / struct_size);
    var endIdx = Math.min(stream.getdents.length, startIdx + Math.floor(count / struct_size));
    for (var idx = startIdx; idx < endIdx; idx++) {
      var id;
      var type;
      var name = stream.getdents[idx];
      if (name === ".") {
        id = stream.node.id;
        type = 4;
      } else if (name === "..") {
        var lookup = FS.lookupPath(stream.path, {
          parent: true
        });
        id = lookup.node.id;
        type = 4;
      } else {
        var child;
        try {
          child = FS.lookupNode(stream.node, name);
        } catch (e) {
          // If the entry is not a directory, file, or symlink, nodefs
          // lookupNode will raise EINVAL. Skip these and continue.
          if (e?.errno === 28) {
            continue;
          }
          throw e;
        }
        id = child.id;
        type = FS.isChrdev(child.mode) ? 2 : // character device.
        FS.isDir(child.mode) ? 4 : // directory
        FS.isLink(child.mode) ? 10 : // symbolic link.
        8;
      }
      (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), ((dirp + pos) >>> 3) >>> 0, ___asan_storeN)] = BigInt(id);
      (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), (((dirp + pos) + (8)) >>> 3) >>> 0, ___asan_storeN)] = BigInt((idx + 1) * struct_size);
      (growMemViews(), HEAP16)[_asan_js_check_index((growMemViews(), HEAP16), (((dirp + pos) + (16)) >>> 1) >>> 0, ___asan_storeN)] = 280;
      (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (dirp + pos) + (18) >>> 0, ___asan_storeN)] = type;
      stringToUTF8(name, dirp + pos + 19, 256);
      pos += struct_size;
    }
    FS.llseek(stream, idx * struct_size, 0);
    return pos;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_ioctl(fd, op, varargs) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(10, 0, 1, fd, op, varargs);
  varargs >>>= 0;
  SYSCALLS.varargs = varargs;
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    switch (op) {
     case 21509:
      {
        if (!stream.tty) return -59;
        return 0;
      }

     case 21505:
      {
        if (!stream.tty) return -59;
        if (stream.tty.ops.ioctl_tcgets) {
          var termios = stream.tty.ops.ioctl_tcgets(stream);
          var argp = syscallGetVarargP();
          (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((argp) >>> 2) >>> 0, ___asan_storeN)] = termios.c_iflag || 0;
          (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((argp) + (4)) >>> 2) >>> 0, ___asan_storeN)] = termios.c_oflag || 0;
          (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((argp) + (8)) >>> 2) >>> 0, ___asan_storeN)] = termios.c_cflag || 0;
          (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((argp) + (12)) >>> 2) >>> 0, ___asan_storeN)] = termios.c_lflag || 0;
          for (var i = 0; i < 32; i++) {
            (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (argp + i) + (17) >>> 0, ___asan_storeN)] = termios.c_cc[i] || 0;
          }
          return 0;
        }
        return 0;
      }

     case 21510:
     case 21511:
     case 21512:
      {
        if (!stream.tty) return -59;
        return 0;
      }

     case 21506:
     case 21507:
     case 21508:
      {
        if (!stream.tty) return -59;
        if (stream.tty.ops.ioctl_tcsets) {
          var argp = syscallGetVarargP();
          var c_iflag = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((argp) >>> 2) >>> 0, ___asan_loadN)];
          var c_oflag = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((argp) + (4)) >>> 2) >>> 0, ___asan_loadN)];
          var c_cflag = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((argp) + (8)) >>> 2) >>> 0, ___asan_loadN)];
          var c_lflag = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((argp) + (12)) >>> 2) >>> 0, ___asan_loadN)];
          var c_cc = [];
          for (var i = 0; i < 32; i++) {
            c_cc.push((growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (argp + i) + (17) >>> 0, ___asan_loadN)]);
          }
          return stream.tty.ops.ioctl_tcsets(stream.tty, op, {
            c_iflag,
            c_oflag,
            c_cflag,
            c_lflag,
            c_cc
          });
        }
        return 0;
      }

     case 21519:
      {
        if (!stream.tty) return -59;
        var argp = syscallGetVarargP();
        (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((argp) >>> 2) >>> 0, ___asan_storeN)] = 0;
        return 0;
      }

     case 21520:
      {
        if (!stream.tty) return -59;
        return -28;
      }

     case 21537:
     case 21531:
      {
        var argp = syscallGetVarargP();
        return FS.ioctl(stream, op, argp);
      }

     case 21523:
      {
        // TODO: in theory we should write to the winsize struct that gets
        // passed in, but for now musl doesn't read anything on it
        if (!stream.tty) return -59;
        if (stream.tty.ops.ioctl_tiocgwinsz) {
          var winsize = stream.tty.ops.ioctl_tiocgwinsz(stream.tty);
          var argp = syscallGetVarargP();
          (growMemViews(), HEAP16)[_asan_js_check_index((growMemViews(), HEAP16), ((argp) >>> 1) >>> 0, ___asan_storeN)] = winsize[0];
          (growMemViews(), HEAP16)[_asan_js_check_index((growMemViews(), HEAP16), (((argp) + (2)) >>> 1) >>> 0, ___asan_storeN)] = winsize[1];
        }
        return 0;
      }

     case 21524:
      {
        // TODO: technically, this ioctl call should change the window size.
        // but, since emscripten doesn't have any concept of a terminal window
        // yet, we'll just silently throw it away as we do TIOCGWINSZ
        if (!stream.tty) return -59;
        return 0;
      }

     case 21515:
      {
        if (!stream.tty) return -59;
        return 0;
      }

     default:
      return -28;
    }
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_lstat64(path, buf) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(11, 0, 1, path, buf);
  path >>>= 0;
  buf >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.writeStat(buf, FS.lstat(path));
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_mkdirat(dirfd, path, mode) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(12, 0, 1, dirfd, path, mode);
  path >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    mode &= ~SYSCALLS.currentUmask;
    FS.mkdir(path, mode);
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_newfstatat(dirfd, path, buf, flags) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(13, 0, 1, dirfd, path, buf, flags);
  path >>>= 0;
  buf >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    var nofollow = flags & 256;
    var allowEmpty = flags & 4096;
    flags = flags & (~6400);
    path = SYSCALLS.calculateAt(dirfd, path, allowEmpty);
    return SYSCALLS.writeStat(buf, nofollow ? FS.lstat(path) : FS.stat(path));
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_openat(dirfd, path, flags, varargs) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(14, 0, 1, dirfd, path, flags, varargs);
  path >>>= 0;
  varargs >>>= 0;
  SYSCALLS.varargs = varargs;
  try {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    var mode = varargs ? syscallGetVarargI() : 0;
    if (flags & 64) {
      mode &= ~SYSCALLS.currentUmask;
    }
    return FS.open(path, flags, mode).fd;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_renameat(olddirfd, oldpath, newdirfd, newpath) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(15, 0, 1, olddirfd, oldpath, newdirfd, newpath);
  oldpath >>>= 0;
  newpath >>>= 0;
  try {
    oldpath = SYSCALLS.getStr(oldpath);
    newpath = SYSCALLS.getStr(newpath);
    oldpath = SYSCALLS.calculateAt(olddirfd, oldpath);
    newpath = SYSCALLS.calculateAt(newdirfd, newpath);
    FS.rename(oldpath, newpath);
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_rmdir(path) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(16, 0, 1, path);
  path >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    FS.rmdir(path);
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_stat64(path, buf) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(17, 0, 1, path, buf);
  path >>>= 0;
  buf >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.writeStat(buf, FS.stat(path));
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function ___syscall_unlinkat(dirfd, path, flags) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(18, 0, 1, dirfd, path, flags);
  path >>>= 0;
  try {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    if (!flags) {
      FS.unlink(path);
    } else if (flags === 512) {
      FS.rmdir(path);
    } else {
      return -28;
    }
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

var __abort_js = () => abort("");

var getExecutableName = () => thisProgram;

function __emscripten_get_progname(str, len) {
  str >>>= 0;
  return stringToUTF8(getExecutableName(), str, len);
}

function __emscripten_init_main_thread_js(tb) {
  tb >>>= 0;
  var can_block = !ENVIRONMENT_IS_WEB;
  // Feature detect whether the main thread can block.
  try {
    Atomics.wait((growMemViews(), HEAP32), 0, 0, 0);
    can_block = true;
  } catch (e) {}
  // Pass the thread address to the native code where they are stored in wasm
  // globals which act as a form of TLS. Global constructors trying
  // to access this value will read the wrong value, but that is UB anyway.
  __emscripten_thread_init(tb, /*is_main=*/ !ENVIRONMENT_IS_WORKER, /*is_runtime=*/ 1, can_block, /*default_stacksize=*/ 4194304, /*start_profiling=*/ false);
  PThread.threadInitTLS();
}

var jsStackTrace = () => (new Error).stack.toString();

/** @param {number=} flags */ var getCallstack = flags => {
  var callstack = jsStackTrace();
  // Process all lines:
  var lines = callstack.split("\n");
  callstack = "";
  // Extract components of form:
  // '       Object._main@http://server.com:4324:12'
  var firefoxRe = new RegExp("\\s*(.*?)@(.*?):([0-9]+):([0-9]+)");
  // Extract components of form:
  // '    at Object._main (http://server.com/file.html:4324:12)'
  var chromeRe = new RegExp("\\s*at (.*?) \\((.*):(.*):(.*)\\)");
  for (var line of lines) {
    var symbolName = "";
    var file = "";
    var lineno = 0;
    var column = 0;
    var parts = chromeRe.exec(line);
    if (parts?.length == 5) {
      symbolName = parts[1];
      file = parts[2];
      lineno = parts[3];
      column = parts[4];
    } else {
      parts = firefoxRe.exec(line);
      if (parts?.length >= 4) {
        symbolName = parts[1];
        file = parts[2];
        lineno = parts[3];
        // Old Firefox doesn't carry column information, but in new FF30, it
        // is present. See https://bugzil.la/762556
        column = parts[4] | 0;
      } else {
        // Was not able to extract this line for demangling/sourcemapping
        // purposes. Output it as-is.
        callstack += line + "\n";
        continue;
      }
    }
    // Find the symbols in the callstack that corresponds to the functions that
    // report callstack information, and remove everything up to these from the
    // output.
    if (symbolName == "_emscripten_log" || symbolName == "_emscripten_get_callstack") {
      callstack = "";
      continue;
    }
    if ((flags & 24)) {
      if (flags & 64) {
        file = file.substring(file.replace(/\\/g, "/").lastIndexOf("/") + 1);
      }
      callstack += `    at ${symbolName} (${file}:${lineno}:${column})\n`;
    }
  }
  // Trim extra whitespace at the end of the output.
  callstack = callstack.replace(/\s+$/, "");
  return callstack;
};

function __emscripten_log_formatted(flags, str) {
  str >>>= 0;
  str = UTF8ToString(str);
  if (flags & 24) {
    str = str.replace(/\s+$/, "");
    // Ensure the message and the callstack are joined cleanly with exactly one newline.
    str += (str.length > 0 ? "\n" : "") + getCallstack(flags);
  }
  if (flags & 1) {
    if (flags & 4) {
      console.error(str);
    } else if (flags & 2) {
      console.warn(str);
    } else if (flags & 512) {
      console.info(str);
    } else if (flags & 256) {
      console.debug(str);
    } else {
      console.log(str);
    }
  } else if (flags & 6) {
    err(str);
  } else {
    out(str);
  }
}

var handleException = e => {
  // Certain exception types we do not treat as errors since they are used for
  // internal control flow.
  // 1. ExitStatus, which is thrown by exit()
  // 2. "unwind", which is thrown by emscripten_unwind_to_js_event_loop() and others
  //    that wish to return to JS event loop.
  if (e instanceof ExitStatus || e == "unwind") {
    return EXITSTATUS;
  }
  checkStackCookie();
  if (e instanceof WebAssembly.RuntimeError) {
    if (_emscripten_stack_get_current() <= 0) {
      err("Stack overflow detected.  You can try increasing -sSTACK_SIZE (currently set to 4194304)");
    }
  }
  quit_(1, e);
};

var maybeExit = () => {
  if (runtimeExited) {
    return;
  }
  if (!keepRuntimeAlive()) {
    try {
      if (ENVIRONMENT_IS_PTHREAD) {
        // exit the current thread, but only if there is one active.
        // TODO(https://github.com/emscripten-core/emscripten/issues/25076):
        // Unify this check with the runtimeExited check above
        if (_pthread_self()) __emscripten_thread_exit(EXITSTATUS);
        return;
      }
      _exit(EXITSTATUS);
    } catch (e) {
      handleException(e);
    }
  }
};

var callUserCallback = (func, ...args) => {
  if (runtimeExited || ABORT) {
    return;
  }
  try {
    return func(...args);
  } catch (e) {
    handleException(e);
  } finally {
    maybeExit();
  }
};

function __emscripten_thread_mailbox_await(pthread_ptr) {
  pthread_ptr >>>= 0;
  if (!waitAsyncPolyfilled) {
    // Wait on the pthread's initial self-pointer field because it is easy and
    // safe to access from sending threads that need to notify the waiting
    // thread.
    // Note: Under wasm64 only the low 32-bit of the pthread_ptr are
    // read/compared here, but we don't actually care about the exact values
    // here as long as they match.
    var wait = Atomics.waitAsync((growMemViews(), HEAP32), ((pthread_ptr) >>> 2), pthread_ptr);
    wait.value.then(checkMailbox);
    var waitingAsync = pthread_ptr + 112;
    Atomics.store((growMemViews(), HEAP32), ((waitingAsync) >>> 2), 1);
  }
}

var checkMailbox = () => {
  // checkMailbox can be called after the pthread has shut down. See
  // Pthread.terminateRuntime().
  // In this case we return silently without re-registering using waitAsync.
  // Perhaps there is a more universal way we can detect runtime has exited.
  // TODO(https://github.com/emscripten-core/emscripten/issues/25076)
  var pthread_ptr = _pthread_self();
  if (!pthread_ptr) return;
  callUserCallback(() => {
    // If we are using Atomics.waitAsync as our notification mechanism, wait
    // for a notification before processing the mailbox to avoid missing any
    // work that could otherwise arrive after we've finished processing the
    // mailbox and before we're ready for the next notification.
    __emscripten_thread_mailbox_await(pthread_ptr);
    __emscripten_check_mailbox();
  });
};

function __emscripten_notify_mailbox_postmessage(targetThread, currThreadId) {
  targetThread >>>= 0;
  currThreadId >>>= 0;
  if (targetThread == currThreadId) {
    setTimeout(checkMailbox);
  } else if (ENVIRONMENT_IS_PTHREAD) {
    postMessage({
      targetThread,
      cmd: 4
    });
  } else {
    var worker = PThread.pthreads[targetThread];
    if (!worker) {
      return;
    }
    worker.postMessage({
      cmd: 4
    });
  }
}

var proxiedJSCallArgs = [];

function __emscripten_receive_on_main_thread_js(funcIndex, emAsmAddr, callingThread, bufSize, args, ctx, ctxArgs) {
  emAsmAddr >>>= 0;
  callingThread >>>= 0;
  args >>>= 0;
  ctx >>>= 0;
  ctxArgs >>>= 0;
  // Sometimes we need to backproxy events to the calling thread (e.g.
  // HTML5 DOM events handlers such as
  // emscripten_set_mousemove_callback()), so keep track in a globally
  // accessible variable about the thread that initiated the proxying.
  proxiedJSCallArgs.length = 0;
  var b = ((args) >>> 3);
  var end = ((args + bufSize) >>> 3);
  while (b < end) {
    var arg;
    if ((growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), b++ >>> 0, ___asan_loadN)]) {
      // It's a BigInt.
      arg = (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), b++ >>> 0, ___asan_loadN)];
    } else {
      // It's a Number.
      arg = (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), b++ >>> 0, ___asan_loadN)];
    }
    proxiedJSCallArgs.push(arg);
  }
  // Proxied JS library funcs use funcIndex and EM_ASM functions use emAsmAddr
  var func = emAsmAddr ? ASM_CONSTS[emAsmAddr] : proxiedFunctionTable[funcIndex];
  PThread.currentProxiedOperationCallerThread = callingThread;
  var rtn = func(...proxiedJSCallArgs);
  PThread.currentProxiedOperationCallerThread = 0;
  if (ctx) {
    // A PROXY_SYNC_ASYNC function may complete synchronously with a plain value.
    Promise.resolve(rtn).then(rtn => __emscripten_run_js_on_main_thread_done(ctx, ctxArgs, rtn));
    return;
  }
  return rtn;
}

var __emscripten_runtime_keepalive_clear = () => {
  noExitRuntime = false;
  runtimeKeepaliveCounter = 0;
};

var stringToNewUTF8 = str => {
  var size = lengthBytesUTF8(str) + 1;
  var ret = _malloc(size);
  if (ret) stringToUTF8(str, ret, size);
  return ret;
};

function __emscripten_sanitizer_get_option(name) {
  name >>>= 0;
  return stringToNewUTF8(Module[UTF8ToString(name)] ?? "");
}

var __emscripten_sanitizer_use_colors = () => {
  var setting = Module["printWithColors"];
  if (setting !== undefined) {
    return setting;
  }
  return ENVIRONMENT_IS_NODE && process.stderr.isTTY;
};

function __emscripten_system(command) {
  command >>>= 0;
  if (ENVIRONMENT_IS_NODE) {
    if (!command) return 1;
    // shell is available
    var cmdstr = UTF8ToString(command);
    if (!cmdstr.length) return 0;
    // this is what glibc seems to do (shell works test?)
    var cp = require("node:child_process");
    var ret = cp.spawnSync(cmdstr, [], {
      shell: true,
      stdio: "inherit"
    });
    var _W_EXITCODE = (ret, sig) => ((ret) << 8 | (sig));
    // this really only can happen if process is killed by signal
    if (ret.status === null) {
      // sadly node doesn't expose such function
      var signalToNumber = sig => {
        // implement only the most common ones, and fallback to SIGINT
        switch (sig) {
         case "SIGHUP":
          return 1;

         case "SIGQUIT":
          return 3;

         case "SIGFPE":
          return 8;

         case "SIGKILL":
          return 9;

         case "SIGALRM":
          return 14;

         case "SIGTERM":
          return 15;

         default:
          return 2;
        }
      };
      return _W_EXITCODE(0, signalToNumber(ret.signal));
    }
    return _W_EXITCODE(ret.status, 0);
  }
  // int system(const char *command);
  // http://pubs.opengroup.org/onlinepubs/000095399/functions/system.html
  // Can't call external programs.
  if (!command) return 0;
  // no shell available
  return -52;
}

function __emscripten_thread_cleanup(thread) {
  thread >>>= 0;
  // Called when a thread needs to be cleaned up so it can be reused.
  // A thread is considered reusable when it either returns from its
  // entry point, calls pthread_exit, or acts upon a cancellation.
  // Detached threads are responsible for calling this themselves,
  // otherwise pthread_join is responsible for calling this.
  if (!ENVIRONMENT_IS_PTHREAD) cleanupThread(thread); else postMessage({
    cmd: 6,
    thread
  });
}

function __emscripten_thread_set_strongref(thread) {
  thread >>>= 0;
  // Called when a thread needs to be strongly referenced.
  // Currently only used for:
  // - keeping the "main" thread alive in PROXY_TO_PTHREAD mode;
  // - crashed threads that need to propagate the uncaught exception
  //   back to the main thread.
  if (ENVIRONMENT_IS_NODE) {
    var worker = PThread.pthreads[thread];
    worker.ref();
    // Also, record that we called strongref, in case this function is called
    // bafore the 'loaded' callback from the thread (where we would normally
    // `unref` it.
    worker.strongref = 1;
  }
}

var __emscripten_throw_longjmp = () => {
  throw new EmscriptenSjLj;
};

function __gmtime_js(time, tmPtr) {
  time = bigintToI53Checked(time);
  tmPtr >>>= 0;
  var date = new Date(time * 1e3);
  if (isNaN(date.getTime())) {
    return 1;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((tmPtr) >>> 2) >>> 0, ___asan_storeN)] = date.getUTCSeconds();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (4)) >>> 2) >>> 0, ___asan_storeN)] = date.getUTCMinutes();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (8)) >>> 2) >>> 0, ___asan_storeN)] = date.getUTCHours();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (12)) >>> 2) >>> 0, ___asan_storeN)] = date.getUTCDate();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (16)) >>> 2) >>> 0, ___asan_storeN)] = date.getUTCMonth();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (20)) >>> 2) >>> 0, ___asan_storeN)] = date.getUTCFullYear() - 1900;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (24)) >>> 2) >>> 0, ___asan_storeN)] = date.getUTCDay();
  var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
  var yday = ((date.getTime() - start) / (1e3 * 60 * 60 * 24)) | 0;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (28)) >>> 2) >>> 0, ___asan_storeN)] = yday;
  return 0;
}

var isLeapYear = year => year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0);

var MONTH_DAYS_LEAP_CUMULATIVE = [ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 ];

var MONTH_DAYS_REGULAR_CUMULATIVE = [ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 ];

var ydayFromDate = date => {
  var leap = isLeapYear(date.getFullYear());
  var monthDaysCumulative = (leap ? MONTH_DAYS_LEAP_CUMULATIVE : MONTH_DAYS_REGULAR_CUMULATIVE);
  var yday = monthDaysCumulative[date.getMonth()] + date.getDate() - 1;
  // -1 since it's days since Jan 1
  return yday;
};

function __localtime_js(time, tmPtr) {
  time = bigintToI53Checked(time);
  tmPtr >>>= 0;
  var date = new Date(time * 1e3);
  if (isNaN(date.getTime())) {
    return 1;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((tmPtr) >>> 2) >>> 0, ___asan_storeN)] = date.getSeconds();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (4)) >>> 2) >>> 0, ___asan_storeN)] = date.getMinutes();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (8)) >>> 2) >>> 0, ___asan_storeN)] = date.getHours();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (12)) >>> 2) >>> 0, ___asan_storeN)] = date.getDate();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (16)) >>> 2) >>> 0, ___asan_storeN)] = date.getMonth();
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (20)) >>> 2) >>> 0, ___asan_storeN)] = date.getFullYear() - 1900;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (24)) >>> 2) >>> 0, ___asan_storeN)] = date.getDay();
  var yday = ydayFromDate(date) | 0;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (28)) >>> 2) >>> 0, ___asan_storeN)] = yday;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (36)) >>> 2) >>> 0, ___asan_storeN)] = -(date.getTimezoneOffset() * 60);
  // Attention: DST is in December in South, and some regions don't have DST at all.
  var start = new Date(date.getFullYear(), 0, 1);
  var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
  var winterOffset = start.getTimezoneOffset();
  var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset)) | 0;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (32)) >>> 2) >>> 0, ___asan_storeN)] = dst;
  return 0;
}

var __mktime_js = function(tmPtr) {
  tmPtr >>>= 0;
  var ret = ((() => {
    var date = new Date((growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
    HEAP32), (((tmPtr) + (20)) >>> 2) >>> 0, ___asan_loadN)] + 1900, (growMemViews(), 
    HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (16)) >>> 2) >>> 0, ___asan_loadN)], (growMemViews(), 
    HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (12)) >>> 2) >>> 0, ___asan_loadN)], (growMemViews(), 
    HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (8)) >>> 2) >>> 0, ___asan_loadN)], (growMemViews(), 
    HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (4)) >>> 2) >>> 0, ___asan_loadN)], (growMemViews(), 
    HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((tmPtr) >>> 2) >>> 0, ___asan_loadN)], 0);
    if (isNaN(date.getTime())) {
      return -1;
    }
    // There's an ambiguous hour when the time goes back; the tm_isdst field is
    // used to disambiguate it.  Date() basically guesses, so we fix it up if it
    // guessed wrong, or fill in tm_isdst with the guess if it's -1.
    var dst = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (32)) >>> 2) >>> 0, ___asan_loadN)];
    var guessedOffset = date.getTimezoneOffset();
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dstOffset = Math.min(winterOffset, summerOffset);
    // DST is in December in South
    if (dst < 0) {
      // Attention: some regions don't have DST at all.
      dst = Number(summerOffset != winterOffset && dstOffset == guessedOffset);
    } else if ((dst > 0) != (dstOffset == guessedOffset)) {
      var nonDstOffset = Math.max(winterOffset, summerOffset);
      var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
      // Don't try setMinutes(date.getMinutes() + ...) -- it's messed up.
      date.setTime(date.getTime() + (trueOffset - guessedOffset) * 6e4);
      if (isNaN(date.getTime())) {
        return -1;
      }
    }
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (32)) >>> 2) >>> 0, ___asan_storeN)] = dst;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (24)) >>> 2) >>> 0, ___asan_storeN)] = date.getDay();
    var yday = ydayFromDate(date) | 0;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (28)) >>> 2) >>> 0, ___asan_storeN)] = yday;
    // To match expected behavior, update fields from date
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((tmPtr) >>> 2) >>> 0, ___asan_storeN)] = date.getSeconds();
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (4)) >>> 2) >>> 0, ___asan_storeN)] = date.getMinutes();
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (8)) >>> 2) >>> 0, ___asan_storeN)] = date.getHours();
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (12)) >>> 2) >>> 0, ___asan_storeN)] = date.getDate();
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (16)) >>> 2) >>> 0, ___asan_storeN)] = date.getMonth();
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((tmPtr) + (20)) >>> 2) >>> 0, ___asan_storeN)] = date.getYear();
    // Return time in seconds
    return date.getTime() / 1e3;
  }))();
  return BigInt(ret);
};

function __mmap_js(len, prot, flags, fd, offset, allocated, addr) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(19, 0, 1, len, prot, flags, fd, offset, allocated, addr);
  len >>>= 0;
  offset = bigintToI53Checked(offset);
  allocated >>>= 0;
  addr >>>= 0;
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    var res = FS.mmap(stream, len, offset, prot, flags);
    var ptr = res.ptr;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((allocated) >>> 2) >>> 0, ___asan_storeN)] = res.allocated;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((addr) >>> 2) >>> 0, ___asan_storeN)] = ptr;
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

function __munmap_js(addr, len, prot, flags, fd, offset) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(20, 0, 1, addr, len, prot, flags, fd, offset);
  addr >>>= 0;
  len >>>= 0;
  offset = bigintToI53Checked(offset);
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    if (prot & 2) {
      SYSCALLS.doMsync(addr, stream, len, flags, offset);
    }
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return -e.errno;
  }
}

var __tzset_js = function(timezone, daylight, std_name, dst_name) {
  timezone >>>= 0;
  daylight >>>= 0;
  std_name >>>= 0;
  dst_name >>>= 0;
  // TODO: Use (malleable) environment variables instead of system settings.
  var currentYear = (new Date).getFullYear();
  var winter = new Date(currentYear, 0, 1);
  var summer = new Date(currentYear, 6, 1);
  var winterOffset = winter.getTimezoneOffset();
  var summerOffset = summer.getTimezoneOffset();
  // Local standard timezone offset. Local standard time is not adjusted for
  // daylight savings.  This code uses the fact that getTimezoneOffset returns
  // a greater value during Standard Time versus Daylight Saving Time (DST).
  // Thus it determines the expected output during Standard Time, and it
  // compares whether the output of the given date the same (Standard) or less
  // (DST).
  var stdTimezoneOffset = Math.max(winterOffset, summerOffset);
  // timezone is specified as seconds west of UTC ("The external variable
  // `timezone` shall be set to the difference, in seconds, between
  // Coordinated Universal Time (UTC) and local standard time."), the same
  // as returned by stdTimezoneOffset.
  // See http://pubs.opengroup.org/onlinepubs/009695399/functions/tzset.html
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((timezone) >>> 2) >>> 0, ___asan_storeN)] = stdTimezoneOffset * 60;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((daylight) >>> 2) >>> 0, ___asan_storeN)] = Number(winterOffset != summerOffset);
  var extractZone = timezoneOffset => {
    // Why inverse sign?
    // Read here https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/getTimezoneOffset
    var sign = timezoneOffset >= 0 ? "-" : "+";
    var absOffset = Math.abs(timezoneOffset);
    var hours = String(Math.floor(absOffset / 60)).padStart(2, "0");
    var minutes = String(absOffset % 60).padStart(2, "0");
    return `UTC${sign}${hours}${minutes}`;
  };
  var winterName = extractZone(winterOffset);
  var summerName = extractZone(summerOffset);
  if (summerOffset < winterOffset) {
    // Northern hemisphere
    stringToUTF8(winterName, std_name, 17);
    stringToUTF8(summerName, dst_name, 17);
  } else {
    stringToUTF8(winterName, dst_name, 17);
    stringToUTF8(summerName, std_name, 17);
  }
};

var _emscripten_get_now = () => performance.timeOrigin + performance.now();

var _emscripten_date_now = () => Date.now();

var nowIsMonotonic = 1;

var checkWasiClock = clock_id => clock_id >= 0 && clock_id <= 3;

function _clock_time_get(clk_id, ignored_precision, ptime) {
  ignored_precision = bigintToI53Checked(ignored_precision);
  ptime >>>= 0;
  if (!checkWasiClock(clk_id)) {
    return 28;
  }
  var now;
  // all wasi clocks but realtime are monotonic
  if (clk_id === 0) {
    now = _emscripten_date_now();
  } else if (nowIsMonotonic) {
    now = _emscripten_get_now();
  } else {
    return 52;
  }
  // "now" is in ms, and wasi times are in ns.
  var nsec = Math.round(now * 1e6);
  (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), ((ptime) >>> 3) >>> 0, ___asan_storeN)] = BigInt(nsec);
  return 0;
}

function getFullscreenElement() {
  return document.fullscreenElement ?? document.webkitFullscreenElement;
}

var runtimeKeepalivePush = () => {
  runtimeKeepaliveCounter += 1;
};

var runtimeKeepalivePop = () => {
  runtimeKeepaliveCounter -= 1;
};

/** @param {number=} timeout */ var safeSetTimeout = (func, timeout) => {
  runtimeKeepalivePush();
  var id = safeSetTimeout.nextId++;
  safeSetTimeout.pending.set(id, setTimeout(() => {
    safeSetTimeout.pending.delete(id);
    runtimeKeepalivePop();
    callUserCallback(func);
  }, timeout));
  return id;
};

var Browser = {
  useWebGL: false,
  isFullscreen: false,
  pointerLock: false,
  moduleContextCreatedCallbacks: [],
  preloadedImages: {},
  preloadedAudios: {},
  getCanvas: () => Module["canvas"],
  init() {
    if (Browser.initted) return;
    Browser.initted = true;
    // Support for plugins that can process preloaded files. You can add more of these to
    // your app by creating and appending to preloadPlugins.
    // Each plugin is asked if it can handle a file based on the file's name. If it can,
    // it is given the file's raw data. When it is done, it calls a callback with the file's
    // (possibly modified) data. For example, a plugin might decompress a file, or it
    // might create some side data structure for use later (like an Image element, etc.).
    var imagePlugin = {};
    imagePlugin["canHandle"] = name => !Module["noImageDecoding"] && /\.(jpg|jpeg|png|bmp|webp)$/i.test(name);
    imagePlugin["handle"] = async (byteArray, name) => {
      var b = new Blob([ byteArray ], {
        type: Browser.getMimetype(name)
      });
      if (b.size !== byteArray.length) {
        // Safari bug #118630
        // Safari's Blob can only take an ArrayBuffer
        b = new Blob([ (new Uint8Array(byteArray)).buffer ], {
          type: Browser.getMimetype(name)
        });
      }
      var url = URL.createObjectURL(b);
      return new Promise((resolve, reject) => {
        var img = new Image;
        img.onload = () => {
          var canvas = /** @type {!HTMLCanvasElement} */ (document.createElement("canvas"));
          canvas.width = img.width;
          canvas.height = img.height;
          var ctx = canvas.getContext("2d");
          ctx.drawImage(img, 0, 0);
          Browser.preloadedImages[name] = canvas;
          URL.revokeObjectURL(url);
          resolve(byteArray);
        };
        img.onerror = event => {
          err(`Image ${url} could not be decoded`);
          reject();
        };
        img.src = url;
      });
    };
    preloadPlugins.push(imagePlugin);
    var audioPlugin = {};
    audioPlugin["canHandle"] = name => !Module["noAudioDecoding"] && name.slice(-4) in {
      ".ogg": 1,
      ".wav": 1,
      ".mp3": 1
    };
    audioPlugin["handle"] = async (byteArray, name) => new Promise((resolve, reject) => {
      var done = false;
      function finish(audio) {
        if (done) return;
        done = true;
        Browser.preloadedAudios[name] = audio;
        resolve(byteArray);
      }
      var b = new Blob([ byteArray ], {
        type: Browser.getMimetype(name)
      });
      var url = URL.createObjectURL(b);
      // XXX we never revoke this!
      var audio = new Audio;
      audio.addEventListener("canplaythrough", () => finish(audio));
      // use addEventListener due to chromium bug 124926
      audio.onerror = event => {
        if (done) return;
        err(`warning: browser could not fully decode audio ${name}, trying slower base64 approach`);
        function encode64(data) {
          var BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
          var PAD = "=";
          var ret = "";
          var leftchar = 0;
          var leftbits = 0;
          for (var byte of data) {
            leftchar = (leftchar << 8) | byte;
            leftbits += 8;
            while (leftbits >= 6) {
              var curr = (leftchar >> (leftbits - 6)) & 63;
              leftbits -= 6;
              ret += BASE[curr];
            }
          }
          if (leftbits == 2) {
            ret += BASE[(leftchar & 3) << 4];
            ret += PAD + PAD;
          } else if (leftbits == 4) {
            ret += BASE[(leftchar & 15) << 2];
            ret += PAD;
          }
          return ret;
        }
        audio.src = "data:audio/x-" + name.slice(-3) + ";base64," + encode64(byteArray);
        finish(audio);
      };
      audio.src = url;
      // workaround for chrome bug 124926 - we do not always get oncanplaythrough or onerror
      safeSetTimeout(() => {
        finish(audio);
      }, 1e4);
    });
    preloadPlugins.push(audioPlugin);
    // Canvas event setup
    function pointerLockChange() {
      var canvas = Browser.getCanvas();
      Browser.pointerLock = document.pointerLockElement === canvas;
    }
    var canvas = Browser.getCanvas();
    if (canvas) {
      // forced aspect ratio can be enabled by defining 'forcedAspectRatio' on Module
      // Module['forcedAspectRatio'] = 4 / 3;
      document.addEventListener("pointerlockchange", pointerLockChange);
      if (Module["elementPointerLock"]) {
        canvas.addEventListener("click", ev => {
          if (!Browser.pointerLock && Browser.getCanvas().requestPointerLock) {
            Browser.getCanvas().requestPointerLock();
            ev.preventDefault();
          }
        });
      }
    }
  },
  createContext(/** @type {HTMLCanvasElement} */ canvas, useWebGL, setInModule, webGLContextAttributes) {
    if (useWebGL && Module["ctx"] && canvas == Browser.getCanvas()) return Module["ctx"];
    // no need to recreate GL context if it's already been created for this canvas.
    var ctx;
    var contextHandle;
    if (useWebGL) {
      // For GLES2/desktop GL compatibility, adjust a few defaults to be different to WebGL defaults, so that they align better with the desktop defaults.
      var contextAttributes = {
        antialias: false,
        alpha: false,
        majorVersion: 2
      };
      if (webGLContextAttributes) {
        for (var attribute in webGLContextAttributes) {
          contextAttributes[attribute] = webGLContextAttributes[attribute];
        }
      }
      // This check of existence of GL is here to satisfy Closure compiler, which yells if variable GL is referenced below but GL object is not
      // actually compiled in because application is not doing any GL operations. TODO: Ideally if GL is not being used, this function
      // Browser.createContext() should not even be emitted.
      if (typeof GL != "undefined") {
        contextHandle = GL.createContext(canvas, contextAttributes);
        if (contextHandle) {
          ctx = GL.getContext(contextHandle).GLctx;
        }
      }
    } else {
      ctx = canvas.getContext("2d");
    }
    if (!ctx) return null;
    if (setInModule) {
      Module["ctx"] = ctx;
      if (useWebGL) GL.makeContextCurrent(contextHandle);
      Browser.useWebGL = useWebGL;
      Browser.moduleContextCreatedCallbacks.forEach(callback => callback());
      Browser.init();
    }
    return ctx;
  },
  fullscreenHandlersInstalled: false,
  lockPointer: undefined,
  resizeCanvas: undefined,
  requestFullscreen(lockPointer, resizeCanvas) {
    Browser.lockPointer = lockPointer;
    Browser.resizeCanvas = resizeCanvas;
    if (typeof Browser.lockPointer == "undefined") Browser.lockPointer = true;
    if (typeof Browser.resizeCanvas == "undefined") Browser.resizeCanvas = false;
    var canvas = Browser.getCanvas();
    function fullscreenChange() {
      Browser.isFullscreen = false;
      var canvasContainer = canvas.parentNode;
      if (getFullscreenElement() === canvasContainer) {
        canvas.exitFullscreen = Browser.exitFullscreen;
        if (Browser.lockPointer) canvas.requestPointerLock();
        Browser.isFullscreen = true;
        if (Browser.resizeCanvas) {
          Browser.setFullscreenCanvasSize();
        } else {
          Browser.updateCanvasDimensions(canvas);
        }
      } else {
        // remove the full screen specific parent of the canvas again to restore the HTML structure from before going full screen
        canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
        canvasContainer.parentNode.removeChild(canvasContainer);
        if (Browser.resizeCanvas) {
          Browser.setWindowedCanvasSize();
        } else {
          Browser.updateCanvasDimensions(canvas);
        }
      }
    }
    if (!Browser.fullscreenHandlersInstalled) {
      Browser.fullscreenHandlersInstalled = true;
      document.addEventListener("fullscreenchange", fullscreenChange);
      document.addEventListener("webkitfullscreenchange", fullscreenChange);
    }
    // create a new parent to ensure the canvas has no siblings. this allows browsers to optimize full screen performance when its parent is the full screen root
    var canvasContainer = document.createElement("div");
    canvas.parentNode.insertBefore(canvasContainer, canvas);
    canvasContainer.appendChild(canvas);
    // use parent of canvas as full screen root to allow aspect ratio correction (Firefox stretches the root to screen size)
    // Safari didn't support Element.requestFullscreen until 16.4
    // See: https://developer.mozilla.org/en-US/docs/Web/API/Element/requestFullscreen
    /** @suppress {checkTypes} */ canvasContainer.requestFullscreen ??= (canvasContainer["webkitRequestFullscreen"] ? () => canvasContainer["webkitRequestFullscreen"](Element.ALLOW_KEYBOARD_INPUT) : null) ?? (canvasContainer["webkitRequestFullScreen"] ? () => canvasContainer["webkitRequestFullScreen"](Element.ALLOW_KEYBOARD_INPUT) : null);
    canvasContainer.requestFullscreen();
  },
  exitFullscreen() {
    // This is workaround for chrome. Trying to exit from fullscreen
    // not in fullscreen state will cause 'TypeError: Document not active'
    // in chrome. See https://github.com/emscripten-core/emscripten/pull/8236
    if (!Browser.isFullscreen) {
      return false;
    }
    var CFS = document.exitFullscreen ?? document["webkitCancelFullScreen"];
    CFS.apply(document, []);
    return true;
  },
  getMimetype(name) {
    return {
      "jpg": "image/jpeg",
      "jpeg": "image/jpeg",
      "png": "image/png",
      "bmp": "image/bmp",
      "ogg": "audio/ogg",
      "wav": "audio/wav",
      "mp3": "audio/mpeg"
    }[name.slice(name.lastIndexOf(".") + 1)];
  },
  getMouseWheelDelta(event) {
    var delta = 0;
    switch (event.type) {
     case "DOMMouseScroll":
      // 3 lines make up a step
      delta = event.detail / 3;
      break;

     case "mousewheel":
      // 120 units make up a step
      delta = event.wheelDelta / 120;
      break;

     case "wheel":
      delta = event.deltaY;
      switch (event.deltaMode) {
       case 0:
        // DOM_DELTA_PIXEL: 100 pixels make up a step
        delta /= 100;
        break;

       case 1:
        // DOM_DELTA_LINE: 3 lines make up a step
        delta /= 3;
        break;

       case 2:
        // DOM_DELTA_PAGE: A page makes up 80 steps
        delta *= 80;
        break;

       default:
        abort("unrecognized mouse wheel delta mode: " + event.deltaMode);
      }
      break;

     default:
      abort("unrecognized mouse wheel event: " + event.type);
    }
    return delta;
  },
  mouseX: 0,
  mouseY: 0,
  mouseMovementX: 0,
  mouseMovementY: 0,
  touches: {},
  lastTouches: {},
  calculateMouseCoords(pageX, pageY) {
    // Calculate the movement based on the changes
    // in the coordinates.
    var canvas = Browser.getCanvas();
    var rect = canvas.getBoundingClientRect();
    var adjustedX = pageX - (window.scrollX + rect.left);
    var adjustedY = pageY - (window.scrollY + rect.top);
    // the canvas might be CSS-scaled compared to its backbuffer;
    // SDL-using content will want mouse coordinates in terms
    // of backbuffer units.
    adjustedX = adjustedX * (canvas.width / rect.width);
    adjustedY = adjustedY * (canvas.height / rect.height);
    return {
      x: adjustedX,
      y: adjustedY
    };
  },
  setMouseCoords(pageX, pageY) {
    const {x, y} = Browser.calculateMouseCoords(pageX, pageY);
    Browser.mouseMovementX = x - Browser.mouseX;
    Browser.mouseMovementY = y - Browser.mouseY;
    Browser.mouseX = x;
    Browser.mouseY = y;
  },
  calculateMouseEvent(event) {
    // event should be mousemove, mousedown or mouseup
    if (Browser.pointerLock) {
      // When the pointer is locked, calculate the coordinates
      // based on the movement of the mouse.
      Browser.mouseMovementX = event.movementX;
      Browser.mouseMovementY = event.movementY;
      // add the mouse delta to the current absolute mouse position
      Browser.mouseX += Browser.mouseMovementX;
      Browser.mouseY += Browser.mouseMovementY;
    } else {
      if (event.type === "touchstart" || event.type === "touchend" || event.type === "touchmove") {
        var touch = event.touch;
        if (touch === undefined) {
          return;
        }
        var coords = Browser.calculateMouseCoords(touch.pageX, touch.pageY);
        if (event.type === "touchstart") {
          Browser.lastTouches[touch.identifier] = coords;
          Browser.touches[touch.identifier] = coords;
        } else if (event.type === "touchend" || event.type === "touchmove") {
          var last = Browser.touches[touch.identifier];
          last ||= coords;
          Browser.lastTouches[touch.identifier] = last;
          Browser.touches[touch.identifier] = coords;
        }
        return;
      }
      Browser.setMouseCoords(event.pageX, event.pageY);
    }
  },
  resizeListeners: [],
  updateResizeListeners() {
    var canvas = Browser.getCanvas();
    Browser.resizeListeners.forEach(listener => listener(canvas.width, canvas.height));
  },
  setCanvasSize(width, height, noUpdates) {
    var canvas = Browser.getCanvas();
    Browser.updateCanvasDimensions(canvas, width, height);
    if (!noUpdates) Browser.updateResizeListeners();
  },
  setFullscreenCanvasSize() {
    // check if SDL is available
    if (typeof SDL != "undefined") {
      var flags = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((SDL.screen) >>> 2) >>> 0, ___asan_loadN)];
      flags = flags | 8388608;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((SDL.screen) >>> 2) >>> 0, ___asan_storeN)] = flags;
    }
    Browser.updateCanvasDimensions(Browser.getCanvas());
    Browser.updateResizeListeners();
  },
  setWindowedCanvasSize() {
    // check if SDL is available
    if (typeof SDL != "undefined") {
      var flags = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((SDL.screen) >>> 2) >>> 0, ___asan_loadN)];
      flags = flags & ~8388608;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((SDL.screen) >>> 2) >>> 0, ___asan_storeN)] = flags;
    }
    Browser.updateCanvasDimensions(Browser.getCanvas());
    Browser.updateResizeListeners();
  },
  updateCanvasDimensions(canvas, wNative, hNative) {
    if (wNative && hNative) {
      canvas.widthNative = wNative;
      canvas.heightNative = hNative;
    } else {
      wNative = canvas.widthNative;
      hNative = canvas.heightNative;
    }
    var w = wNative;
    var h = hNative;
    if ((getFullscreenElement() === canvas.parentNode) && (typeof screen != "undefined")) {
      var factor = Math.min(screen.width / w, screen.height / h);
      w = Math.round(w * factor);
      h = Math.round(h * factor);
    }
    if (Browser.resizeCanvas) {
      if (canvas.width != w) canvas.width = w;
      if (canvas.height != h) canvas.height = h;
      if (typeof canvas.style != "undefined") {
        canvas.style.removeProperty("width");
        canvas.style.removeProperty("height");
      }
    } else {
      if (canvas.width != wNative) canvas.width = wNative;
      if (canvas.height != hNative) canvas.height = hNative;
      if (typeof canvas.style != "undefined") {
        if (w != wNative || h != hNative) {
          canvas.style.setProperty("width", w + "px", "important");
          canvas.style.setProperty("height", h + "px", "important");
        } else {
          canvas.style.removeProperty("width");
          canvas.style.removeProperty("height");
        }
      }
    }
  }
};

var EGL = {
  errorCode: 12288,
  defaultDisplayInitialized: false,
  currentContext: 0,
  currentReadSurface: 0,
  currentDrawSurface: 0,
  contextAttributes: {
    alpha: false,
    depth: false,
    stencil: false,
    antialias: false
  },
  stringCache: {},
  setErrorCode(code) {
    EGL.errorCode = code;
  },
  chooseConfig(display, attribList, config, config_size, numConfigs) {
    if (display != 62e3) {
      EGL.setErrorCode(12296);
      return 0;
    }
    if (attribList) {
      // read attribList if it is non-null
      for (;;) {
        var param = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((attribList) >>> 2) >>> 0, ___asan_loadN)];
        if (param == 12321) {
          var alphaSize = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((attribList) + (4)) >>> 2) >>> 0, ___asan_loadN)];
          EGL.contextAttributes.alpha = (alphaSize > 0);
        } else if (param == 12325) {
          var depthSize = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((attribList) + (4)) >>> 2) >>> 0, ___asan_loadN)];
          EGL.contextAttributes.depth = (depthSize > 0);
        } else if (param == 12326) {
          var stencilSize = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
          HEAP32), (((attribList) + (4)) >>> 2) >>> 0, ___asan_loadN)];
          EGL.contextAttributes.stencil = (stencilSize > 0);
        } else if (param == 12337) {
          var samples = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((attribList) + (4)) >>> 2) >>> 0, ___asan_loadN)];
          EGL.contextAttributes.antialias = (samples > 0);
        } else if (param == 12338) {
          var samples = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((attribList) + (4)) >>> 2) >>> 0, ___asan_loadN)];
          EGL.contextAttributes.antialias = (samples == 1);
        } else if (param == 12544) {
          var requestedPriority = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
          HEAP32), (((attribList) + (4)) >>> 2) >>> 0, ___asan_loadN)];
          EGL.contextAttributes.lowLatency = (requestedPriority != 12547);
        } else if (param == 12344) {
          break;
        }
        attribList += 8;
      }
    }
    if ((!config || !config_size) && !numConfigs) {
      EGL.setErrorCode(12300);
      return 0;
    }
    if (numConfigs) {
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((numConfigs) >>> 2) >>> 0, ___asan_storeN)] = 1;
    }
    if (config && config_size > 0) {
      (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((config) >>> 2) >>> 0, ___asan_storeN)] = 62002;
    }
    EGL.setErrorCode(12288);
    return 1;
  }
};

function _eglBindAPI(api) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(21, 0, 1, api);
  if (api == 12448) {
    EGL.setErrorCode(12288);
    return 1;
  }
  // if (api == 0x30A1 /* EGL_OPENVG_API */ || api == 0x30A2 /* EGL_OPENGL_API */) {
  EGL.setErrorCode(12300);
  return 0;
}

function _eglChooseConfig(display, attrib_list, configs, config_size, numConfigs) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(22, 0, 1, display, attrib_list, configs, config_size, numConfigs);
  display >>>= 0;
  attrib_list >>>= 0;
  configs >>>= 0;
  numConfigs >>>= 0;
  return EGL.chooseConfig(display, attrib_list, configs, config_size, numConfigs);
}

var GLctx;

var webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance = ctx => // Closure is expected to be allowed to minify the '.dibvbi' property, so not accessing it quoted.
!!(ctx.dibvbi = ctx.getExtension("WEBGL_draw_instanced_base_vertex_base_instance"));

var webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance = ctx => !!(ctx.mdibvbi = ctx.getExtension("WEBGL_multi_draw_instanced_base_vertex_base_instance"));

var webgl_enable_EXT_polygon_offset_clamp = ctx => !!(ctx.extPolygonOffsetClamp = ctx.getExtension("EXT_polygon_offset_clamp"));

var webgl_enable_EXT_clip_control = ctx => !!(ctx.extClipControl = ctx.getExtension("EXT_clip_control"));

var webgl_enable_WEBGL_polygon_mode = ctx => !!(ctx.webglPolygonMode = ctx.getExtension("WEBGL_polygon_mode"));

var webgl_enable_WEBGL_multi_draw = ctx => // Closure is expected to be allowed to minify the '.multiDrawWebgl' property, so not accessing it quoted.
!!(ctx.multiDrawWebgl = ctx.getExtension("WEBGL_multi_draw"));

var getEmscriptenSupportedExtensions = ctx => {
  // Restrict the list of advertised extensions to those that we actually
  // support.
  var supportedExtensions = [ // WebGL 2 extensions
  "EXT_color_buffer_float", "EXT_conservative_depth", "EXT_disjoint_timer_query_webgl2", "EXT_texture_norm16", "NV_shader_noperspective_interpolation", "WEBGL_clip_cull_distance", // WebGL 1 and WebGL 2 extensions
  "EXT_clip_control", "EXT_color_buffer_half_float", "EXT_depth_clamp", "EXT_float_blend", "EXT_polygon_offset_clamp", "EXT_texture_compression_bptc", "EXT_texture_compression_rgtc", "EXT_texture_filter_anisotropic", "KHR_parallel_shader_compile", "OES_texture_float_linear", "WEBGL_blend_func_extended", "WEBGL_compressed_texture_astc", "WEBGL_compressed_texture_etc", "WEBGL_compressed_texture_etc1", "WEBGL_compressed_texture_s3tc", "WEBGL_compressed_texture_s3tc_srgb", "WEBGL_debug_renderer_info", "WEBGL_debug_shaders", "WEBGL_lose_context", "WEBGL_multi_draw", "WEBGL_polygon_mode" ];
  // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
  return ctx.getSupportedExtensions()?.filter(ext => supportedExtensions.includes(ext)) ?? [];
};

var registerPreMainLoop = f => {
  // Does nothing unless $MainLoop is included/used.
  typeof MainLoop != "undefined" && MainLoop.preMainLoop.push(f);
};

var webglBufferSubData = (target, offset, size, data, src = (growMemViews(), HEAPU8)) => {
  GLctx.bufferSubData(target, offset, src.subarray(data, data + size));
};

var GL = {
  counter: 1,
  buffers: [],
  mappedBuffers: {},
  programs: [],
  framebuffers: [],
  renderbuffers: [],
  textures: [],
  shaders: [],
  vaos: [],
  contexts: {},
  offscreenCanvases: {},
  queries: [],
  samplers: [],
  transformFeedbacks: [],
  syncs: [],
  byteSizeByTypeRoot: 5120,
  byteSizeByType: [ 1, 1, 2, 2, 4, 4, 4, 2, 3, 4, 8 ],
  stringCache: {},
  stringiCache: {},
  unpackAlignment: 4,
  unpackRowLength: 0,
  recordError: errorCode => {
    if (!GL.lastError) {
      GL.lastError = errorCode;
    }
  },
  getNewId: table => {
    var ret = GL.counter++;
    for (var i = table.length; i < ret; i++) {
      table[i] = null;
    }
    // Skip over any non-null elements that might have been created by
    // glBindBuffer.
    while (table[ret]) {
      ret = GL.counter++;
    }
    return ret;
  },
  genObject: (n, buffers, createFunction, objectTable) => {
    for (var i = 0; i < n; i++) {
      var buffer = GLctx[createFunction]();
      var id = buffer && GL.getNewId(objectTable);
      if (buffer) {
        buffer.name = id;
        objectTable[id] = buffer;
      } else {
        GL.recordError(1282);
      }
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((buffers) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = id;
    }
  },
  MAX_TEMP_BUFFER_SIZE: 2097152,
  numTempVertexBuffersPerSize: 64,
  log2ceilLookup: i => 32 - Math.clz32(i ? i - 1 : 0),
  generateTempBuffers: (quads, context) => {
    var largestIndex = GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
    context.tempVertexBufferCounters1 = [];
    context.tempVertexBufferCounters2 = [];
    context.tempVertexBufferCounters1.length = context.tempVertexBufferCounters2.length = largestIndex + 1;
    context.tempVertexBuffers1 = [];
    context.tempVertexBuffers2 = [];
    context.tempVertexBuffers1.length = context.tempVertexBuffers2.length = largestIndex + 1;
    context.tempIndexBuffers = [];
    context.tempIndexBuffers.length = largestIndex + 1;
    for (var i = 0; i <= largestIndex; ++i) {
      context.tempIndexBuffers[i] = null;
      // Created on-demand
      context.tempVertexBufferCounters1[i] = context.tempVertexBufferCounters2[i] = 0;
      var ringbufferLength = GL.numTempVertexBuffersPerSize;
      context.tempVertexBuffers1[i] = [];
      context.tempVertexBuffers2[i] = [];
      var ringbuffer1 = context.tempVertexBuffers1[i];
      var ringbuffer2 = context.tempVertexBuffers2[i];
      ringbuffer1.length = ringbuffer2.length = ringbufferLength;
      for (var j = 0; j < ringbufferLength; ++j) {
        ringbuffer1[j] = ringbuffer2[j] = null;
      }
    }
    if (quads) {
      // GL_QUAD indexes can be precalculated
      context.tempQuadIndexBuffer = GLctx.createBuffer();
      context.GLctx.bindBuffer(34963, context.tempQuadIndexBuffer);
      var numIndexes = GL.MAX_TEMP_BUFFER_SIZE >> 1;
      var quadIndexes = new Uint16Array(numIndexes);
      var i = 0, v = 0;
      while (1) {
        quadIndexes[i++] = v;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v + 1;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v + 2;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v + 2;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v + 3;
        if (i >= numIndexes) break;
        v += 4;
      }
      context.GLctx.bufferData(34963, quadIndexes, 35044);
      context.GLctx.bindBuffer(34963, null);
    }
  },
  getTempVertexBuffer: sizeBytes => {
    var idx = GL.log2ceilLookup(sizeBytes);
    var ringbuffer = GL.currentContext.tempVertexBuffers1[idx];
    var nextFreeBufferIndex = GL.currentContext.tempVertexBufferCounters1[idx];
    GL.currentContext.tempVertexBufferCounters1[idx] = (GL.currentContext.tempVertexBufferCounters1[idx] + 1) & (GL.numTempVertexBuffersPerSize - 1);
    var vbo = ringbuffer[nextFreeBufferIndex];
    if (vbo) {
      return vbo;
    }
    var prevVBO = GLctx.getParameter(34964);
    ringbuffer[nextFreeBufferIndex] = GLctx.createBuffer();
    GLctx.bindBuffer(34962, ringbuffer[nextFreeBufferIndex]);
    GLctx.bufferData(34962, 1 << idx, 35048);
    GLctx.bindBuffer(34962, prevVBO);
    return ringbuffer[nextFreeBufferIndex];
  },
  getTempIndexBuffer: sizeBytes => {
    var idx = GL.log2ceilLookup(sizeBytes);
    var ibo = GL.currentContext.tempIndexBuffers[idx];
    if (ibo) {
      return ibo;
    }
    var prevIBO = GLctx.getParameter(34965);
    GL.currentContext.tempIndexBuffers[idx] = GLctx.createBuffer();
    GLctx.bindBuffer(34963, GL.currentContext.tempIndexBuffers[idx]);
    GLctx.bufferData(34963, 1 << idx, 35048);
    GLctx.bindBuffer(34963, prevIBO);
    return GL.currentContext.tempIndexBuffers[idx];
  },
  newRenderingFrameStarted: () => {
    if (!GL.currentContext) {
      return;
    }
    var vb = GL.currentContext.tempVertexBuffers1;
    GL.currentContext.tempVertexBuffers1 = GL.currentContext.tempVertexBuffers2;
    GL.currentContext.tempVertexBuffers2 = vb;
    vb = GL.currentContext.tempVertexBufferCounters1;
    GL.currentContext.tempVertexBufferCounters1 = GL.currentContext.tempVertexBufferCounters2;
    GL.currentContext.tempVertexBufferCounters2 = vb;
    var largestIndex = GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
    for (var i = 0; i <= largestIndex; ++i) {
      GL.currentContext.tempVertexBufferCounters1[i] = 0;
    }
  },
  getSource: (shader, count, string, length) => {
    var source = "";
    for (var i = 0; i < count; ++i) {
      var len = length ? (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
      HEAPU32), (((length) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)] : undefined;
      source += UTF8ToString((growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
      HEAPU32), (((string) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)], len);
    }
    return source;
  },
  calcBufLength: (size, type, stride, count) => {
    if (stride > 0) {
      return count * stride;
    }
    var typeSize = GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
    return size * typeSize * count;
  },
  preDrawHandleClientVertexAttribBindings: count => {
    GL.resetBufferBinding = false;
    // TODO: initial pass to detect ranges we need to upload, might not need
    // an upload per attrib
    for (var i = 0; i < GL.currentContext.maxVertexAttribs; ++i) {
      var cb = GL.currentContext.clientBuffers[i];
      if (!cb.clientside || !cb.enabled) continue;
      GL.resetBufferBinding = true;
      var size = GL.calcBufLength(cb.size, cb.type, cb.stride, count);
      var buf = GL.getTempVertexBuffer(size);
      GLctx.bindBuffer(34962, buf);
      webglBufferSubData(34962, 0, size, cb.ptr);
      cb.vertexAttribPointerAdaptor.call(GLctx, i, cb.size, cb.type, cb.normalized, cb.stride, 0);
    }
  },
  postDrawHandleClientVertexAttribBindings: () => {
    if (GL.resetBufferBinding) {
      GLctx.bindBuffer(34962, GL.buffers[GLctx.currentArrayBufferBinding]);
    }
  },
  createContext: (/** @type {HTMLCanvasElement} */ canvas, webGLContextAttributes) => {
    // BUG: Workaround Safari WebGL issue: After successfully acquiring WebGL
    // context on a canvas, calling .getContext() will always return that
    // context independent of which 'webgl' or 'webgl2'
    // context version was passed. See:
    //   https://webkit.org/b/222758
    // and:
    //   https://github.com/emscripten-core/emscripten/issues/13295.
    // TODO: Once the bug is fixed and shipped in Safari, adjust the Safari
    // version field in above check.
    if (!canvas.getContextSafariWebGL2Fixed) {
      canvas.getContextSafariWebGL2Fixed = canvas.getContext;
      /** @type {function(this:HTMLCanvasElement, string, (Object|null)=): (RenderingContext|null)} */ function fixedGetContext(ver, attrs) {
        var gl = canvas.getContextSafariWebGL2Fixed(ver, attrs);
        return ((ver == "webgl") == (gl instanceof WebGLRenderingContext)) ? gl : null;
      }
      canvas.getContext = fixedGetContext;
    }
    var ctx = canvas.getContext("webgl2", webGLContextAttributes);
    if (!ctx) return 0;
    var handle = GL.registerContext(ctx, webGLContextAttributes);
    return handle;
  },
  registerContext: (ctx, webGLContextAttributes) => {
    // with pthreads a context is a location in memory with some synchronized
    // data between threads
    var handle = _malloc(8);
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((handle) + (4)) >>> 2) >>> 0, ___asan_storeN)] = _pthread_self();
    // the thread pointer of the thread that owns the control of the context
    var context = {
      handle,
      attributes: webGLContextAttributes,
      version: webGLContextAttributes.majorVersion,
      GLctx: ctx
    };
    // Store the created context object so that we can access the context
    // given a canvas without having to pass the parameters again.
    if (ctx.canvas) ctx.canvas.GLctxObject = context;
    GL.contexts[handle] = context;
    if (typeof webGLContextAttributes.enableExtensionsByDefault == "undefined" || webGLContextAttributes.enableExtensionsByDefault) {
      GL.initExtensions(context);
    }
    context.maxVertexAttribs = context.GLctx.getParameter(34921);
    context.clientBuffers = [];
    for (var i = 0; i < context.maxVertexAttribs; i++) {
      context.clientBuffers[i] = {
        enabled: false,
        clientside: false,
        size: 0,
        type: 0,
        normalized: 0,
        stride: 0,
        ptr: 0,
        vertexAttribPointerAdaptor: null
      };
    }
    GL.generateTempBuffers(false, context);
    return handle;
  },
  makeContextCurrent: contextHandle => {
    // Active Emscripten GL layer context object.
    GL.currentContext = GL.contexts[contextHandle];
    // Active WebGL context object.
    Module["ctx"] = GLctx = GL.currentContext?.GLctx;
    return !(contextHandle && !GLctx);
  },
  getContext: contextHandle => GL.contexts[contextHandle],
  deleteContext: contextHandle => {
    if (GL.currentContext === GL.contexts[contextHandle]) {
      GL.currentContext = null;
    }
    if (typeof JSEvents == "object") {
      // Release all JS event handlers on the DOM element that the GL context is
      // associated with since the context is now deleted.
      JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas);
    }
    // Make sure the canvas object no longer refers to the context object so
    // there are no GC surprises.
    if (GL.contexts[contextHandle]?.GLctx.canvas) {
      GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined;
    }
    _free(GL.contexts[contextHandle].handle);
    GL.contexts[contextHandle] = null;
  },
  initExtensions: context => {
    // If this function is called without a specific context object, init the
    // extensions of the currently active context.
    context ||= GL.currentContext;
    if (context.initExtensionsDone) return;
    context.initExtensionsDone = true;
    var GLctx = context.GLctx;
    // Detect the presence of a few extensions manually, since the GL interop
    // layer itself will need to know if they exist.
    // Extensions that are available in both WebGL 1 and WebGL 2
    webgl_enable_WEBGL_multi_draw(GLctx);
    webgl_enable_EXT_polygon_offset_clamp(GLctx);
    webgl_enable_EXT_clip_control(GLctx);
    webgl_enable_WEBGL_polygon_mode(GLctx);
    // Extensions that are available from WebGL >= 2 (no-op if called on a WebGL 1 context active)
    webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
    webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);
    // On WebGL 2, EXT_disjoint_timer_query is replaced with an alternative
    // that's based on core APIs, and exposes only the queryCounterEXT()
    // entrypoint.
    if (context.version >= 2) {
      GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query_webgl2");
    }
    // However, Firefox exposes the WebGL 1 version on WebGL 2 as well and
    // thus we look for the WebGL 1 version again if the WebGL 2 version
    // isn't present. https://bugzil.la/1328882
    if (context.version < 2 || !GLctx.disjointTimerQueryExt) {
      GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
    }
    for (var ext of getEmscriptenSupportedExtensions(GLctx)) {
      // WEBGL_lose_context, WEBGL_debug_renderer_info and WEBGL_debug_shaders
      // are not enabled by default.
      if (!ext.includes("lose_context") && !ext.includes("debug")) {
        // Call .getExtension() to enable that extension permanently.
        GLctx.getExtension(ext);
      }
    }
  }
};

function _eglCreateContext(display, config, hmm, contextAttribs) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(23, 0, 1, display, config, hmm, contextAttribs);
  display >>>= 0;
  config >>>= 0;
  hmm >>>= 0;
  contextAttribs >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  // EGL 1.4 spec says default EGL_CONTEXT_CLIENT_VERSION is GLES1, but this is not supported by Emscripten.
  // So user must pass EGL_CONTEXT_CLIENT_VERSION == 2 to initialize EGL.
  var glesContextVersion = 1;
  for (;;) {
    var param = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((contextAttribs) >>> 2) >>> 0, ___asan_loadN)];
    if (param == 12440) {
      glesContextVersion = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
      HEAP32), (((contextAttribs) + (4)) >>> 2) >>> 0, ___asan_loadN)];
    } else if (param == 12344) {
      break;
    } else {
      /* EGL1.4 specifies only EGL_CONTEXT_CLIENT_VERSION as supported attribute */ EGL.setErrorCode(12292);
      return 0;
    }
    contextAttribs += 8;
  }
  if (glesContextVersion < 2 || glesContextVersion > 3) {
    EGL.setErrorCode(12293);
    return 0;
  }
  EGL.contextAttributes.majorVersion = glesContextVersion - 1;
  // WebGL 1 is GLES 2, WebGL2 is GLES3
  EGL.contextAttributes.minorVersion = 0;
  EGL.context = GL.createContext(Browser.getCanvas(), EGL.contextAttributes);
  if (EGL.context != 0) {
    EGL.setErrorCode(12288);
    // Run callbacks so that GL emulation works
    GL.makeContextCurrent(EGL.context);
    Browser.useWebGL = true;
    Browser.moduleContextCreatedCallbacks.forEach(callback => callback());
    // Note: This function only creates a context, but it shall not make it active.
    GL.makeContextCurrent(null);
    return 62004;
  } else {
    EGL.setErrorCode(12297);
    // By the EGL 1.4 spec, an implementation that does not support GLES2 (WebGL in this case), this error code is set.
    return 0;
  }
}

function _eglCreateWindowSurface(display, config, win, attrib_list) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(24, 0, 1, display, config, win, attrib_list);
  display >>>= 0;
  config >>>= 0;
  attrib_list >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  if (config != 62002) {
    EGL.setErrorCode(12293);
    return 0;
  }
  // TODO: Examine attrib_list! Parameters that can be present there are:
  // - EGL_RENDER_BUFFER (must be EGL_BACK_BUFFER)
  // - EGL_VG_COLORSPACE (can't be set)
  // - EGL_VG_ALPHA_FORMAT (can't be set)
  EGL.setErrorCode(12288);
  return 62006;
}

function _eglDestroyContext(display, context) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(25, 0, 1, display, context);
  display >>>= 0;
  context >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  if (context != 62004) {
    EGL.setErrorCode(12294);
    return 0;
  }
  GL.deleteContext(EGL.context);
  EGL.setErrorCode(12288);
  if (EGL.currentContext == context) {
    EGL.currentContext = 0;
  }
  return 1;
}

function _eglDestroySurface(display, surface) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(26, 0, 1, display, surface);
  display >>>= 0;
  surface >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  if (surface != 62006) {
    EGL.setErrorCode(12301);
    return 1;
  }
  if (EGL.currentReadSurface == surface) {
    EGL.currentReadSurface = 0;
  }
  if (EGL.currentDrawSurface == surface) {
    EGL.currentDrawSurface = 0;
  }
  EGL.setErrorCode(12288);
  return 1;
}

function _eglGetConfigAttrib(display, config, attribute, value) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(27, 0, 1, display, config, attribute, value);
  display >>>= 0;
  config >>>= 0;
  value >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  if (config != 62002) {
    EGL.setErrorCode(12293);
    return 0;
  }
  if (!value) {
    EGL.setErrorCode(12300);
    return 0;
  }
  EGL.setErrorCode(12288);
  switch (attribute) {
   case 12320:
    // EGL_BUFFER_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = EGL.contextAttributes.alpha ? 32 : 24;
    return 1;

   case 12321:
    // EGL_ALPHA_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = EGL.contextAttributes.alpha ? 8 : 0;
    return 1;

   case 12322:
    // EGL_BLUE_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 8;
    return 1;

   case 12323:
    // EGL_GREEN_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 8;
    return 1;

   case 12324:
    // EGL_RED_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 8;
    return 1;

   case 12325:
    // EGL_DEPTH_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = EGL.contextAttributes.depth ? 24 : 0;
    return 1;

   case 12326:
    // EGL_STENCIL_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = EGL.contextAttributes.stencil ? 8 : 0;
    return 1;

   case 12327:
    // EGL_CONFIG_CAVEAT
    // We can return here one of EGL_NONE (0x3038), EGL_SLOW_CONFIG (0x3050) or EGL_NON_CONFORMANT_CONFIG (0x3051).
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 12344;
    return 1;

   case 12328:
    // EGL_CONFIG_ID
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 62002;
    return 1;

   case 12329:
    // EGL_LEVEL
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 0;
    return 1;

   case 12330:
    // EGL_MAX_PBUFFER_HEIGHT
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 4096;
    return 1;

   case 12331:
    // EGL_MAX_PBUFFER_PIXELS
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 16777216;
    return 1;

   case 12332:
    // EGL_MAX_PBUFFER_WIDTH
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 4096;
    return 1;

   case 12333:
    // EGL_NATIVE_RENDERABLE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 0;
    return 1;

   case 12334:
    // EGL_NATIVE_VISUAL_ID
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 0;
    return 1;

   case 12335:
    // EGL_NATIVE_VISUAL_TYPE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 12344;
    return 1;

   case 12337:
    // EGL_SAMPLES
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = EGL.contextAttributes.antialias ? 4 : 0;
    return 1;

   case 12338:
    // EGL_SAMPLE_BUFFERS
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = EGL.contextAttributes.antialias ? 1 : 0;
    return 1;

   case 12339:
    // EGL_SURFACE_TYPE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 4;
    return 1;

   case 12340:
    // EGL_TRANSPARENT_TYPE
    // If this returns EGL_TRANSPARENT_RGB (0x3052), transparency is used through color-keying. No such thing applies to Emscripten canvas.
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 12344;
    return 1;

   case 12341:
   // EGL_TRANSPARENT_BLUE_VALUE
    case 12342:
   // EGL_TRANSPARENT_GREEN_VALUE
    case 12343:
    // EGL_TRANSPARENT_RED_VALUE
    // "If EGL_TRANSPARENT_TYPE is EGL_NONE, then the values for EGL_TRANSPARENT_RED_VALUE, EGL_TRANSPARENT_GREEN_VALUE, and EGL_TRANSPARENT_BLUE_VALUE are undefined."
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = -1;
    return 1;

   case 12345:
   // EGL_BIND_TO_TEXTURE_RGB
    case 12346:
    // EGL_BIND_TO_TEXTURE_RGBA
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 0;
    return 1;

   case 12347:
    // EGL_MIN_SWAP_INTERVAL
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 0;
    return 1;

   case 12348:
    // EGL_MAX_SWAP_INTERVAL
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 1;
    return 1;

   case 12349:
   // EGL_LUMINANCE_SIZE
    case 12350:
    // EGL_ALPHA_MASK_SIZE
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 0;
    return 1;

   case 12351:
    // EGL_COLOR_BUFFER_TYPE
    // EGL has two types of buffers: EGL_RGB_BUFFER and EGL_LUMINANCE_BUFFER.
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 12430;
    return 1;

   case 12352:
    // EGL_RENDERABLE_TYPE
    // A bit combination of EGL_OPENGL_ES_BIT,EGL_OPENVG_BIT,EGL_OPENGL_ES2_BIT and EGL_OPENGL_BIT.
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 4;
    return 1;

   case 12354:
    // EGL_CONFORMANT
    // "EGL_CONFORMANT is a mask indicating if a client API context created with respect to the corresponding EGLConfig will pass the required conformance tests for that API."
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((value) >>> 2) >>> 0, ___asan_storeN)] = 0;
    return 1;

   default:
    EGL.setErrorCode(12292);
    return 0;
  }
}

function _eglGetDisplay(nativeDisplayType) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(28, 0, 1, nativeDisplayType);
  nativeDisplayType >>>= 0;
  EGL.setErrorCode(12288);
  // Emscripten EGL implementation "emulates" X11, and eglGetDisplay is
  // expected to accept/receive a pointer to an X11 Display object (or
  // EGL_DEFAULT_DISPLAY).
  if (nativeDisplayType != 0 && nativeDisplayType != 1) {
    return 0;
  }
  return 62e3;
}

function _eglGetError() {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(29, 0, 1);
  return EGL.errorCode;
}

function _eglInitialize(display, majorVersion, minorVersion) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(30, 0, 1, display, majorVersion, minorVersion);
  display >>>= 0;
  majorVersion >>>= 0;
  minorVersion >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  if (majorVersion) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((majorVersion) >>> 2) >>> 0, ___asan_storeN)] = 1;
  }
  if (minorVersion) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((minorVersion) >>> 2) >>> 0, ___asan_storeN)] = 4;
  }
  EGL.defaultDisplayInitialized = true;
  EGL.setErrorCode(12288);
  return 1;
}

function _eglMakeCurrent(display, draw, read, context) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(31, 0, 1, display, draw, read, context);
  display >>>= 0;
  draw >>>= 0;
  read >>>= 0;
  context >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
  if (context != 0 && context != 62004) {
    EGL.setErrorCode(12294);
    return 0;
  }
  if ((read != 0 && read != 62006) || (draw != 0 && draw != 62006)) {
    EGL.setErrorCode(12301);
    return 0;
  }
  GL.makeContextCurrent(context ? EGL.context : null);
  EGL.currentContext = context;
  EGL.currentDrawSurface = draw;
  EGL.currentReadSurface = read;
  EGL.setErrorCode(12288);
  return 1;
}

/** @suppress{checkTypes} */ var noLeakCheck = func => {
  if (runtimeInitialized) ___lsan_disable();
  try {
    return func();
  } finally {
    if (runtimeInitialized) ___lsan_enable();
  }
};

var _eglQueryString = (display, name) => noLeakCheck(() => {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(32, 0, 1, display, name);
  display >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
  EGL.setErrorCode(12288);
  if (EGL.stringCache[name]) return EGL.stringCache[name];
  var ret;
  switch (name) {
   case 12371:
    ret = stringToNewUTF8("Emscripten");
    break;

   case 12372:
    ret = stringToNewUTF8("1.4 Emscripten EGL");
    break;

   case 12373:
    ret = stringToNewUTF8("");
    break;

   // Currently not supporting any EGL extensions.
    case 12429:
    ret = stringToNewUTF8("OpenGL_ES");
    break;

   default:
    EGL.setErrorCode(12300);
    return 0;
  }
  EGL.stringCache[name] = ret;
  return ret;
});

function _eglSwapBuffers(dpy, surface) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(33, 0, 1, dpy, surface);
  dpy >>>= 0;
  surface >>>= 0;
  if (!EGL.defaultDisplayInitialized) {
    EGL.setErrorCode(12289);
  } else if (!GLctx) {
    EGL.setErrorCode(12290);
  } else if (GLctx.isContextLost()) {
    EGL.setErrorCode(12302);
  } else {
    // According to documentation this does an implicit flush.
    // Due to discussion at https://github.com/emscripten-core/emscripten/pull/1871
    // the flush was removed since this _may_ result in slowing code down.
    //_glFlush();
    EGL.setErrorCode(12288);
    return 1;
  }
  return 0;
}

/**
   * @param {number=} arg
   * @param {boolean=} noSetTiming
   */ var setMainLoop = (iterFunc, fps, simulateInfiniteLoop, arg, noSetTiming) => {
  MainLoop.func = iterFunc;
  MainLoop.arg = arg;
  var thisMainLoopId = MainLoop.currentlyRunningMainloop;
  function checkIsRunning() {
    if (thisMainLoopId < MainLoop.currentlyRunningMainloop) {
      maybeExit();
      return false;
    }
    return true;
  }
  // We create the loop runner here but it is not actually running until
  // _emscripten_set_main_loop_timing is called (which might happen at a
  // later time).
  MainLoop.runner = function MainLoop_runner() {
    if (ABORT) return;
    if (MainLoop.queue.length > 0) {
      var start = Date.now();
      var blocker = MainLoop.queue.shift();
      blocker.func(blocker.arg);
      if (MainLoop.remainingBlockers) {
        var remaining = MainLoop.remainingBlockers;
        var next = remaining % 1 == 0 ? remaining - 1 : Math.floor(remaining);
        if (blocker.counted) {
          MainLoop.remainingBlockers = next;
        } else {
          // not counted, but move the progress along a tiny bit
          next = next + .5;
          // do not steal all the next one's progress
          MainLoop.remainingBlockers = (8 * remaining + next) / 9;
        }
      }
      MainLoop.updateStatus();
      // catches pause/resume main loop from blocker execution
      if (!checkIsRunning()) return;
      setTimeout(MainLoop.runner, 0);
      return;
    }
    // catch pauses from non-main loop sources
    if (!checkIsRunning()) return;
    // Implement very basic swap interval control
    MainLoop.currentFrameNumber = MainLoop.currentFrameNumber + 1 | 0;
    if (MainLoop.timingMode == 1 && MainLoop.timingValue > 1 && MainLoop.currentFrameNumber % MainLoop.timingValue != 0) {
      // Not the scheduled time to render this frame - skip.
      MainLoop.scheduler();
      return;
    } else if (MainLoop.timingMode == 0) {
      MainLoop.tickStartTime = _emscripten_get_now();
    }
    MainLoop.runIter(iterFunc);
    // catch pauses from the main loop itself
    if (!checkIsRunning()) return;
    MainLoop.scheduler();
  };
  if (!noSetTiming) {
    if (fps > 0) {
      _emscripten_set_main_loop_timing(0, 1e3 / fps);
    } else {
      // Do rAF by rendering each frame (no decimating)
      _emscripten_set_main_loop_timing(1, 1);
    }
    MainLoop.scheduler();
  }
  if (simulateInfiniteLoop) {
    throw "unwind";
  }
};

var MainLoop = {
  func: null,
  scheduler: null,
  currentlyRunningMainloop: 0,
  arg: 0,
  timingMode: 0,
  timingValue: 0,
  currentFrameNumber: 0,
  queue: [],
  preMainLoop: [],
  postMainLoop: [],
  pause() {
    if (MainLoop.scheduler) {
      MainLoop.scheduler = null;
      // Incrementing this signals the previous main loop that it's now become old, and it must return.
      MainLoop.currentlyRunningMainloop++;
      runtimeKeepalivePop();
    }
  },
  resume() {
    MainLoop.currentlyRunningMainloop++;
    var timingMode = MainLoop.timingMode;
    var timingValue = MainLoop.timingValue;
    var func = MainLoop.func;
    MainLoop.func = null;
    // do not set timing and call scheduler, we will do it on the next lines
    setMainLoop(func, 0, false, MainLoop.arg, true);
    _emscripten_set_main_loop_timing(timingMode, timingValue);
    MainLoop.scheduler();
  },
  updateStatus() {
    if (Module["setStatus"]) {
      var message = Module["statusMessage"] || "Please wait...";
      var remaining = MainLoop.remainingBlockers ?? 0;
      var expected = MainLoop.expectedBlockers ?? 0;
      if (remaining) {
        if (remaining < expected) {
          Module["setStatus"](`{message} ({expected - remaining}/{expected})`);
        } else {
          Module["setStatus"](message);
        }
      } else {
        Module["setStatus"]("");
      }
    }
  },
  init() {},
  runIter(func) {
    if (ABORT) return;
    for (var pre of MainLoop.preMainLoop) {
      if (pre() === false) {
        return;
      }
    }
    callUserCallback(func);
    for (var post of MainLoop.postMainLoop) {
      post();
    }
    checkStackCookie();
  },
  nextRAF: 0,
  fakeRequestAnimationFrame(func) {
    // try to keep 60fps between calls to here
    var now = Date.now();
    if (!MainLoop.nextRAF) {
      MainLoop.nextRAF = now + 1e3 / 60;
    } else {
      while (now + 2 >= MainLoop.nextRAF) {
        // fudge a little, to avoid timer jitter causing us to do lots of delay:0
        MainLoop.nextRAF += 1e3 / 60;
      }
    }
    var delay = Math.max(MainLoop.nextRAF - now, 0);
    setTimeout(func, delay);
  },
  requestAnimationFrame(func) {
    if (globalThis.requestAnimationFrame) {
      requestAnimationFrame(func);
    } else {
      MainLoop.fakeRequestAnimationFrame(func);
    }
  }
};

var _emscripten_set_main_loop_timing = (mode, value) => {
  MainLoop.timingMode = mode;
  MainLoop.timingValue = value;
  if (!MainLoop.func) {
    return 1;
  }
  // If there is no existing scheduler then we are transitioning from
  // inactive to active and we add to runtime keepalive counter.
  if (!MainLoop.scheduler) {
    runtimeKeepalivePush();
  }
  if (mode == 0) {
    MainLoop.scheduler = function MainLoop_scheduler_setTimeout() {
      var timeUntilNextTick = Math.max(0, MainLoop.tickStartTime + value - _emscripten_get_now()) | 0;
      setTimeout(MainLoop.runner, timeUntilNextTick);
    };
  } else if (mode == 1) {
    MainLoop.scheduler = function MainLoop_scheduler_rAF() {
      MainLoop.requestAnimationFrame(MainLoop.runner);
    };
  } else {
    if (!MainLoop.setImmediate) {
      if (globalThis.scheduler) {
        // Some modern browsers implement scheduler.postTask, but not all.
        MainLoop.setImmediate = scheduler.postTask.bind(scheduler);
      } else if (globalThis.setImmediate) {
        MainLoop.setImmediate = setImmediate;
      } else {
        // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
        var setImmediates = [];
        var emscriptenMainLoopMessageId = "setimmediate";
        /** @param {Event} event */ var MainLoop_setImmediate_messageHandler = event => {
          if (event.data === emscriptenMainLoopMessageId) {
            event.stopPropagation();
            setImmediates.shift()();
          }
        };
        addEventListener("message", MainLoop_setImmediate_messageHandler, true);
        MainLoop.setImmediate = /** @type{function(function(): ?, ...?): number} */ (func => {
          setImmediates.push(func);
          if (ENVIRONMENT_IS_WORKER) {
            // The postMessge API in a Worker, sends message to the main
            // thread and does not support the `targetOrigin` (*) argument.
            postMessage(emscriptenMainLoopMessageId);
          } else {
            postMessage(emscriptenMainLoopMessageId, "*");
          }
        });
      }
    }
    MainLoop.scheduler = function MainLoop_scheduler_setImmediate() {
      MainLoop.setImmediate(MainLoop.runner);
    };
  }
  return 0;
};

function _eglSwapInterval(display, interval) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(34, 0, 1, display, interval);
  display >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  if (interval == 0) _emscripten_set_main_loop_timing(0, 0); else _emscripten_set_main_loop_timing(1, interval);
  EGL.setErrorCode(12288);
  return 1;
}

function _eglTerminate(display) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(35, 0, 1, display);
  display >>>= 0;
  if (display != 62e3) {
    EGL.setErrorCode(12296);
    return 0;
  }
  EGL.currentContext = 0;
  EGL.currentReadSurface = 0;
  EGL.currentDrawSurface = 0;
  EGL.defaultDisplayInitialized = false;
  EGL.setErrorCode(12288);
  return 1;
}

function _eglWaitClient() {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(36, 0, 1);
  EGL.setErrorCode(12288);
  return 1;
}

var _eglWaitGL = _eglWaitClient;

function _eglWaitNative(nativeEngineId) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(37, 0, 1, nativeEngineId);
  EGL.setErrorCode(12288);
  return 1;
}

var readEmAsmArgsArray = [];

var readEmAsmArgs = (sigPtr, buf) => {
  readEmAsmArgsArray.length = 0;
  var ch;
  // Most arguments are i32s, so shift the buffer pointer so it is a plain
  // index into HEAP32.
  while (ch = (growMemViews(), HEAPU8)[_asan_js_check_index((growMemViews(), HEAPU8), sigPtr++ >>> 0, ___asan_loadN)]) {
    // Floats are always passed as doubles, so all types except for 'i'
    // are 8 bytes and require alignment.
    var wide = (ch != 105);
    wide &= (ch != 112);
    buf += wide && (buf % 8) ? 4 : 0;
    readEmAsmArgsArray.push(// Special case for pointers under wasm64 or CAN_ADDRESS_2GB mode.
    ch == 112 ? (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((buf) >>> 2) >>> 0, ___asan_loadN)] : ch == 106 ? (growMemViews(), 
    HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), ((buf) >>> 3) >>> 0, ___asan_loadN)] : ch == 105 ? (growMemViews(), 
    HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((buf) >>> 2) >>> 0, ___asan_loadN)] : (growMemViews(), 
    HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((buf) >>> 3) >>> 0, ___asan_loadN)]);
    buf += wide ? 8 : 4;
  }
  return readEmAsmArgsArray;
};

var runMainThreadEmAsm = (emAsmAddr, sigPtr, argbuf, sync) => {
  var args = readEmAsmArgs(sigPtr, argbuf);
  if (ENVIRONMENT_IS_PTHREAD) {
    // EM_ASM functions are variadic, receiving the actual arguments as a buffer
    // in memory. the last parameter (argBuf) points to that data. We need to
    // always un-variadify that, *before proxying*, as in the async case this
    // is a stack allocation that LLVM made, which may go away before the main
    // thread gets the message. For that reason we handle proxying *after* the
    // call to readEmAsmArgs, and therefore we do that manually here instead
    // of using __proxy. (And for simplicity, do the same in the sync
    // case as well, even though it's not strictly necessary, to keep the two
    // code paths as similar as possible on both sides.)
    return proxyToMainThread(0, emAsmAddr, sync, ...args);
  }
  return ASM_CONSTS[emAsmAddr](...args);
};

function _emscripten_asm_const_async_on_main_thread(emAsmAddr, sigPtr, argbuf) {
  emAsmAddr >>>= 0;
  sigPtr >>>= 0;
  argbuf >>>= 0;
  return runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 0);
}

var runEmAsmFunction = (code, sigPtr, argbuf) => {
  var args = readEmAsmArgs(sigPtr, argbuf);
  return ASM_CONSTS[code](...args);
};

function _emscripten_asm_const_double(code, sigPtr, argbuf) {
  code >>>= 0;
  sigPtr >>>= 0;
  argbuf >>>= 0;
  return runEmAsmFunction(code, sigPtr, argbuf);
}

function _emscripten_asm_const_int(code, sigPtr, argbuf) {
  code >>>= 0;
  sigPtr >>>= 0;
  argbuf >>>= 0;
  return runEmAsmFunction(code, sigPtr, argbuf);
}

function _emscripten_asm_const_int_sync_on_main_thread(emAsmAddr, sigPtr, argbuf) {
  emAsmAddr >>>= 0;
  sigPtr >>>= 0;
  argbuf >>>= 0;
  return runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1);
}

function _emscripten_asm_const_ptr_sync_on_main_thread(emAsmAddr, sigPtr, argbuf) {
  emAsmAddr >>>= 0;
  sigPtr >>>= 0;
  argbuf >>>= 0;
  return runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1);
}

var _emscripten_check_blocking_allowed = () => {};

var onExits = [];

var addOnExit = cb => onExits.push(cb);

var JSEvents = {
  removeAllEventListeners() {
    while (JSEvents.eventHandlers.length) {
      JSEvents._removeHandler(JSEvents.eventHandlers.length - 1);
    }
    JSEvents.deferredCalls = [];
  },
  registerRemoveEventListeners() {
    if (!JSEvents.removeEventListenersRegistered) {
      addOnExit(JSEvents.removeAllEventListeners);
      JSEvents.removeEventListenersRegistered = true;
    }
  },
  inEventHandler: 0,
  deferredCalls: [],
  deferCall(targetFunction, precedence, argsList) {
    function arraysHaveEqualContent(arrA, arrB) {
      if (arrA.length != arrB.length) return false;
      for (var i = 0; i < arrA.length; i++) {
        if (arrA[i] != arrB[i]) return false;
      }
      return true;
    }
    // Test if the given call was already queued, and if so, don't add it again.
    for (var call of JSEvents.deferredCalls) {
      if (call.targetFunction == targetFunction && arraysHaveEqualContent(call.argsList, argsList)) {
        return;
      }
    }
    JSEvents.deferredCalls.push({
      targetFunction,
      precedence,
      argsList
    });
    JSEvents.deferredCalls.sort((x, y) => x.precedence - y.precedence);
  },
  removeDeferredCalls(targetFunction) {
    JSEvents.deferredCalls = JSEvents.deferredCalls.filter(call => call.targetFunction != targetFunction);
  },
  canPerformEventHandlerRequests() {
    // Browsers that support navigator.userActivation.isActive: https://developer.mozilla.org/en-US/docs/Web/API/UserActivation/isActive
    if (navigator.userActivation) {
      // Verify against transient activation status from UserActivation API
      // whether it is possible to perform a request here without needing to defer. See
      // https://developer.mozilla.org/en-US/docs/Web/Security/User_activation#transient_activation
      // and https://caniuse.com/mdn-api_useractivation
      return navigator.userActivation.isActive;
    }
    return JSEvents.inEventHandler && JSEvents.currentEventHandler.allowsDeferredCalls;
  },
  runDeferredCalls() {
    if (!JSEvents.canPerformEventHandlerRequests()) {
      return;
    }
    var deferredCalls = JSEvents.deferredCalls;
    JSEvents.deferredCalls = [];
    for (var call of deferredCalls) {
      call.targetFunction(...call.argsList);
    }
  },
  eventHandlers: [],
  removeAllHandlersOnTarget: (target, eventTypeString) => {
    for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
      if (JSEvents.eventHandlers[i].target == target && (!eventTypeString || eventTypeString == JSEvents.eventHandlers[i].eventTypeString)) {
        JSEvents._removeHandler(i--);
      }
    }
  },
  _removeHandler(i) {
    var h = JSEvents.eventHandlers[i];
    h.target.removeEventListener(h.eventTypeString, h.eventListenerFunc, h.useCapture);
    JSEvents.eventHandlers.splice(i, 1);
  },
  registerOrRemoveHandler(eventHandler) {
    if (!eventHandler.target) {
      return -4;
    }
    if (eventHandler.callbackfunc) {
      eventHandler.eventListenerFunc = function(event) {
        // Increment nesting count for the event handler.
        ++JSEvents.inEventHandler;
        JSEvents.currentEventHandler = eventHandler;
        // Process any old deferred calls the user has placed.
        JSEvents.runDeferredCalls();
        // Process the actual event, calls back to user C code handler.
        eventHandler.handlerFunc(event);
        // Process any new deferred calls that were placed right now from this event handler.
        JSEvents.runDeferredCalls();
        // Out of event handler - restore nesting count.
        --JSEvents.inEventHandler;
      };
      eventHandler.target.addEventListener(eventHandler.eventTypeString, eventHandler.eventListenerFunc, eventHandler.useCapture);
      JSEvents.eventHandlers.push(eventHandler);
      JSEvents.registerRemoveEventListeners();
    } else {
      for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
        if (JSEvents.eventHandlers[i].target == eventHandler.target && JSEvents.eventHandlers[i].eventTypeString == eventHandler.eventTypeString) {
          JSEvents._removeHandler(i--);
        }
      }
    }
    return 0;
  },
  removeSingleHandler(eventHandler) {
    let success = false;
    for (let i = 0; i < JSEvents.eventHandlers.length; ++i) {
      const handler = JSEvents.eventHandlers[i];
      if (handler.target === eventHandler.target && handler.eventTypeId === eventHandler.eventTypeId && handler.callbackfunc === eventHandler.callbackfunc && handler.userData === eventHandler.userData) {
        // in some very rare cases (ex: Safari / fullscreen events), there is more than 1 handler (eventTypeString is different)
        JSEvents._removeHandler(i--);
        success = true;
      }
    }
    return success ? 0 : -5;
  },
  getTargetThreadForEventCallback(targetThread) {
    switch (targetThread) {
     case 1:
      // The event callback for the current event should be called on the
      // main browser thread. (0 == don't proxy)
      return 0;

     case 2:
      // The event callback for the current event should be backproxied to
      // the thread that is registering the event.
      // This can be 0 in the case that the caller uses
      // EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD but on the main thread
      // itself.
      return PThread.currentProxiedOperationCallerThread;

     default:
      // The event callback for the current event should be proxied to the
      // given specific thread.
      return targetThread;
    }
  },
  getNodeNameForTarget(target) {
    if (target == window) return "#window";
    if (target == screen) return "#screen";
    return target?.nodeName ?? "";
  },
  fullscreenEnabled() {
    return document.fullscreenEnabled ?? document.webkitFullscreenEnabled;
  }
};

/** @type {Object} */ var specialHTMLTargets = [ 0, globalThis.document ?? 0, globalThis.window ?? 0 ];

var maybeCStringToJsString = cString => cString > 2 ? UTF8ToString(cString) : cString;

var findEventTarget = target => {
  target = maybeCStringToJsString(target);
  var domElement = specialHTMLTargets[target] || globalThis.document?.querySelector(target);
  return domElement;
};

var findCanvasEventTarget = findEventTarget;

var getCanvasSizeCallingThread = (target, width, height) => {
  var canvas = findCanvasEventTarget(target);
  if (!canvas) return -4;
  if (!canvas.controlTransferredOffscreen) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((width) >>> 2) >>> 0, ___asan_storeN)] = canvas.width;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((height) >>> 2) >>> 0, ___asan_storeN)] = canvas.height;
  } else {
    return -4;
  }
  return 0;
};

function getCanvasSizeMainThread(target, width, height) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(39, 0, 1, target, width, height);
  return getCanvasSizeCallingThread(target, width, height);
}

function _emscripten_get_canvas_element_size(target, width, height) {
  target >>>= 0;
  width >>>= 0;
  height >>>= 0;
  var canvas = findCanvasEventTarget(target);
  if (canvas) {
    return getCanvasSizeCallingThread(target, width, height);
  }
  return getCanvasSizeMainThread(target, width, height);
}

var stringToUTF8OnStack = str => {
  var size = lengthBytesUTF8(str) + 1;
  var ret = stackAlloc(size);
  stringToUTF8(str, ret, size);
  return ret;
};

var getCanvasElementSize = target => {
  var sp = stackSave();
  var w = stackAlloc(8);
  var h = w + 4;
  var targetInt = stringToUTF8OnStack(target.id);
  var ret = _emscripten_get_canvas_element_size(targetInt, w, h);
  var size = [ (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((w) >>> 2) >>> 0, ___asan_loadN)], (growMemViews(), 
  HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((h) >>> 2) >>> 0, ___asan_loadN)] ];
  stackRestore(sp);
  return size;
};

var setCanvasElementSizeCallingThread = (target, width, height) => {
  var canvas = findCanvasEventTarget(target);
  if (!canvas) return -4;
  if (!canvas.controlTransferredOffscreen) {
    var autoResizeViewport = false;
    if (canvas.GLctxObject?.GLctx) {
      var prevViewport = canvas.GLctxObject.GLctx.getParameter(2978);
      // TODO: Perhaps autoResizeViewport should only be true if FBO 0 is currently active?
      autoResizeViewport = (!prevViewport[0] && !prevViewport[1] && prevViewport[2] === canvas.width && prevViewport[3] === canvas.height);
    }
    canvas.width = width;
    canvas.height = height;
    if (autoResizeViewport) {
      // TODO: Add -sCANVAS_RESIZE_SETS_GL_VIEWPORT=0/1 option (default=1). This is commonly done and several graphics engines depend on this,
      // but this can be quite disruptive.
      canvas.GLctxObject.GLctx.viewport(0, 0, width, height);
    }
  } else {
    return -4;
  }
  return 0;
};

function setCanvasElementSizeMainThread(target, width, height) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(40, 0, 1, target, width, height);
  return setCanvasElementSizeCallingThread(target, width, height);
}

function _emscripten_set_canvas_element_size(target, width, height) {
  target >>>= 0;
  var canvas = findCanvasEventTarget(target);
  if (canvas) {
    return setCanvasElementSizeCallingThread(target, width, height);
  }
  return setCanvasElementSizeMainThread(target, width, height);
}

var setCanvasElementSize = (target, width, height) => {
  if (!target.controlTransferredOffscreen) {
    target.width = width;
    target.height = height;
  } else {
    // This function is being called from high-level JavaScript code instead of asm.js/Wasm,
    // and it needs to synchronously proxy over to another thread, so marshal the string onto the heap to do the call.
    var sp = stackSave();
    var targetInt = stringToUTF8OnStack(target.id);
    _emscripten_set_canvas_element_size(targetInt, width, height);
    stackRestore(sp);
  }
};

var currentFullscreenStrategy = 0;

var callCanvasResizedCallback = strategy => {
  if (strategy.canvasResizedCallback) {
    if (strategy.canvasResizedCallbackTargetThread) __emscripten_run_callback_on_thread(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, 37, 0, 0, strategy.canvasResizedCallbackUserData); else (((a1, a2, a3) => dynCall_iiii(strategy.canvasResizedCallback, a1, a2, a3)))(37, 0, strategy.canvasResizedCallbackUserData);
  }
};

var registerRestoreOldStyle = canvas => {
  var canvasSize = getCanvasElementSize(canvas);
  var oldWidth = canvasSize[0];
  var oldHeight = canvasSize[1];
  var oldCssWidth = canvas.style.width;
  var oldCssHeight = canvas.style.height;
  var oldBackgroundColor = canvas.style.backgroundColor;
  // Chrome reads color from here.
  var oldDocumentBackgroundColor = document.body.style.backgroundColor;
  // IE11 reads color from here.
  // Firefox always has black background color.
  var oldPaddingLeft = canvas.style.paddingLeft;
  // Chrome, FF, Safari
  var oldPaddingRight = canvas.style.paddingRight;
  var oldPaddingTop = canvas.style.paddingTop;
  var oldPaddingBottom = canvas.style.paddingBottom;
  var oldMarginLeft = canvas.style.marginLeft;
  // IE11
  var oldMarginRight = canvas.style.marginRight;
  var oldMarginTop = canvas.style.marginTop;
  var oldMarginBottom = canvas.style.marginBottom;
  var oldDocumentBodyMargin = document.body.style.margin;
  var oldDocumentOverflow = document.documentElement.style.overflow;
  // Chrome, Firefox
  var oldDocumentScroll = document.body.scroll;
  // IE
  var oldImageRendering = canvas.style.imageRendering;
  function restoreOldStyle() {
    if (!getFullscreenElement()) {
      document.removeEventListener("fullscreenchange", restoreOldStyle);
      document.removeEventListener("webkitfullscreenchange", restoreOldStyle);
      setCanvasElementSize(canvas, oldWidth, oldHeight);
      canvas.style.width = oldCssWidth;
      canvas.style.height = oldCssHeight;
      canvas.style.backgroundColor = oldBackgroundColor;
      // Chrome
      // IE11 hack: assigning 'undefined' or an empty string to document.body.style.backgroundColor has no effect, so first assign back the default color
      // before setting the undefined value. Setting undefined value is also important, or otherwise we would later treat that as something that the user
      // had explicitly set so subsequent fullscreen transitions would not set background color properly.
      if (!oldDocumentBackgroundColor) document.body.style.backgroundColor = "white";
      document.body.style.backgroundColor = oldDocumentBackgroundColor;
      // IE11
      canvas.style.paddingLeft = oldPaddingLeft;
      // Chrome, FF, Safari
      canvas.style.paddingRight = oldPaddingRight;
      canvas.style.paddingTop = oldPaddingTop;
      canvas.style.paddingBottom = oldPaddingBottom;
      canvas.style.marginLeft = oldMarginLeft;
      // IE11
      canvas.style.marginRight = oldMarginRight;
      canvas.style.marginTop = oldMarginTop;
      canvas.style.marginBottom = oldMarginBottom;
      document.body.style.margin = oldDocumentBodyMargin;
      document.documentElement.style.overflow = oldDocumentOverflow;
      // Chrome, Firefox
      document.body.scroll = oldDocumentScroll;
      // IE
      canvas.style.imageRendering = oldImageRendering;
      if (canvas.GLctxObject) canvas.GLctxObject.GLctx.viewport(0, 0, oldWidth, oldHeight);
      callCanvasResizedCallback(currentFullscreenStrategy);
    }
  }
  document.addEventListener("fullscreenchange", restoreOldStyle);
  document.addEventListener("webkitfullscreenchange", restoreOldStyle);
  return restoreOldStyle;
};

var setLetterbox = (element, topBottom, leftRight) => {
  // Cannot use margin to specify letterboxes in FF or Chrome, since those ignore margins in fullscreen mode.
  element.style.paddingLeft = element.style.paddingRight = leftRight + "px";
  element.style.paddingTop = element.style.paddingBottom = topBottom + "px";
};

var getBoundingClientRect = e => specialHTMLTargets.indexOf(e) < 0 ? e.getBoundingClientRect() : {
  "left": 0,
  "top": 0
};

var JSEvents_resizeCanvasForFullscreen = (target, strategy) => {
  var restoreOldStyle = registerRestoreOldStyle(target);
  var cssWidth = strategy.softFullscreen ? innerWidth : screen.width;
  var cssHeight = strategy.softFullscreen ? innerHeight : screen.height;
  var rect = getBoundingClientRect(target);
  var windowedCssWidth = rect.width;
  var windowedCssHeight = rect.height;
  var canvasSize = getCanvasElementSize(target);
  var windowedRttWidth = canvasSize[0];
  var windowedRttHeight = canvasSize[1];
  if (strategy.scaleMode == 3) {
    setLetterbox(target, (cssHeight - windowedCssHeight) / 2, (cssWidth - windowedCssWidth) / 2);
    cssWidth = windowedCssWidth;
    cssHeight = windowedCssHeight;
  } else if (strategy.scaleMode == 2) {
    if (cssWidth * windowedRttHeight < windowedRttWidth * cssHeight) {
      var desiredCssHeight = windowedRttHeight * cssWidth / windowedRttWidth;
      setLetterbox(target, (cssHeight - desiredCssHeight) / 2, 0);
      cssHeight = desiredCssHeight;
    } else {
      var desiredCssWidth = windowedRttWidth * cssHeight / windowedRttHeight;
      setLetterbox(target, 0, (cssWidth - desiredCssWidth) / 2);
      cssWidth = desiredCssWidth;
    }
  }
  // If we are adding padding, must choose a background color or otherwise Chrome will give the
  // padding a default white color. Do it only if user has not customized their own background color.
  target.style.backgroundColor ||= "black";
  // IE11 does the same, but requires the color to be set in the document body.
  document.body.style.backgroundColor ||= "black";
  // IE11
  // Firefox always shows black letterboxes independent of style color.
  target.style.width = cssWidth + "px";
  target.style.height = cssHeight + "px";
  if (strategy.filteringMode == 1) {
    target.style.imageRendering = "optimizeSpeed";
    target.style.imageRendering = "-moz-crisp-edges";
    target.style.imageRendering = "-o-crisp-edges";
    target.style.imageRendering = "-webkit-optimize-contrast";
    target.style.imageRendering = "optimize-contrast";
    target.style.imageRendering = "crisp-edges";
    target.style.imageRendering = "pixelated";
  }
  var dpiScale = (strategy.canvasResolutionScaleMode == 2) ? devicePixelRatio : 1;
  if (strategy.canvasResolutionScaleMode != 0) {
    var newWidth = (cssWidth * dpiScale) | 0;
    var newHeight = (cssHeight * dpiScale) | 0;
    setCanvasElementSize(target, newWidth, newHeight);
    if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, newWidth, newHeight);
  }
  return restoreOldStyle;
};

var JSEvents_requestFullscreen = (target, strategy) => {
  // EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT + EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE is a mode where no extra logic is performed to the DOM elements.
  if (strategy.scaleMode != 0 || strategy.canvasResolutionScaleMode != 0) {
    JSEvents_resizeCanvasForFullscreen(target, strategy);
  }
  if (target.requestFullscreen) {
    target.requestFullscreen();
  } else if (target.webkitRequestFullscreen) {
    // Safari didn't Element.requestFullscreen support until 16.4
    // See: https://developer.mozilla.org/en-US/docs/Web/API/Element/requestFullscreen
    target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
  } else {
    return JSEvents.fullscreenEnabled() ? -3 : -1;
  }
  currentFullscreenStrategy = strategy;
  callCanvasResizedCallback(strategy);
  return 0;
};

function _emscripten_exit_fullscreen() {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(38, 0, 1);
  if (!JSEvents.fullscreenEnabled()) return -1;
  // Make sure no queued up calls will fire after this.
  JSEvents.removeDeferredCalls(JSEvents_requestFullscreen);
  var d = specialHTMLTargets[1];
  if (d.exitFullscreen) {
    d.fullscreenElement && d.exitFullscreen();
  } else if (d.webkitExitFullscreen) {
    d.webkitFullscreenElement && d.webkitExitFullscreen();
  } else {
    return -1;
  }
  return 0;
}

var requestPointerLock = target => {
  if (target.requestPointerLock) {
    target.requestPointerLock();
  } else {
    // document.body is known to accept pointer lock, so use that to differentiate if the user passed a bad element,
    // or if the whole browser just doesn't support the feature.
    if (document.body.requestPointerLock) {
      return -3;
    }
    return -1;
  }
  return 0;
};

function _emscripten_exit_pointerlock() {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(41, 0, 1);
  // Make sure no queued up calls will fire after this.
  JSEvents.removeDeferredCalls(requestPointerLock);
  if (!document.exitPointerLock) return -1;
  document.exitPointerLock();
  return 0;
}

var _emscripten_exit_with_live_runtime = () => {
  runtimeKeepalivePush();
  throw "unwind";
};

function _emscripten_force_exit(status) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(42, 0, 1, status);
  __emscripten_runtime_keepalive_clear();
  _exit(status);
}

function _emscripten_get_callstack(flags, str, maxbytes) {
  str >>>= 0;
  var callstack = getCallstack(flags);
  // User can query the required amount of bytes to hold the callstack.
  if (!str || maxbytes <= 0) {
    return lengthBytesUTF8(callstack) + 1;
  }
  // Output callstack string as C string to HEAP.
  var bytesWrittenExcludingNull = stringToUTF8(callstack, str, maxbytes);
  // Return number of bytes written, including null.
  return bytesWrittenExcludingNull + 1;
}

function _emscripten_get_device_pixel_ratio() {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(43, 0, 1);
  return globalThis.devicePixelRatio ?? 1;
}

function _emscripten_get_element_css_size(target, width, height) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(44, 0, 1, target, width, height);
  target >>>= 0;
  width >>>= 0;
  height >>>= 0;
  target = findEventTarget(target);
  if (!target) return -4;
  var rect = getBoundingClientRect(target);
  (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((width) >>> 3) >>> 0, ___asan_storeN)] = rect.width;
  (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((height) >>> 3) >>> 0, ___asan_storeN)] = rect.height;
  return 0;
}

var fillGamepadEventData = (eventStruct, e) => {
  (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((eventStruct) >>> 3) >>> 0, ___asan_storeN)] = e.timestamp;
  for (var i = 0; i < e.axes.length; ++i) {
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), (((eventStruct + i * 8) + (16)) >>> 3) >>> 0, ___asan_storeN)] = e.axes[i];
  }
  for (var i = 0; i < e.buttons.length; ++i) {
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (eventStruct + i) + (1040) >>> 0, ___asan_storeN)] = e.buttons[i].pressed;
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), (((eventStruct + i * 8) + (528)) >>> 3) >>> 0, ___asan_storeN)] = e.buttons[i].value;
  }
  (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (eventStruct) + (1104) >>> 0, ___asan_storeN)] = e.connected;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (1108)) >>> 2) >>> 0, ___asan_storeN)] = e.index;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (8)) >>> 2) >>> 0, ___asan_storeN)] = e.axes.length;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (12)) >>> 2) >>> 0, ___asan_storeN)] = e.buttons.length;
  stringToUTF8(e.id, eventStruct + 1112, 64);
  stringToUTF8(e.mapping, eventStruct + 1176, 64);
};

function _emscripten_get_gamepad_status(index, gamepadState) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(45, 0, 1, index, gamepadState);
  gamepadState >>>= 0;
  // INVALID_PARAM is returned on a Gamepad index that never was there.
  if (index < 0 || index >= JSEvents.lastGamepadState.length) return -5;
  // NO_DATA is returned on a Gamepad index that was removed.
  // For previously disconnected gamepads there should be an empty slot (null/undefined/false) at the index.
  // This is because gamepads must keep their original position in the array.
  // For example, removing the first of two gamepads produces [null/undefined/false, gamepad].
  if (!JSEvents.lastGamepadState[index]) return -7;
  fillGamepadEventData(gamepadState, JSEvents.lastGamepadState[index]);
  return 0;
}

var getHeapMax = () => // Stay one Wasm page short of 4GB: while e.g. Chrome is able to allocate
// full 4GB Wasm memories, the size will wrap back to 0 bytes in Wasm side
// for any code that deals with heap sizes, which would require special
// casing all heap size related code to treat 0 specially.
4294901760;

function _emscripten_get_heap_max() {
  return getHeapMax();
}

function _emscripten_get_num_gamepads() {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(46, 0, 1);
  // N.B. Do not call emscripten_get_num_gamepads() unless having first called emscripten_sample_gamepad_data(), and that has returned EMSCRIPTEN_RESULT_SUCCESS.
  // Otherwise the following line will throw an exception.
  return JSEvents.lastGamepadState.length;
}

function _emscripten_get_screen_size(width, height) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(47, 0, 1, width, height);
  width >>>= 0;
  height >>>= 0;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((width) >>> 2) >>> 0, ___asan_storeN)] = screen.width;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((height) >>> 2) >>> 0, ___asan_storeN)] = screen.height;
}

var _emscripten_glActiveTexture = x0 => GLctx.activeTexture(x0);

var _emscripten_glAttachShader = (program, shader) => {
  GLctx.attachShader(GL.programs[program], GL.shaders[shader]);
};

var _emscripten_glBeginQuery = (target, id) => {
  GLctx.beginQuery(target, GL.queries[id]);
};

var _emscripten_glBeginQueryEXT = (target, id) => {
  GLctx.disjointTimerQueryExt["beginQueryEXT"](target, GL.queries[id]);
};

var _emscripten_glBeginTransformFeedback = x0 => GLctx.beginTransformFeedback(x0);

function _emscripten_glBindAttribLocation(program, index, name) {
  name >>>= 0;
  GLctx.bindAttribLocation(GL.programs[program], index, UTF8ToString(name));
}

var _emscripten_glBindBuffer = (target, buffer) => {
  // Calling glBindBuffer with an unknown buffer will implicitly create a
  // new one.  Here we bypass `GL.counter` and directly using the ID passed
  // in.
  if (buffer && !GL.buffers[buffer]) {
    var b = GLctx.createBuffer();
    b.name = buffer;
    GL.buffers[buffer] = b;
  }
  if (target == 34962) {
    GLctx.currentArrayBufferBinding = buffer;
  } else if (target == 34963) {
    GLctx.currentElementArrayBufferBinding = buffer;
  }
  if (target == 35051) {
    // In WebGL 2 glReadPixels entry point, we need to use a different WebGL 2
    // API function call when a buffer is bound to
    // GL_PIXEL_PACK_BUFFER_BINDING point, so must keep track whether that
    // binding point is non-null to know what is the proper API function to
    // call.
    GLctx.currentPixelPackBufferBinding = buffer;
  } else if (target == 35052) {
    // In WebGL 2 gl(Compressed)Tex(Sub)Image[23]D entry points, we need to
    // use a different WebGL 2 API function call when a buffer is bound to
    // GL_PIXEL_UNPACK_BUFFER_BINDING point, so must keep track whether that
    // binding point is non-null to know what is the proper API function to
    // call.
    GLctx.currentPixelUnpackBufferBinding = buffer;
  }
  GLctx.bindBuffer(target, GL.buffers[buffer]);
};

var _emscripten_glBindBufferBase = (target, index, buffer) => {
  GLctx.bindBufferBase(target, index, GL.buffers[buffer]);
};

function _emscripten_glBindBufferRange(target, index, buffer, offset, ptrsize) {
  offset >>>= 0;
  ptrsize >>>= 0;
  GLctx.bindBufferRange(target, index, GL.buffers[buffer], offset, ptrsize);
}

var _emscripten_glBindFramebuffer = (target, framebuffer) => {
  GLctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
};

var _emscripten_glBindRenderbuffer = (target, renderbuffer) => {
  GLctx.bindRenderbuffer(target, GL.renderbuffers[renderbuffer]);
};

var _emscripten_glBindSampler = (unit, sampler) => {
  GLctx.bindSampler(unit, GL.samplers[sampler]);
};

var _emscripten_glBindTexture = (target, texture) => {
  GLctx.bindTexture(target, GL.textures[texture]);
};

var _emscripten_glBindTransformFeedback = (target, id) => {
  GLctx.bindTransformFeedback(target, GL.transformFeedbacks[id]);
};

var _emscripten_glBindVertexArray = vao => {
  GLctx.bindVertexArray(GL.vaos[vao]);
  var ibo = GLctx.getParameter(34965);
  GLctx.currentElementArrayBufferBinding = ibo ? (ibo.name | 0) : 0;
};

var _emscripten_glBindVertexArrayOES = _emscripten_glBindVertexArray;

var _emscripten_glBlendColor = (x0, x1, x2, x3) => GLctx.blendColor(x0, x1, x2, x3);

var _emscripten_glBlendEquation = x0 => GLctx.blendEquation(x0);

var _emscripten_glBlendEquationSeparate = (x0, x1) => GLctx.blendEquationSeparate(x0, x1);

var _emscripten_glBlendFunc = (x0, x1) => GLctx.blendFunc(x0, x1);

var _emscripten_glBlendFuncSeparate = (x0, x1, x2, x3) => GLctx.blendFuncSeparate(x0, x1, x2, x3);

var _emscripten_glBlitFramebuffer = (x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) => GLctx.blitFramebuffer(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);

function _emscripten_glBufferData(target, size, data, usage) {
  size >>>= 0;
  data >>>= 0;
  // N.b. here first form specifies a heap subarray, second form an integer
  // size, so the ?: code here is polymorphic. It is advised to avoid
  // randomly mixing both uses in calling code, to avoid any potential JS
  // engine JIT issues.
  GLctx.bufferData(target, data ? (growMemViews(), HEAPU8).subarray(data >>> 0, data + size >>> 0) : size, usage);
}

function _emscripten_glBufferSubData(target, offset, size, data) {
  offset >>>= 0;
  size >>>= 0;
  data >>>= 0;
  return webglBufferSubData(target, offset, size, data);
}

var _emscripten_glCheckFramebufferStatus = x0 => GLctx.checkFramebufferStatus(x0);

var _emscripten_glClear = x0 => GLctx.clear(x0);

var _emscripten_glClearBufferfi = (x0, x1, x2, x3) => GLctx.clearBufferfi(x0, x1, x2, x3);

/** @type {!Float32Array} */ var HEAPF32;

function _emscripten_glClearBufferfv(buffer, drawbuffer, value) {
  value >>>= 0;
  GLctx.clearBufferfv(buffer, drawbuffer, (growMemViews(), HEAPF32), ((value) >>> 2));
}

function _emscripten_glClearBufferiv(buffer, drawbuffer, value) {
  value >>>= 0;
  GLctx.clearBufferiv(buffer, drawbuffer, (growMemViews(), HEAP32), ((value) >>> 2));
}

function _emscripten_glClearBufferuiv(buffer, drawbuffer, value) {
  value >>>= 0;
  GLctx.clearBufferuiv(buffer, drawbuffer, (growMemViews(), HEAPU32), ((value) >>> 2));
}

var _emscripten_glClearColor = (x0, x1, x2, x3) => GLctx.clearColor(x0, x1, x2, x3);

var _emscripten_glClearDepthf = x0 => GLctx.clearDepth(x0);

var _emscripten_glClearStencil = x0 => GLctx.clearStencil(x0);

function _emscripten_glClientWaitSync(sync, flags, timeout) {
  sync >>>= 0;
  // WebGL2 vs GLES3 differences: in GLES3, the timeout parameter is a uint64, where 0xFFFFFFFFFFFFFFFFULL means GL_TIMEOUT_IGNORED.
  // In JS, there's no 64-bit value types, so instead timeout is taken to be signed, and GL_TIMEOUT_IGNORED is given value -1.
  // Inherently the value accepted in the timeout is lossy, and can't take in arbitrary u64 bit pattern (but most likely doesn't matter)
  // See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15
  timeout = Number(timeout);
  return GLctx.clientWaitSync(GL.syncs[sync], flags, timeout);
}

var _emscripten_glClipControlEXT = (origin, depth) => {
  GLctx.extClipControl["clipControlEXT"](origin, depth);
};

var _emscripten_glColorMask = (red, green, blue, alpha) => {
  GLctx.colorMask(!!red, !!green, !!blue, !!alpha);
};

var _emscripten_glCompileShader = shader => {
  GLctx.compileShader(GL.shaders[shader]);
};

function _emscripten_glCompressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data) {
  data >>>= 0;
  // `data` may be null here, which means "allocate uninitialized space but
  // don't upload" in GLES parlance, but `compressedTexImage2D` requires the
  // final data parameter, so we simply pass a heap view starting at zero
  // effectively uploading whatever happens to be near address zero.  See
  // https://github.com/emscripten-core/emscripten/issues/19300.
  if (true) {
    if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
      GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);
      return;
    }
  }
  GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, (growMemViews(), 
  HEAPU8).subarray(data >>> 0, data + imageSize >>> 0));
}

function _emscripten_glCompressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, data) {
  data >>>= 0;
  if (GLctx.currentPixelUnpackBufferBinding) {
    GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, data);
  } else {
    GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, (growMemViews(), 
    HEAPU8), data, imageSize);
  }
}

function _emscripten_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
  data >>>= 0;
  if (true) {
    if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
      GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
      return;
    }
  }
  GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, (growMemViews(), 
  HEAPU8).subarray(data >>> 0, data + imageSize >>> 0));
}

function _emscripten_glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) {
  data >>>= 0;
  if (GLctx.currentPixelUnpackBufferBinding) {
    GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
  } else {
    GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, (growMemViews(), 
    HEAPU8), data, imageSize);
  }
}

function _emscripten_glCopyBufferSubData(x0, x1, x2, x3, x4) {
  x2 >>>= 0;
  x3 >>>= 0;
  x4 >>>= 0;
  return GLctx.copyBufferSubData(x0, x1, x2, x3, x4);
}

var _emscripten_glCopyTexImage2D = (x0, x1, x2, x3, x4, x5, x6, x7) => GLctx.copyTexImage2D(x0, x1, x2, x3, x4, x5, x6, x7);

var _emscripten_glCopyTexSubImage2D = (x0, x1, x2, x3, x4, x5, x6, x7) => GLctx.copyTexSubImage2D(x0, x1, x2, x3, x4, x5, x6, x7);

var _emscripten_glCopyTexSubImage3D = (x0, x1, x2, x3, x4, x5, x6, x7, x8) => GLctx.copyTexSubImage3D(x0, x1, x2, x3, x4, x5, x6, x7, x8);

var _emscripten_glCreateProgram = () => {
  var id = GL.getNewId(GL.programs);
  var program = GLctx.createProgram();
  // Store additional information needed for each shader program:
  program.name = id;
  // Lazy cache results of
  // glGetProgramiv(GL_ACTIVE_UNIFORM_MAX_LENGTH/GL_ACTIVE_ATTRIBUTE_MAX_LENGTH/GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)
  program.maxUniformLength = program.maxAttributeLength = program.maxUniformBlockNameLength = 0;
  program.uniformIdCounter = 1;
  GL.programs[id] = program;
  return id;
};

var _emscripten_glCreateShader = shaderType => {
  var id = GL.getNewId(GL.shaders);
  GL.shaders[id] = GLctx.createShader(shaderType);
  return id;
};

var _emscripten_glCullFace = x0 => GLctx.cullFace(x0);

function _emscripten_glDeleteBuffers(n, buffers) {
  buffers >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((buffers) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var buffer = GL.buffers[id];
    // From spec: "glDeleteBuffers silently ignores 0's and names that do not
    // correspond to existing buffer objects."
    if (!buffer) continue;
    GLctx.deleteBuffer(buffer);
    buffer.name = 0;
    GL.buffers[id] = null;
    if (id == GLctx.currentArrayBufferBinding) GLctx.currentArrayBufferBinding = 0;
    if (id == GLctx.currentElementArrayBufferBinding) GLctx.currentElementArrayBufferBinding = 0;
    if (id == GLctx.currentPixelPackBufferBinding) GLctx.currentPixelPackBufferBinding = 0;
    if (id == GLctx.currentPixelUnpackBufferBinding) GLctx.currentPixelUnpackBufferBinding = 0;
  }
}

function _emscripten_glDeleteFramebuffers(n, framebuffers) {
  framebuffers >>>= 0;
  for (var i = 0; i < n; ++i) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((framebuffers) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var framebuffer = GL.framebuffers[id];
    if (!framebuffer) continue;
    // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
    GLctx.deleteFramebuffer(framebuffer);
    framebuffer.name = 0;
    GL.framebuffers[id] = null;
  }
}

var _emscripten_glDeleteProgram = id => {
  if (!id) return;
  var program = GL.programs[id];
  if (!program) {
    // glDeleteProgram actually signals an error when deleting a nonexisting
    // object, unlike some other GL delete functions.
    GL.recordError(1281);
    return;
  }
  GLctx.deleteProgram(program);
  program.name = 0;
  GL.programs[id] = null;
};

function _emscripten_glDeleteQueries(n, ids) {
  ids >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((ids) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var query = GL.queries[id];
    if (!query) continue;
    // GL spec: "unused names in ids are ignored, as is the name zero."
    GLctx.deleteQuery(query);
    GL.queries[id] = null;
  }
}

function _emscripten_glDeleteQueriesEXT(n, ids) {
  ids >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((ids) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var query = GL.queries[id];
    if (!query) continue;
    // GL spec: "unused names in ids are ignored, as is the name zero."
    GLctx.disjointTimerQueryExt["deleteQueryEXT"](query);
    GL.queries[id] = null;
  }
}

function _emscripten_glDeleteRenderbuffers(n, renderbuffers) {
  renderbuffers >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((renderbuffers) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var renderbuffer = GL.renderbuffers[id];
    if (!renderbuffer) continue;
    // GL spec: "glDeleteRenderbuffers silently ignores 0s and names that do not correspond to existing renderbuffer objects".
    GLctx.deleteRenderbuffer(renderbuffer);
    renderbuffer.name = 0;
    GL.renderbuffers[id] = null;
  }
}

function _emscripten_glDeleteSamplers(n, samplers) {
  samplers >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((samplers) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var sampler = GL.samplers[id];
    if (!sampler) continue;
    GLctx.deleteSampler(sampler);
    sampler.name = 0;
    GL.samplers[id] = null;
  }
}

var _emscripten_glDeleteShader = id => {
  if (!id) return;
  var shader = GL.shaders[id];
  if (!shader) {
    // glDeleteShader actually signals an error when deleting a nonexisting
    // object, unlike some other GL delete functions.
    GL.recordError(1281);
    return;
  }
  GLctx.deleteShader(shader);
  GL.shaders[id] = null;
};

function _emscripten_glDeleteSync(id) {
  id >>>= 0;
  if (!id) return;
  var sync = GL.syncs[id];
  if (!sync) {
    // glDeleteSync signals an error when deleting a nonexisting object, unlike some other GL delete functions.
    GL.recordError(1281);
    return;
  }
  GLctx.deleteSync(sync);
  sync.name = 0;
  GL.syncs[id] = null;
}

function _emscripten_glDeleteTextures(n, textures) {
  textures >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((textures) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var texture = GL.textures[id];
    // GL spec: "glDeleteTextures silently ignores 0s and names that do not
    // correspond to existing textures".
    if (!texture) continue;
    GLctx.deleteTexture(texture);
    texture.name = 0;
    GL.textures[id] = null;
  }
}

function _emscripten_glDeleteTransformFeedbacks(n, ids) {
  ids >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((ids) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    var transformFeedback = GL.transformFeedbacks[id];
    if (!transformFeedback) continue;
    // GL spec: "unused names in ids are ignored, as is the name zero."
    GLctx.deleteTransformFeedback(transformFeedback);
    transformFeedback.name = 0;
    GL.transformFeedbacks[id] = null;
  }
}

function _emscripten_glDeleteVertexArrays(n, vaos) {
  vaos >>>= 0;
  for (var i = 0; i < n; i++) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((vaos) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
    GLctx.deleteVertexArray(GL.vaos[id]);
    GL.vaos[id] = null;
  }
}

var _emscripten_glDeleteVertexArraysOES = _emscripten_glDeleteVertexArrays;

var _emscripten_glDepthFunc = x0 => GLctx.depthFunc(x0);

var _emscripten_glDepthMask = flag => {
  GLctx.depthMask(!!flag);
};

var _emscripten_glDepthRangef = (x0, x1) => GLctx.depthRange(x0, x1);

var _emscripten_glDetachShader = (program, shader) => {
  GLctx.detachShader(GL.programs[program], GL.shaders[shader]);
};

var _emscripten_glDisable = x0 => GLctx.disable(x0);

var _emscripten_glDisableVertexAttribArray = index => {
  var cb = GL.currentContext.clientBuffers[index];
  cb.enabled = false;
  GLctx.disableVertexAttribArray(index);
};

var _emscripten_glDrawArrays = (mode, first, count) => {
  // bind any client-side buffers
  GL.preDrawHandleClientVertexAttribBindings(first + count);
  GLctx.drawArrays(mode, first, count);
  GL.postDrawHandleClientVertexAttribBindings();
};

var _emscripten_glDrawArraysInstanced = (mode, first, count, primcount) => {
  GLctx.drawArraysInstanced(mode, first, count, primcount);
};

var _emscripten_glDrawArraysInstancedANGLE = _emscripten_glDrawArraysInstanced;

var _emscripten_glDrawArraysInstancedARB = _emscripten_glDrawArraysInstanced;

var _emscripten_glDrawArraysInstancedEXT = _emscripten_glDrawArraysInstanced;

var _emscripten_glDrawArraysInstancedNV = _emscripten_glDrawArraysInstanced;

var tempFixedLengthArray = [];

function _emscripten_glDrawBuffers(n, bufs) {
  bufs >>>= 0;
  var bufArray = tempFixedLengthArray[n];
  for (var i = 0; i < n; i++) {
    bufArray[i] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((bufs) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
  }
  GLctx.drawBuffers(bufArray);
}

var _emscripten_glDrawBuffersEXT = _emscripten_glDrawBuffers;

var _emscripten_glDrawBuffersWEBGL = _emscripten_glDrawBuffers;

function _emscripten_glDrawElements(mode, count, type, indices) {
  indices >>>= 0;
  var buf;
  var vertexes = 0;
  if (!GLctx.currentElementArrayBufferBinding) {
    var size = GL.calcBufLength(1, type, 0, count);
    buf = GL.getTempIndexBuffer(size);
    GLctx.bindBuffer(34963, buf);
    webglBufferSubData(34963, 0, size, indices);
    // Calculating vertex count if shader's attribute data is on client side
    if (count > 0) {
      for (var i = 0; i < GL.currentContext.maxVertexAttribs; ++i) {
        var cb = GL.currentContext.clientBuffers[i];
        if (cb.clientside && cb.enabled) {
          let arrayClass;
          switch (type) {
           case 5121:
            arrayClass = Uint8Array;
            break;

           case 5123:
            arrayClass = Uint16Array;
            break;

           case 5125:
            arrayClass = Uint32Array;
            break;

           default:
            GL.recordError(1282);
            return;
          }
          vertexes = new arrayClass((growMemViews(), HEAPU8).buffer, indices, count).reduce((max, current) => Math.max(max, current)) + 1;
          break;
        }
      }
    }
    // the index is now 0
    indices = 0;
  }
  // bind any client-side buffers
  GL.preDrawHandleClientVertexAttribBindings(vertexes);
  GLctx.drawElements(mode, count, type, indices);
  GL.postDrawHandleClientVertexAttribBindings();
  if (!GLctx.currentElementArrayBufferBinding) {
    GLctx.bindBuffer(34963, null);
  }
}

function _emscripten_glDrawElementsInstanced(mode, count, type, indices, primcount) {
  indices >>>= 0;
  GLctx.drawElementsInstanced(mode, count, type, indices, primcount);
}

var _emscripten_glDrawElementsInstancedANGLE = _emscripten_glDrawElementsInstanced;

var _emscripten_glDrawElementsInstancedARB = _emscripten_glDrawElementsInstanced;

var _emscripten_glDrawElementsInstancedEXT = _emscripten_glDrawElementsInstanced;

var _emscripten_glDrawElementsInstancedNV = _emscripten_glDrawElementsInstanced;

var _glDrawElements = _emscripten_glDrawElements;

function _emscripten_glDrawRangeElements(mode, start, end, count, type, indices) {
  indices >>>= 0;
  // TODO: This should be a trivial pass-through function registered at the bottom of this page as
  // glFuncs[6][1] += ' drawRangeElements';
  // but due to https://bugzil.la/1202427,
  // we work around by ignoring the range.
  _glDrawElements(mode, count, type, indices);
}

var _emscripten_glEnable = x0 => GLctx.enable(x0);

var _emscripten_glEnableVertexAttribArray = index => {
  var cb = GL.currentContext.clientBuffers[index];
  cb.enabled = true;
  GLctx.enableVertexAttribArray(index);
};

var _emscripten_glEndQuery = x0 => GLctx.endQuery(x0);

var _emscripten_glEndQueryEXT = target => {
  GLctx.disjointTimerQueryExt["endQueryEXT"](target);
};

var _emscripten_glEndTransformFeedback = () => GLctx.endTransformFeedback();

function _emscripten_glFenceSync(condition, flags) {
  var sync = GLctx.fenceSync(condition, flags);
  if (sync) {
    var id = GL.getNewId(GL.syncs);
    sync.name = id;
    GL.syncs[id] = sync;
    return id;
  }
  return 0;
}

var _emscripten_glFinish = () => GLctx.finish();

var _emscripten_glFlush = () => GLctx.flush();

var emscriptenWebGLGetBufferBinding = target => {
  switch (target) {
   case 34962:
    target = 34964;
    break;

   case 34963:
    target = 34965;
    break;

   case 35051:
    target = 35053;
    break;

   case 35052:
    target = 35055;
    break;

   case 35982:
    target = 35983;
    break;

   case 36662:
    target = 36662;
    break;

   case 36663:
    target = 36663;
    break;

   case 35345:
    target = 35368;
    break;
  }
  var buffer = GLctx.getParameter(target);
  if (buffer) return buffer.name | 0; else return 0;
};

var emscriptenWebGLValidateMapBufferTarget = target => {
  switch (target) {
   case 34962:
   // GL_ARRAY_BUFFER
    case 34963:
   // GL_ELEMENT_ARRAY_BUFFER
    case 36662:
   // GL_COPY_READ_BUFFER
    case 36663:
   // GL_COPY_WRITE_BUFFER
    case 35051:
   // GL_PIXEL_PACK_BUFFER
    case 35052:
   // GL_PIXEL_UNPACK_BUFFER
    case 35882:
   // GL_TEXTURE_BUFFER
    case 35982:
   // GL_TRANSFORM_FEEDBACK_BUFFER
    case 35345:
    // GL_UNIFORM_BUFFER
    return true;

   default:
    return false;
  }
};

function _emscripten_glFlushMappedBufferRange(target, offset, length) {
  offset >>>= 0;
  length >>>= 0;
  if (!emscriptenWebGLValidateMapBufferTarget(target)) {
    GL.recordError(1280);
    err("GL_INVALID_ENUM in glFlushMappedBufferRange");
    return;
  }
  var mapping = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
  if (!mapping) {
    GL.recordError(1282);
    err("buffer was never mapped in glFlushMappedBufferRange");
    return;
  }
  if (!(mapping.access & 16)) {
    GL.recordError(1282);
    err("buffer was not mapped with GL_MAP_FLUSH_EXPLICIT_BIT in glFlushMappedBufferRange");
    return;
  }
  if (offset < 0 || length < 0 || offset + length > mapping.length) {
    GL.recordError(1281);
    err("invalid range in glFlushMappedBufferRange");
    return;
  }
  webglBufferSubData(target, mapping.offset, length, mapping.mem + offset);
}

var _emscripten_glFramebufferRenderbuffer = (target, attachment, renderbuffertarget, renderbuffer) => {
  GLctx.framebufferRenderbuffer(target, attachment, renderbuffertarget, GL.renderbuffers[renderbuffer]);
};

var _emscripten_glFramebufferTexture2D = (target, attachment, textarget, texture, level) => {
  GLctx.framebufferTexture2D(target, attachment, textarget, GL.textures[texture], level);
};

var _emscripten_glFramebufferTextureLayer = (target, attachment, texture, level, layer) => {
  GLctx.framebufferTextureLayer(target, attachment, GL.textures[texture], level, layer);
};

var _emscripten_glFrontFace = x0 => GLctx.frontFace(x0);

function _emscripten_glGenBuffers(n, buffers) {
  buffers >>>= 0;
  GL.genObject(n, buffers, "createBuffer", GL.buffers);
}

function _emscripten_glGenFramebuffers(n, ids) {
  ids >>>= 0;
  GL.genObject(n, ids, "createFramebuffer", GL.framebuffers);
}

function _emscripten_glGenQueries(n, ids) {
  ids >>>= 0;
  GL.genObject(n, ids, "createQuery", GL.queries);
}

function _emscripten_glGenQueriesEXT(n, ids) {
  ids >>>= 0;
  for (var i = 0; i < n; i++) {
    var query = GLctx.disjointTimerQueryExt["createQueryEXT"]();
    if (!query) {
      GL.recordError(1282);
      while (i < n) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((ids) + (i++ * 4)) >>> 2) >>> 0, ___asan_storeN)] = 0;
      return;
    }
    var id = GL.getNewId(GL.queries);
    query.name = id;
    GL.queries[id] = query;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((ids) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = id;
  }
}

function _emscripten_glGenRenderbuffers(n, renderbuffers) {
  renderbuffers >>>= 0;
  GL.genObject(n, renderbuffers, "createRenderbuffer", GL.renderbuffers);
}

function _emscripten_glGenSamplers(n, samplers) {
  samplers >>>= 0;
  GL.genObject(n, samplers, "createSampler", GL.samplers);
}

function _emscripten_glGenTextures(n, textures) {
  textures >>>= 0;
  GL.genObject(n, textures, "createTexture", GL.textures);
}

function _emscripten_glGenTransformFeedbacks(n, ids) {
  ids >>>= 0;
  GL.genObject(n, ids, "createTransformFeedback", GL.transformFeedbacks);
}

function _emscripten_glGenVertexArrays(n, arrays) {
  arrays >>>= 0;
  GL.genObject(n, arrays, "createVertexArray", GL.vaos);
}

var _emscripten_glGenVertexArraysOES = _emscripten_glGenVertexArrays;

var _emscripten_glGenerateMipmap = x0 => GLctx.generateMipmap(x0);

var __glGetActiveAttribOrUniform = (funcName, program, index, bufSize, length, size, type, name) => {
  program = GL.programs[program];
  var info = GLctx[funcName](program, index);
  if (info) {
    // If an error occurs, nothing will be written to length, size and type and name.
    var numBytesWrittenExclNull = name && stringToUTF8(info.name, name, bufSize);
    if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = numBytesWrittenExclNull;
    if (size) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((size) >>> 2) >>> 0, ___asan_storeN)] = info.size;
    if (type) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((type) >>> 2) >>> 0, ___asan_storeN)] = info.type;
  }
};

function _emscripten_glGetActiveAttrib(program, index, bufSize, length, size, type, name) {
  length >>>= 0;
  size >>>= 0;
  type >>>= 0;
  name >>>= 0;
  return __glGetActiveAttribOrUniform("getActiveAttrib", program, index, bufSize, length, size, type, name);
}

function _emscripten_glGetActiveUniform(program, index, bufSize, length, size, type, name) {
  length >>>= 0;
  size >>>= 0;
  type >>>= 0;
  name >>>= 0;
  return __glGetActiveAttribOrUniform("getActiveUniform", program, index, bufSize, length, size, type, name);
}

function _emscripten_glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName) {
  length >>>= 0;
  uniformBlockName >>>= 0;
  program = GL.programs[program];
  var result = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
  if (!result) return;
  // If an error occurs, nothing will be written to uniformBlockName or length.
  if (uniformBlockName && bufSize > 0) {
    var numBytesWrittenExclNull = stringToUTF8(result, uniformBlockName, bufSize);
    if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = numBytesWrittenExclNull;
  } else {
    if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = 0;
  }
}

function _emscripten_glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if params == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  program = GL.programs[program];
  if (pname == 35393) {
    var name = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = name.length + 1;
    return;
  }
  var result = GLctx.getActiveUniformBlockParameter(program, uniformBlockIndex, pname);
  if (result === null) return;
  // If an error occurs, nothing should be written to params.
  if (pname == 35395) {
    for (var i = 0; i < result.length; i++) {
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = result[i];
    }
  } else {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = result;
  }
}

function _emscripten_glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params) {
  uniformIndices >>>= 0;
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if params == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  if (uniformCount > 0 && uniformIndices == 0) {
    GL.recordError(1281);
    return;
  }
  program = GL.programs[program];
  var ids = [];
  for (var i = 0; i < uniformCount; i++) {
    ids.push((growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uniformIndices) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)]);
  }
  var result = GLctx.getActiveUniforms(program, ids, pname);
  if (!result) return;
  // GL spec: If an error is generated, nothing is written out to params.
  var len = result.length;
  for (var i = 0; i < len; i++) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = result[i];
  }
}

function _emscripten_glGetAttachedShaders(program, maxCount, count, shaders) {
  count >>>= 0;
  shaders >>>= 0;
  var result = GLctx.getAttachedShaders(GL.programs[program]);
  var len = result.length;
  if (len > maxCount) {
    len = maxCount;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((count) >>> 2) >>> 0, ___asan_storeN)] = len;
  for (var i = 0; i < len; ++i) {
    var id = GL.shaders.indexOf(result[i]);
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((shaders) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = id;
  }
}

function _emscripten_glGetAttribLocation(program, name) {
  name >>>= 0;
  return GLctx.getAttribLocation(GL.programs[program], UTF8ToString(name));
}

var writeI53ToI64 = (ptr, num) => {
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((ptr) >>> 2) >>> 0, ___asan_storeN)] = num;
  var lower = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((ptr) >>> 2) >>> 0, ___asan_loadN)];
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((ptr) + (4)) >>> 2) >>> 0, ___asan_storeN)] = (num - lower) / 4294967296;
};

var webglGetExtensions = () => {
  var exts = getEmscriptenSupportedExtensions(GLctx);
  exts = exts.concat(exts.map(e => "GL_" + e));
  return exts;
};

var emscriptenWebGLGet = (name_, p, type) => {
  // Guard against user passing a null pointer.
  // Note that GLES2 spec does not say anything about how passing a null
  // pointer should be treated.  Testing on desktop core GL 3, the application
  // crashes on glGetIntegerv to a null pointer, but better to report an error
  // instead of doing anything random.
  if (!p) {
    GL.recordError(1281);
    return;
  }
  var ret = undefined;
  switch (name_) {
   // Handle a few trivial GLES values
    case 36346:
    // GL_SHADER_COMPILER
    ret = 1;
    break;

   case 36344:
    // GL_SHADER_BINARY_FORMATS
    if (type != 0 && type != 1) {
      GL.recordError(1280);
    }
    // Do not write anything to the out pointer, since no binary formats are
    // supported.
    return;

   case 34814:
   // GL_NUM_PROGRAM_BINARY_FORMATS
    case 36345:
    // GL_NUM_SHADER_BINARY_FORMATS
    ret = 0;
    break;

   case 34466:
    // GL_NUM_COMPRESSED_TEXTURE_FORMATS
    // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete
    // since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be
    // queried for length), so implement it ourselves to allow C++ GLES2
    // code to get the length.
    var formats = GLctx.getParameter(34467);
    ret = formats ? formats.length : 0;
    break;

   case 33309:
    // GL_NUM_EXTENSIONS
    if (GL.currentContext.version < 2) {
      // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
      GL.recordError(1282);
      return;
    }
    ret = webglGetExtensions().length;
    break;

   case 33307:
   // GL_MAJOR_VERSION
    case 33308:
    // GL_MINOR_VERSION
    if (GL.currentContext.version < 2) {
      GL.recordError(1280);
      // GL_INVALID_ENUM
      return;
    }
    ret = name_ == 33307 ? 3 : 0;
    // return version 3.0
    break;
  }
  if (ret === undefined) {
    var result = GLctx.getParameter(name_);
    switch (typeof result) {
     case "number":
      ret = result;
      break;

     case "boolean":
      ret = result ? 1 : 0;
      break;

     case "string":
      GL.recordError(1280);
      // GL_INVALID_ENUM
      return;

     case "object":
      if (result === null) {
        // null is a valid result for some (e.g., which buffer is bound -
        // perhaps nothing is bound), but otherwise can mean an invalid
        // name_, which we need to report as an error
        switch (name_) {
         case 34964:
         // ARRAY_BUFFER_BINDING
          case 35725:
         // CURRENT_PROGRAM
          case 34965:
         // ELEMENT_ARRAY_BUFFER_BINDING
          case 36006:
         // FRAMEBUFFER_BINDING or DRAW_FRAMEBUFFER_BINDING
          case 36007:
         // RENDERBUFFER_BINDING
          case 32873:
         // TEXTURE_BINDING_2D
          case 34229:
         // WebGL 2 GL_VERTEX_ARRAY_BINDING, or WebGL 1 extension OES_vertex_array_object GL_VERTEX_ARRAY_BINDING_OES
          case 36662:
         // COPY_READ_BUFFER_BINDING or COPY_READ_BUFFER
          case 36663:
         // COPY_WRITE_BUFFER_BINDING or COPY_WRITE_BUFFER
          case 35053:
         // PIXEL_PACK_BUFFER_BINDING
          case 35055:
         // PIXEL_UNPACK_BUFFER_BINDING
          case 36010:
         // READ_FRAMEBUFFER_BINDING
          case 35097:
         // SAMPLER_BINDING
          case 35869:
         // TEXTURE_BINDING_2D_ARRAY
          case 32874:
         // TEXTURE_BINDING_3D
          case 36389:
         // TRANSFORM_FEEDBACK_BINDING
          case 35983:
         // TRANSFORM_FEEDBACK_BUFFER_BINDING
          case 35368:
         // UNIFORM_BUFFER_BINDING
          case 34068:
          {
            // TEXTURE_BINDING_CUBE_MAP
            ret = 0;
            break;
          }

         default:
          {
            GL.recordError(1280);
            // GL_INVALID_ENUM
            return;
          }
        }
      } else if (result instanceof Float32Array || result instanceof Uint32Array || result instanceof Int32Array || result instanceof Array) {
        for (var i = 0; i < result.length; ++i) {
          switch (type) {
           case 0:
            (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((p) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = result[i];
            break;

           case 2:
            (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((p) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = result[i];
            break;

           case 4:
            (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (p) + (i) >>> 0, ___asan_storeN)] = result[i] ? 1 : 0;
            break;
          }
        }
        return;
      } else {
        try {
          ret = result.name | 0;
        } catch (e) {
          GL.recordError(1280);
          // GL_INVALID_ENUM
          err(`GL_INVALID_ENUM in glGet${type}v: Unknown object returned from WebGL getParameter(${name_})! (error: ${e})`);
          return;
        }
      }
      break;

     default:
      GL.recordError(1280);
      // GL_INVALID_ENUM
      err(`GL_INVALID_ENUM in glGet${type}v: Native code calling glGet${type}v(${name_}) and it returns ${result} of type ${typeof (result)}!`);
      return;
    }
  }
  switch (type) {
   case 1:
    writeI53ToI64(p, ret);
    break;

   case 0:
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = ret;
    break;

   case 2:
    (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((p) >>> 2) >>> 0, ___asan_storeN)] = ret;
    break;

   case 4:
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), p >>> 0, ___asan_storeN)] = ret ? 1 : 0;
    break;
  }
};

function _emscripten_glGetBooleanv(name_, p) {
  p >>>= 0;
  return emscriptenWebGLGet(name_, p, 4);
}

function _emscripten_glGetBufferParameteri64v(target, value, data) {
  data >>>= 0;
  if (!data) {
    // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
    // if data == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  writeI53ToI64(data, GLctx.getBufferParameter(target, value));
}

function _emscripten_glGetBufferParameteriv(target, value, data) {
  data >>>= 0;
  if (!data) {
    // GLES2 specification does not specify how to behave if data is a null
    // pointer. Since calling this function does not make sense if data ==
    // null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((data) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getBufferParameter(target, value);
}

function _emscripten_glGetBufferPointerv(target, pname, params) {
  params >>>= 0;
  if (pname == 35005) {
    var ptr = 0;
    var mappedBuffer = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
    if (mappedBuffer) {
      ptr = mappedBuffer.mem;
    }
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = ptr;
  } else {
    GL.recordError(1280);
    err("GL_INVALID_ENUM in glGetBufferPointerv");
  }
}

var _emscripten_glGetError = () => {
  var error = GLctx.getError() || GL.lastError;
  GL.lastError = 0;
  return error;
};

function _emscripten_glGetFloatv(name_, p) {
  p >>>= 0;
  return emscriptenWebGLGet(name_, p, 2);
}

function _emscripten_glGetFragDataLocation(program, name) {
  name >>>= 0;
  return GLctx.getFragDataLocation(GL.programs[program], UTF8ToString(name));
}

function _emscripten_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params) {
  params >>>= 0;
  var result = GLctx.getFramebufferAttachmentParameter(target, attachment, pname);
  if (result instanceof WebGLRenderbuffer || result instanceof WebGLTexture) {
    result = result.name | 0;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = result;
}

var emscriptenWebGLGetIndexed = (target, index, data, type) => {
  if (!data) {
    // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
    // if data == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  var result = GLctx.getIndexedParameter(target, index);
  var ret;
  switch (typeof result) {
   case "boolean":
    ret = result ? 1 : 0;
    break;

   case "number":
    ret = result;
    break;

   case "object":
    if (result === null) {
      switch (target) {
       case 35983:
       // TRANSFORM_FEEDBACK_BUFFER_BINDING
        case 35368:
        // UNIFORM_BUFFER_BINDING
        ret = 0;
        break;

       default:
        {
          GL.recordError(1280);
          // GL_INVALID_ENUM
          return;
        }
      }
    } else if (result instanceof WebGLBuffer) {
      ret = result.name | 0;
    } else {
      GL.recordError(1280);
      // GL_INVALID_ENUM
      return;
    }
    break;

   default:
    GL.recordError(1280);
    // GL_INVALID_ENUM
    return;
  }
  switch (type) {
   case 1:
    writeI53ToI64(data, ret);
    break;

   case 0:
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((data) >>> 2) >>> 0, ___asan_storeN)] = ret;
    break;

   case 2:
    (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((data) >>> 2) >>> 0, ___asan_storeN)] = ret;
    break;

   case 4:
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), data >>> 0, ___asan_storeN)] = ret ? 1 : 0;
    break;

   default:
    abort("internal emscriptenWebGLGetIndexed() error, bad type: " + type);
  }
};

function _emscripten_glGetInteger64i_v(target, index, data) {
  data >>>= 0;
  return emscriptenWebGLGetIndexed(target, index, data, 1);
}

function _emscripten_glGetInteger64v(name_, p) {
  p >>>= 0;
  emscriptenWebGLGet(name_, p, 1);
}

function _emscripten_glGetIntegeri_v(target, index, data) {
  data >>>= 0;
  return emscriptenWebGLGetIndexed(target, index, data, 0);
}

function _emscripten_glGetIntegerv(name_, p) {
  p >>>= 0;
  return emscriptenWebGLGet(name_, p, 0);
}

function _emscripten_glGetInternalformativ(target, internalformat, pname, bufSize, params) {
  params >>>= 0;
  if (bufSize < 0) {
    GL.recordError(1281);
    return;
  }
  if (!params) {
    // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
    // if values == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  var ret = GLctx.getInternalformatParameter(target, internalformat, pname);
  if (ret === null) return;
  for (var i = 0; i < ret.length && i < bufSize; ++i) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = ret[i];
  }
}

function _emscripten_glGetProgramBinary(program, bufSize, length, binaryFormat, binary) {
  length >>>= 0;
  binaryFormat >>>= 0;
  binary >>>= 0;
  GL.recordError(1282);
}

function _emscripten_glGetProgramInfoLog(program, maxLength, length, infoLog) {
  length >>>= 0;
  infoLog >>>= 0;
  var log = GLctx.getProgramInfoLog(GL.programs[program]);
  if (log === null) log = "(unknown error)";
  var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
  if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = numBytesWrittenExclNull;
}

function _emscripten_glGetProgramiv(program, pname, p) {
  p >>>= 0;
  if (!p) {
    // GLES2 specification does not specify how to behave if p is a null
    // pointer. Since calling this function does not make sense if p == null,
    // issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  if (program >= GL.counter) {
    GL.recordError(1281);
    return;
  }
  program = GL.programs[program];
  if (pname == 35716) {
    // GL_INFO_LOG_LENGTH
    var log = GLctx.getProgramInfoLog(program);
    if (log === null) log = "(unknown error)";
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = log.length + 1;
  } else if (pname == 35719) {
    if (!program.maxUniformLength) {
      var numActiveUniforms = GLctx.getProgramParameter(program, 35718);
      for (var i = 0; i < numActiveUniforms; ++i) {
        program.maxUniformLength = Math.max(program.maxUniformLength, GLctx.getActiveUniform(program, i).name.length + 1);
      }
    }
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = program.maxUniformLength;
  } else if (pname == 35722) {
    if (!program.maxAttributeLength) {
      var numActiveAttributes = GLctx.getProgramParameter(program, 35721);
      for (var i = 0; i < numActiveAttributes; ++i) {
        program.maxAttributeLength = Math.max(program.maxAttributeLength, GLctx.getActiveAttrib(program, i).name.length + 1);
      }
    }
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = program.maxAttributeLength;
  } else if (pname == 35381) {
    if (!program.maxUniformBlockNameLength) {
      var numActiveUniformBlocks = GLctx.getProgramParameter(program, 35382);
      for (var i = 0; i < numActiveUniformBlocks; ++i) {
        program.maxUniformBlockNameLength = Math.max(program.maxUniformBlockNameLength, GLctx.getActiveUniformBlockName(program, i).length + 1);
      }
    }
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = program.maxUniformBlockNameLength;
  } else {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getProgramParameter(program, pname);
  }
}

function _emscripten_glGetQueryObjecti64vEXT(id, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if p == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  var query = GL.queries[id];
  var param;
  if (GL.currentContext.version < 2) {
    param = GLctx.disjointTimerQueryExt["getQueryObjectEXT"](query, pname);
  } else {
    param = GLctx.getQueryParameter(query, pname);
  }
  var ret;
  if (typeof param == "boolean") {
    ret = param ? 1 : 0;
  } else {
    ret = param;
  }
  writeI53ToI64(params, ret);
}

function _emscripten_glGetQueryObjectivEXT(id, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if p == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  var query = GL.queries[id];
  var param = GLctx.disjointTimerQueryExt["getQueryObjectEXT"](query, pname);
  var ret;
  if (typeof param == "boolean") {
    ret = param ? 1 : 0;
  } else {
    ret = param;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = ret;
}

var _emscripten_glGetQueryObjectui64vEXT = _emscripten_glGetQueryObjecti64vEXT;

function _emscripten_glGetQueryObjectuiv(id, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if p == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  var query = GL.queries[id];
  var param = GLctx.getQueryParameter(query, pname);
  var ret;
  if (typeof param == "boolean") {
    ret = param ? 1 : 0;
  } else {
    ret = param;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = ret;
}

var _emscripten_glGetQueryObjectuivEXT = _emscripten_glGetQueryObjectivEXT;

function _emscripten_glGetQueryiv(target, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if p == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getQuery(target, pname);
}

function _emscripten_glGetQueryivEXT(target, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if p == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = GLctx.disjointTimerQueryExt["getQueryEXT"](target, pname);
}

function _emscripten_glGetRenderbufferParameteriv(target, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if params == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getRenderbufferParameter(target, pname);
}

function _emscripten_glGetSamplerParameterfv(sampler, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if p == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((params) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getSamplerParameter(GL.samplers[sampler], pname);
}

function _emscripten_glGetSamplerParameteriv(sampler, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
    // if p == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getSamplerParameter(GL.samplers[sampler], pname);
}

function _emscripten_glGetShaderInfoLog(shader, maxLength, length, infoLog) {
  length >>>= 0;
  infoLog >>>= 0;
  var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
  if (log === null) log = "(unknown error)";
  var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
  if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = numBytesWrittenExclNull;
}

function _emscripten_glGetShaderPrecisionFormat(shaderType, precisionType, range, precision) {
  range >>>= 0;
  precision >>>= 0;
  var result = GLctx.getShaderPrecisionFormat(shaderType, precisionType);
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((range) >>> 2) >>> 0, ___asan_storeN)] = result.rangeMin;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((range) + (4)) >>> 2) >>> 0, ___asan_storeN)] = result.rangeMax;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((precision) >>> 2) >>> 0, ___asan_storeN)] = result.precision;
}

function _emscripten_glGetShaderSource(shader, bufSize, length, source) {
  length >>>= 0;
  source >>>= 0;
  var result = GLctx.getShaderSource(GL.shaders[shader]);
  if (!result) return;
  // If an error occurs, nothing will be written to length or source.
  var numBytesWrittenExclNull = (bufSize > 0 && source) ? stringToUTF8(result, source, bufSize) : 0;
  if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = numBytesWrittenExclNull;
}

function _emscripten_glGetShaderiv(shader, pname, p) {
  p >>>= 0;
  if (!p) {
    // GLES2 specification does not specify how to behave if p is a null
    // pointer. Since calling this function does not make sense if p == null,
    // issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  if (pname == 35716) {
    // GL_INFO_LOG_LENGTH
    var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
    if (log === null) log = "(unknown error)";
    // The GLES2 specification says that if the shader has an empty info log,
    // a value of 0 is returned. Otherwise the log has a null char appended.
    // (An empty string is falsey, so we can just check that instead of
    // looking at log.length.)
    var logLength = log ? log.length + 1 : 0;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = logLength;
  } else if (pname == 35720) {
    // GL_SHADER_SOURCE_LENGTH
    var source = GLctx.getShaderSource(GL.shaders[shader]);
    // source may be a null, or the empty string, both of which are falsey
    // values that we report a 0 length for.
    var sourceLength = source ? source.length + 1 : 0;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = sourceLength;
  } else {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((p) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getShaderParameter(GL.shaders[shader], pname);
  }
}

var _emscripten_glGetString = name_ => noLeakCheck(() => {
  var ret = GL.stringCache[name_];
  if (!ret) {
    switch (name_) {
     case 7939:
      ret = stringToNewUTF8(webglGetExtensions().join(" "));
      break;

     case 7936:
     case 7937:
     case 37445:
     case 37446:
      var s = GLctx.getParameter(name_);
      if (!s) {
        GL.recordError(1280);
      }
      ret = s ? stringToNewUTF8(s) : 0;
      break;

     case 7938:
      var webGLVersion = GLctx.getParameter(7938);
      // return GLES version string corresponding to the version of the WebGL context
      var glVersion = `OpenGL ES 2.0 (${webGLVersion})`;
      if (true) glVersion = `OpenGL ES 3.0 (${webGLVersion})`;
      ret = stringToNewUTF8(glVersion);
      break;

     case 35724:
      var glslVersion = GLctx.getParameter(35724);
      // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
      var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
      var ver_num = glslVersion.match(ver_re);
      if (ver_num !== null) {
        if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + "0";
        // ensure minor version has 2 digits
        glslVersion = `OpenGL ES GLSL ES ${ver_num[1]} (${glslVersion})`;
      }
      ret = stringToNewUTF8(glslVersion);
      break;

     default:
      GL.recordError(1280);
    }
    GL.stringCache[name_] = ret;
  }
  return ret;
});

function _emscripten_glGetStringi(name, index) {
  if (GL.currentContext.version < 2) {
    GL.recordError(1282);
    // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
    return 0;
  }
  var stringiCache = GL.stringiCache[name];
  if (stringiCache) {
    if (index < 0 || index >= stringiCache.length) {
      GL.recordError(1281);
      return 0;
    }
    return stringiCache[index];
  }
  switch (name) {
   case 7939:
    var exts = webglGetExtensions().map(stringToNewUTF8);
    stringiCache = GL.stringiCache[name] = exts;
    if (index < 0 || index >= stringiCache.length) {
      GL.recordError(1281);
      return 0;
    }
    return stringiCache[index];

   default:
    GL.recordError(1280);
    return 0;
  }
}

function _emscripten_glGetSynciv(sync, pname, bufSize, length, values) {
  sync >>>= 0;
  length >>>= 0;
  values >>>= 0;
  if (bufSize < 0) {
    // GLES3 specification does not specify how to behave if bufSize < 0, however in the spec wording for glGetInternalformativ, it does say that GL_INVALID_VALUE should be raised,
    // so raise GL_INVALID_VALUE here as well.
    GL.recordError(1281);
    return;
  }
  if (!values) {
    // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
    // if values == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  var ret = GLctx.getSyncParameter(GL.syncs[sync], pname);
  if (ret !== null) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((values) >>> 2) >>> 0, ___asan_storeN)] = ret;
    if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = 1;
  }
}

function _emscripten_glGetTexParameterfv(target, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null
    // pointer. Since calling this function does not make sense if p == null,
    // issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((params) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getTexParameter(target, pname);
}

function _emscripten_glGetTexParameteriv(target, pname, params) {
  params >>>= 0;
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null
    // pointer. Since calling this function does not make sense if p == null,
    // issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getTexParameter(target, pname);
}

function _emscripten_glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name) {
  length >>>= 0;
  size >>>= 0;
  type >>>= 0;
  name >>>= 0;
  program = GL.programs[program];
  var info = GLctx.getTransformFeedbackVarying(program, index);
  if (!info) return;
  // If an error occurred, the return parameters length, size, type and name will be unmodified.
  if (name && bufSize > 0) {
    var numBytesWrittenExclNull = stringToUTF8(info.name, name, bufSize);
    if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = numBytesWrittenExclNull;
  } else {
    if (length) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((length) >>> 2) >>> 0, ___asan_storeN)] = 0;
  }
  if (size) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((size) >>> 2) >>> 0, ___asan_storeN)] = info.size;
  if (type) (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((type) >>> 2) >>> 0, ___asan_storeN)] = info.type;
}

function _emscripten_glGetUniformBlockIndex(program, uniformBlockName) {
  uniformBlockName >>>= 0;
  return GLctx.getUniformBlockIndex(GL.programs[program], UTF8ToString(uniformBlockName));
}

function _emscripten_glGetUniformIndices(program, uniformCount, uniformNames, uniformIndices) {
  uniformNames >>>= 0;
  uniformIndices >>>= 0;
  if (!uniformIndices) {
    // GLES2 specification does not specify how to behave if uniformIndices is a null pointer. Since calling this function does not make sense
    // if uniformIndices == null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  if (uniformCount > 0 && (uniformNames == 0 || uniformIndices == 0)) {
    GL.recordError(1281);
    return;
  }
  program = GL.programs[program];
  var names = [];
  for (var i = 0; i < uniformCount; i++) names.push(UTF8ToString((growMemViews(), 
  HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((uniformNames) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)]));
  var result = GLctx.getUniformIndices(program, names);
  if (!result) return;
  // GL spec: If an error is generated, nothing is written out to uniformIndices.
  var len = result.length;
  for (var i = 0; i < len; i++) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uniformIndices) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = result[i];
  }
}

/** @suppress {checkTypes} */ var jstoi_q = str => parseInt(str);

/** @noinline */ var webglGetLeftBracePos = name => name.slice(-1) == "]" && name.lastIndexOf("[");

var webglPrepareUniformLocationsBeforeFirstUse = program => {
  var uniformLocsById = program.uniformLocsById, // Maps GLuint -> WebGLUniformLocation
  uniformSizeAndIdsByName = program.uniformSizeAndIdsByName, // Maps name -> [uniform array length, GLuint]
  i, j;
  // On the first time invocation of glGetUniformLocation on this shader program:
  // initialize cache data structures and discover which uniforms are arrays.
  if (!uniformLocsById) {
    // maps GLint integer locations to WebGLUniformLocations
    program.uniformLocsById = uniformLocsById = {};
    // maps integer locations back to uniform name strings, so that we can lazily fetch uniform array locations
    program.uniformArrayNamesById = {};
    var numActiveUniforms = GLctx.getProgramParameter(program, 35718);
    for (i = 0; i < numActiveUniforms; ++i) {
      var u = GLctx.getActiveUniform(program, i);
      var nm = u.name;
      var sz = u.size;
      var lb = webglGetLeftBracePos(nm);
      var arrayName = lb > 0 ? nm.slice(0, lb) : nm;
      // Assign a new location.
      var id = program.uniformIdCounter;
      program.uniformIdCounter += sz;
      // Eagerly get the location of the uniformArray[0] base element.
      // The remaining indices >0 will be left for lazy evaluation to
      // improve performance. Those may never be needed to fetch, if the
      // application fills arrays always in full starting from the first
      // element of the array.
      uniformSizeAndIdsByName[arrayName] = [ sz, id ];
      // Store placeholder integers in place that highlight that these
      // >0 index locations are array indices pending population.
      for (j = 0; j < sz; ++j) {
        uniformLocsById[id] = j;
        program.uniformArrayNamesById[id++] = arrayName;
      }
    }
  }
};

function _emscripten_glGetUniformLocation(program, name) {
  name >>>= 0;
  name = UTF8ToString(name);
  if (program = GL.programs[program]) {
    webglPrepareUniformLocationsBeforeFirstUse(program);
    var uniformLocsById = program.uniformLocsById;
    // Maps GLuint -> WebGLUniformLocation
    var arrayIndex = 0;
    var uniformBaseName = name;
    // Invariant: when populating integer IDs for uniform locations, we must
    // maintain the precondition that arrays reside in contiguous addresses,
    // i.e. for a 'vec4 colors[10];', colors[4] must be at location
    // colors[0]+4.  However, user might call glGetUniformLocation(program,
    // "colors") for an array, so we cannot discover based on the user input
    // arguments whether the uniform we are dealing with is an array. The only
    // way to discover which uniforms are arrays is to enumerate over all the
    // active uniforms in the program.
    var leftBrace = webglGetLeftBracePos(name);
    // If user passed an array accessor "[index]", parse the array index off the accessor.
    if (leftBrace > 0) {
      arrayIndex = jstoi_q(name.slice(leftBrace + 1)) >>> 0;
      // "index]", coerce parseInt(']') with >>>0 to treat "foo[]" as "foo[0]" and foo[-1] as unsigned out-of-bounds.
      uniformBaseName = name.slice(0, leftBrace);
    }
    // Have we cached the location of this uniform before?
    // A pair [array length, GLint of the uniform location]
    var sizeAndId = program.uniformSizeAndIdsByName[uniformBaseName];
    // If a uniform with this name exists, and if its index is within the
    // array limits (if it's even an array), query the WebGLlocation, or
    // return an existing cached location.
    if (sizeAndId && arrayIndex < sizeAndId[0]) {
      arrayIndex += sizeAndId[1];
      // Add the base location of the uniform to the array index offset.
      if ((uniformLocsById[arrayIndex] = uniformLocsById[arrayIndex] || GLctx.getUniformLocation(program, name))) {
        return arrayIndex;
      }
    }
  } else {
    // N.b. we are currently unable to distinguish between GL program IDs that
    // never existed vs GL program IDs that have been deleted, so report
    // GL_INVALID_VALUE in both cases.
    GL.recordError(1281);
  }
  return -1;
}

var webglGetProgramUniformLocation = (program, location) => {
  if (program) {
    var webglLoc = program.uniformLocsById[location];
    // program.uniformLocsById[location] stores either an integer, or a
    // WebGLUniformLocation.
    // If an integer, we have not yet bound the location, so do it now. The
    // integer value specifies the array index we should bind to.
    if (typeof webglLoc == "number") {
      program.uniformLocsById[location] = webglLoc = GLctx.getUniformLocation(program, program.uniformArrayNamesById[location] + (webglLoc > 0 ? `[${webglLoc}]` : ""));
    }
    // Else an already cached WebGLUniformLocation, return it.
    return webglLoc;
  } else {
    GL.recordError(1282);
  }
};

/** @suppress{checkTypes} */ var emscriptenWebGLGetUniform = (program, location, params, type) => {
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null
    // pointer. Since calling this function does not make sense if params ==
    // null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  program = GL.programs[program];
  webglPrepareUniformLocationsBeforeFirstUse(program);
  var data = GLctx.getUniform(program, webglGetProgramUniformLocation(program, location));
  if (typeof data == "number" || typeof data == "boolean") {
    switch (type) {
     case 0:
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = data;
      break;

     case 2:
      (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((params) >>> 2) >>> 0, ___asan_storeN)] = data;
      break;
    }
  } else {
    for (var i = 0; i < data.length; i++) {
      switch (type) {
       case 0:
        (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = data[i];
        break;

       case 2:
        (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = data[i];
        break;
      }
    }
  }
};

function _emscripten_glGetUniformfv(program, location, params) {
  params >>>= 0;
  emscriptenWebGLGetUniform(program, location, params, 2);
}

function _emscripten_glGetUniformiv(program, location, params) {
  params >>>= 0;
  emscriptenWebGLGetUniform(program, location, params, 0);
}

function _emscripten_glGetUniformuiv(program, location, params) {
  params >>>= 0;
  return emscriptenWebGLGetUniform(program, location, params, 0);
}

/** @suppress{checkTypes} */ var emscriptenWebGLGetVertexAttrib = (index, pname, params, type) => {
  if (!params) {
    // GLES2 specification does not specify how to behave if params is a null
    // pointer. Since calling this function does not make sense if params ==
    // null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  if (GL.currentContext.clientBuffers[index].enabled) {
    err("glGetVertexAttrib*v on client-side array: not supported, bad data returned");
  }
  var data = GLctx.getVertexAttrib(index, pname);
  if (pname == 34975) {
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = data && data["name"];
  } else if (typeof data == "number" || typeof data == "boolean") {
    switch (type) {
     case 0:
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = data;
      break;

     case 2:
      (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((params) >>> 2) >>> 0, ___asan_storeN)] = data;
      break;

     case 5:
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_storeN)] = Math.fround(data);
      break;
    }
  } else {
    for (var i = 0; i < data.length; i++) {
      switch (type) {
       case 0:
        (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = data[i];
        break;

       case 2:
        (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = data[i];
        break;

       case 5:
        (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((params) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = Math.fround(data[i]);
        break;
      }
    }
  }
};

function _emscripten_glGetVertexAttribIiv(index, pname, params) {
  params >>>= 0;
  // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttribI4iv(),
  // otherwise the results are undefined. (GLES3 spec 6.1.12)
  emscriptenWebGLGetVertexAttrib(index, pname, params, 0);
}

var _emscripten_glGetVertexAttribIuiv = _emscripten_glGetVertexAttribIiv;

function _emscripten_glGetVertexAttribPointerv(index, pname, pointer) {
  pointer >>>= 0;
  if (!pointer) {
    // GLES2 specification does not specify how to behave if pointer is a null
    // pointer. Since calling this function does not make sense if pointer ==
    // null, issue a GL error to notify user about it.
    GL.recordError(1281);
    return;
  }
  if (GL.currentContext.clientBuffers[index].enabled) {
    err("glGetVertexAttribPointer on client-side array: not supported, bad data returned");
  }
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((pointer) >>> 2) >>> 0, ___asan_storeN)] = GLctx.getVertexAttribOffset(index, pname);
}

function _emscripten_glGetVertexAttribfv(index, pname, params) {
  params >>>= 0;
  // N.B. This function may only be called if the vertex attribute was
  // specified using the function glVertexAttrib*f(), otherwise the results
  // are undefined. (GLES3 spec 6.1.12)
  emscriptenWebGLGetVertexAttrib(index, pname, params, 2);
}

function _emscripten_glGetVertexAttribiv(index, pname, params) {
  params >>>= 0;
  // N.B. This function may only be called if the vertex attribute was
  // specified using the function glVertexAttrib*f(), otherwise the results
  // are undefined. (GLES3 spec 6.1.12)
  emscriptenWebGLGetVertexAttrib(index, pname, params, 5);
}

var _emscripten_glHint = (x0, x1) => GLctx.hint(x0, x1);

function _emscripten_glInvalidateFramebuffer(target, numAttachments, attachments) {
  attachments >>>= 0;
  var list = tempFixedLengthArray[numAttachments];
  for (var i = 0; i < numAttachments; i++) {
    list[i] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((attachments) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
  }
  GLctx.invalidateFramebuffer(target, list);
}

function _emscripten_glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height) {
  attachments >>>= 0;
  var list = tempFixedLengthArray[numAttachments];
  for (var i = 0; i < numAttachments; i++) {
    list[i] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((attachments) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)];
  }
  GLctx.invalidateSubFramebuffer(target, list, x, y, width, height);
}

var _emscripten_glIsBuffer = buffer => {
  var b = GL.buffers[buffer];
  if (!b) return 0;
  return GLctx.isBuffer(b);
};

var _emscripten_glIsEnabled = x0 => GLctx.isEnabled(x0);

var _emscripten_glIsFramebuffer = framebuffer => {
  var fb = GL.framebuffers[framebuffer];
  if (!fb) return 0;
  return GLctx.isFramebuffer(fb);
};

var _emscripten_glIsProgram = program => {
  program = GL.programs[program];
  if (!program) return 0;
  return GLctx.isProgram(program);
};

var _emscripten_glIsQuery = id => {
  var query = GL.queries[id];
  if (!query) return 0;
  return GLctx.isQuery(query);
};

var _emscripten_glIsQueryEXT = id => {
  var query = GL.queries[id];
  if (!query) return 0;
  return GLctx.disjointTimerQueryExt["isQueryEXT"](query);
};

var _emscripten_glIsRenderbuffer = renderbuffer => {
  var rb = GL.renderbuffers[renderbuffer];
  if (!rb) return 0;
  return GLctx.isRenderbuffer(rb);
};

var _emscripten_glIsSampler = id => {
  var sampler = GL.samplers[id];
  if (!sampler) return 0;
  return GLctx.isSampler(sampler);
};

var _emscripten_glIsShader = shader => {
  var s = GL.shaders[shader];
  if (!s) return 0;
  return GLctx.isShader(s);
};

function _emscripten_glIsSync(sync) {
  sync >>>= 0;
  return GLctx.isSync(GL.syncs[sync]);
}

var _emscripten_glIsTexture = id => {
  var texture = GL.textures[id];
  if (!texture) return 0;
  return GLctx.isTexture(texture);
};

var _emscripten_glIsTransformFeedback = id => GLctx.isTransformFeedback(GL.transformFeedbacks[id]);

var _emscripten_glIsVertexArray = array => {
  var vao = GL.vaos[array];
  if (!vao) return 0;
  return GLctx.isVertexArray(vao);
};

var _emscripten_glIsVertexArrayOES = _emscripten_glIsVertexArray;

var _emscripten_glLineWidth = x0 => GLctx.lineWidth(x0);

var _emscripten_glLinkProgram = program => {
  program = GL.programs[program];
  GLctx.linkProgram(program);
  // Invalidate earlier computed uniform->ID mappings, those have now become stale
  program.uniformLocsById = 0;
  // Mark as null-like so that glGetUniformLocation() knows to populate this again.
  program.uniformSizeAndIdsByName = {};
};

function _emscripten_glMapBufferRange(target, offset, length, access) {
  offset >>>= 0;
  length >>>= 0;
  if ((access & (1 | 32)) != 0) {
    err("glMapBufferRange access does not support MAP_READ or MAP_UNSYNCHRONIZED");
    return 0;
  }
  if ((access & 2) == 0) {
    err("glMapBufferRange access must include MAP_WRITE");
    return 0;
  }
  if ((access & (4 | 8)) == 0) {
    err("glMapBufferRange access must include INVALIDATE_BUFFER or INVALIDATE_RANGE");
    return 0;
  }
  if (!emscriptenWebGLValidateMapBufferTarget(target)) {
    GL.recordError(1280);
    err("GL_INVALID_ENUM in glMapBufferRange");
    return 0;
  }
  var mem = _malloc(length), binding = emscriptenWebGLGetBufferBinding(target);
  if (!mem) return 0;
  binding = GL.mappedBuffers[binding] ??= {};
  binding.offset = offset;
  binding.length = length;
  binding.mem = mem;
  binding.access = access;
  return mem;
}

var _emscripten_glPauseTransformFeedback = () => GLctx.pauseTransformFeedback();

var _emscripten_glPixelStorei = (pname, param) => {
  if (pname == 3317) {
    GL.unpackAlignment = param;
  } else if (pname == 3314) {
    GL.unpackRowLength = param;
  }
  GLctx.pixelStorei(pname, param);
};

var _emscripten_glPolygonModeWEBGL = (face, mode) => {
  GLctx.webglPolygonMode["polygonModeWEBGL"](face, mode);
};

var _emscripten_glPolygonOffset = (x0, x1) => GLctx.polygonOffset(x0, x1);

var _emscripten_glPolygonOffsetClampEXT = (factor, units, clamp) => {
  GLctx.extPolygonOffsetClamp["polygonOffsetClampEXT"](factor, units, clamp);
};

function _emscripten_glProgramBinary(program, binaryFormat, binary, length) {
  binary >>>= 0;
  GL.recordError(1280);
}

var _emscripten_glProgramParameteri = (program, pname, value) => {
  GL.recordError(1280);
};

var _emscripten_glQueryCounterEXT = (id, target) => {
  GLctx.disjointTimerQueryExt["queryCounterEXT"](GL.queries[id], target);
};

var _emscripten_glReadBuffer = x0 => GLctx.readBuffer(x0);

var computeUnpackAlignedImageSize = (width, height, sizePerPixel) => {
  function roundedToNextMultipleOf(x, y) {
    return (x + y - 1) & -y;
  }
  var plainRowSize = (GL.unpackRowLength || width) * sizePerPixel;
  var alignedRowSize = roundedToNextMultipleOf(plainRowSize, GL.unpackAlignment);
  return height * alignedRowSize;
};

var colorChannelsInGlTextureFormat = format => {
  // Micro-optimizations for size: map format to size by subtracting smallest
  // enum value (0x1902) from all values first.  Also omit the most common
  // size value (1) from the list, which is assumed by formats not on the
  // list.
  var colorChannels = {
    // 0x1902 /* GL_DEPTH_COMPONENT */ - 0x1902: 1,
    // 0x1906 /* GL_ALPHA */ - 0x1902: 1,
    5: 3,
    6: 4,
    // 0x1909 /* GL_LUMINANCE */ - 0x1902: 1,
    8: 2,
    29502: 3,
    29504: 4,
    // 0x1903 /* GL_RED */ - 0x1902: 1,
    26917: 2,
    26918: 2,
    // 0x8D94 /* GL_RED_INTEGER */ - 0x1902: 1,
    29846: 3,
    29847: 4
  };
  return colorChannels[format - 6402] || 1;
};

/** @type {!Uint16Array} */ var HEAPU16;

var heapObjectForWebGLType = type => {
  // Micro-optimization for size: Subtract lowest GL enum number (0x1400/* GL_BYTE */) from type to compare
  // smaller values for the heap, for shorter generated code size.
  // Also the type HEAPU16 is not tested for explicitly, but any unrecognized type will return out HEAPU16.
  // (since most types are HEAPU16)
  type -= 5120;
  if (type == 0) return growMemViews(), HEAP8;
  if (type == 1) return growMemViews(), HEAPU8;
  if (type == 2) return growMemViews(), HEAP16;
  if (type == 4) return growMemViews(), HEAP32;
  if (type == 6) return growMemViews(), HEAPF32;
  if (type == 5 || type == 28922 || type == 28520 || type == 30779 || type == 30782) return growMemViews(), 
  HEAPU32;
  return growMemViews(), HEAPU16;
};

var toTypedArrayIndex = (pointer, heap) => pointer >>> (31 - Math.clz32(heap.BYTES_PER_ELEMENT));

var emscriptenWebGLGetTexPixelData = (type, format, width, height, pixels) => {
  var heap = heapObjectForWebGLType(type);
  var sizePerPixel = colorChannelsInGlTextureFormat(format) * heap.BYTES_PER_ELEMENT;
  var bytes = computeUnpackAlignedImageSize(width, height, sizePerPixel);
  return heap.subarray(toTypedArrayIndex(pixels, heap) >>> 0, toTypedArrayIndex(pixels + bytes, heap) >>> 0);
};

function _emscripten_glReadPixels(x, y, width, height, format, type, pixels) {
  pixels >>>= 0;
  if (true) {
    if (GLctx.currentPixelPackBufferBinding) {
      GLctx.readPixels(x, y, width, height, format, type, pixels);
      return;
    }
  }
  var pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels);
  if (!pixelData) {
    GL.recordError(1280);
    return;
  }
  GLctx.readPixels(x, y, width, height, format, type, pixelData);
}

var _emscripten_glReleaseShaderCompiler = () => {};

var _emscripten_glRenderbufferStorage = (x0, x1, x2, x3) => GLctx.renderbufferStorage(x0, x1, x2, x3);

var _emscripten_glRenderbufferStorageMultisample = (x0, x1, x2, x3, x4) => GLctx.renderbufferStorageMultisample(x0, x1, x2, x3, x4);

var _emscripten_glResumeTransformFeedback = () => GLctx.resumeTransformFeedback();

var _emscripten_glSampleCoverage = (value, invert) => {
  GLctx.sampleCoverage(value, !!invert);
};

var _emscripten_glSamplerParameterf = (sampler, pname, param) => {
  GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
};

function _emscripten_glSamplerParameterfv(sampler, pname, params) {
  params >>>= 0;
  var param = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((params) >>> 2) >>> 0, ___asan_loadN)];
  GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
}

var _emscripten_glSamplerParameteri = (sampler, pname, param) => {
  GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
};

function _emscripten_glSamplerParameteriv(sampler, pname, params) {
  params >>>= 0;
  var param = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_loadN)];
  GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
}

var _emscripten_glScissor = (x0, x1, x2, x3) => GLctx.scissor(x0, x1, x2, x3);

function _emscripten_glShaderBinary(count, shaders, binaryformat, binary, length) {
  shaders >>>= 0;
  binary >>>= 0;
  GL.recordError(1280);
}

function _emscripten_glShaderSource(shader, count, string, length) {
  string >>>= 0;
  length >>>= 0;
  var source = GL.getSource(shader, count, string, length);
  GLctx.shaderSource(GL.shaders[shader], source);
}

var _emscripten_glStencilFunc = (x0, x1, x2) => GLctx.stencilFunc(x0, x1, x2);

var _emscripten_glStencilFuncSeparate = (x0, x1, x2, x3) => GLctx.stencilFuncSeparate(x0, x1, x2, x3);

var _emscripten_glStencilMask = x0 => GLctx.stencilMask(x0);

var _emscripten_glStencilMaskSeparate = (x0, x1) => GLctx.stencilMaskSeparate(x0, x1);

var _emscripten_glStencilOp = (x0, x1, x2) => GLctx.stencilOp(x0, x1, x2);

var _emscripten_glStencilOpSeparate = (x0, x1, x2, x3) => GLctx.stencilOpSeparate(x0, x1, x2, x3);

function _emscripten_glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels) {
  pixels >>>= 0;
  if (true) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
      return;
    }
  }
  var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels) : null;
  GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixelData);
}

function _emscripten_glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels) {
  pixels >>>= 0;
  if (GLctx.currentPixelUnpackBufferBinding) {
    GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);
  } else if (pixels) {
    var heap = heapObjectForWebGLType(type);
    var pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height * depth, pixels);
    GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixelData);
  } else {
    GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, null);
  }
}

var _emscripten_glTexParameterf = (x0, x1, x2) => GLctx.texParameterf(x0, x1, x2);

function _emscripten_glTexParameterfv(target, pname, params) {
  params >>>= 0;
  var param = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((params) >>> 2) >>> 0, ___asan_loadN)];
  GLctx.texParameterf(target, pname, param);
}

var _emscripten_glTexParameteri = (x0, x1, x2) => GLctx.texParameteri(x0, x1, x2);

function _emscripten_glTexParameteriv(target, pname, params) {
  params >>>= 0;
  var param = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((params) >>> 2) >>> 0, ___asan_loadN)];
  GLctx.texParameteri(target, pname, param);
}

var _emscripten_glTexStorage2D = (x0, x1, x2, x3, x4) => GLctx.texStorage2D(x0, x1, x2, x3, x4);

var _emscripten_glTexStorage3D = (x0, x1, x2, x3, x4, x5) => GLctx.texStorage3D(x0, x1, x2, x3, x4, x5);

function _emscripten_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) {
  pixels >>>= 0;
  if (true) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
      return;
    }
  }
  var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels) : null;
  GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
}

function _emscripten_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) {
  pixels >>>= 0;
  if (GLctx.currentPixelUnpackBufferBinding) {
    GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
  } else if (pixels) {
    var heap = heapObjectForWebGLType(type);
    var pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height * depth, pixels);
    GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixelData);
  } else {
    GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, null);
  }
}

function _emscripten_glTransformFeedbackVaryings(program, count, varyings, bufferMode) {
  varyings >>>= 0;
  program = GL.programs[program];
  var vars = [];
  for (var i = 0; i < count; i++) vars.push(UTF8ToString((growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
  HEAPU32), (((varyings) + (i * 4)) >>> 2) >>> 0, ___asan_loadN)]));
  GLctx.transformFeedbackVaryings(program, vars, bufferMode);
}

var webglGetUniformLocation = location => webglGetProgramUniformLocation(GLctx.currentProgram, location);

var _emscripten_glUniform1f = (location, v0) => {
  GLctx.uniform1f(webglGetUniformLocation(location), v0);
};

var miniTempWebGLFloatBuffers = [];

function _emscripten_glUniform1fv(location, count, value) {
  value >>>= 0;
  if (count <= 288) {
    // avoid allocation when uploading few enough uniforms
    var view = miniTempWebGLFloatBuffers[count];
    for (var i = 0; i < count; ++i) {
      view[i] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAPF32).subarray(((value) >>> 2) >>> 0, ((value + count * 4) >>> 2) >>> 0);
  }
  GLctx.uniform1fv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform1i = (location, v0) => {
  GLctx.uniform1i(webglGetUniformLocation(location), v0);
};

var miniTempWebGLIntBuffers = [];

function _emscripten_glUniform1iv(location, count, value) {
  value >>>= 0;
  if (count <= 288) {
    // avoid allocation when uploading few enough uniforms
    var view = miniTempWebGLIntBuffers[count];
    for (var i = 0; i < count; ++i) {
      view[i] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAP32).subarray(((value) >>> 2) >>> 0, ((value + count * 4) >>> 2) >>> 0);
  }
  GLctx.uniform1iv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform1ui = (location, v0) => {
  GLctx.uniform1ui(webglGetUniformLocation(location), v0);
};

function _emscripten_glUniform1uiv(location, count, value) {
  value >>>= 0;
  count && GLctx.uniform1uiv(webglGetUniformLocation(location), (growMemViews(), HEAPU32), ((value) >>> 2), count);
}

var _emscripten_glUniform2f = (location, v0, v1) => {
  GLctx.uniform2f(webglGetUniformLocation(location), v0, v1);
};

function _emscripten_glUniform2fv(location, count, value) {
  value >>>= 0;
  if (count <= 144) {
    // avoid allocation when uploading few enough uniforms
    count *= 2;
    var view = miniTempWebGLFloatBuffers[count];
    for (var i = 0; i < count; i += 2) {
      view[i] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 1] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 4)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAPF32).subarray(((value) >>> 2) >>> 0, ((value + count * 8) >>> 2) >>> 0);
  }
  GLctx.uniform2fv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform2i = (location, v0, v1) => {
  GLctx.uniform2i(webglGetUniformLocation(location), v0, v1);
};

function _emscripten_glUniform2iv(location, count, value) {
  value >>>= 0;
  if (count <= 144) {
    // avoid allocation when uploading few enough uniforms
    count *= 2;
    var view = miniTempWebGLIntBuffers[count];
    for (var i = 0; i < count; i += 2) {
      view[i] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 1] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i + 4)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAP32).subarray(((value) >>> 2) >>> 0, ((value + count * 8) >>> 2) >>> 0);
  }
  GLctx.uniform2iv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform2ui = (location, v0, v1) => {
  GLctx.uniform2ui(webglGetUniformLocation(location), v0, v1);
};

function _emscripten_glUniform2uiv(location, count, value) {
  value >>>= 0;
  count && GLctx.uniform2uiv(webglGetUniformLocation(location), (growMemViews(), HEAPU32), ((value) >>> 2), count * 2);
}

var _emscripten_glUniform3f = (location, v0, v1, v2) => {
  GLctx.uniform3f(webglGetUniformLocation(location), v0, v1, v2);
};

function _emscripten_glUniform3fv(location, count, value) {
  value >>>= 0;
  if (count <= 96) {
    // avoid allocation when uploading few enough uniforms
    count *= 3;
    var view = miniTempWebGLFloatBuffers[count];
    for (var i = 0; i < count; i += 3) {
      view[i] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 1] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 4)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 2] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 8)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAPF32).subarray(((value) >>> 2) >>> 0, ((value + count * 12) >>> 2) >>> 0);
  }
  GLctx.uniform3fv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform3i = (location, v0, v1, v2) => {
  GLctx.uniform3i(webglGetUniformLocation(location), v0, v1, v2);
};

function _emscripten_glUniform3iv(location, count, value) {
  value >>>= 0;
  if (count <= 96) {
    // avoid allocation when uploading few enough uniforms
    count *= 3;
    var view = miniTempWebGLIntBuffers[count];
    for (var i = 0; i < count; i += 3) {
      view[i] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 1] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i + 4)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 2] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i + 8)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAP32).subarray(((value) >>> 2) >>> 0, ((value + count * 12) >>> 2) >>> 0);
  }
  GLctx.uniform3iv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform3ui = (location, v0, v1, v2) => {
  GLctx.uniform3ui(webglGetUniformLocation(location), v0, v1, v2);
};

function _emscripten_glUniform3uiv(location, count, value) {
  value >>>= 0;
  count && GLctx.uniform3uiv(webglGetUniformLocation(location), (growMemViews(), HEAPU32), ((value) >>> 2), count * 3);
}

var _emscripten_glUniform4f = (location, v0, v1, v2, v3) => {
  GLctx.uniform4f(webglGetUniformLocation(location), v0, v1, v2, v3);
};

function _emscripten_glUniform4fv(location, count, value) {
  value >>>= 0;
  if (count <= 72) {
    // avoid allocation when uploading few enough uniforms
    var view = miniTempWebGLFloatBuffers[4 * count];
    // hoist the heap out of the loop for size and for pthreads+growth.
    var heap = (growMemViews(), HEAPF32);
    value = ((value) >>> 2);
    count *= 4;
    for (var i = 0; i < count; i += 4) {
      var dst = value + i;
      view[i] = heap[dst >>> 0];
      view[i + 1] = heap[dst + 1 >>> 0];
      view[i + 2] = heap[dst + 2 >>> 0];
      view[i + 3] = heap[dst + 3 >>> 0];
    }
  } else {
    var view = (growMemViews(), HEAPF32).subarray(((value) >>> 2) >>> 0, ((value + count * 16) >>> 2) >>> 0);
  }
  GLctx.uniform4fv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform4i = (location, v0, v1, v2, v3) => {
  GLctx.uniform4i(webglGetUniformLocation(location), v0, v1, v2, v3);
};

function _emscripten_glUniform4iv(location, count, value) {
  value >>>= 0;
  if (count <= 72) {
    // avoid allocation when uploading few enough uniforms
    count *= 4;
    var view = miniTempWebGLIntBuffers[count];
    for (var i = 0; i < count; i += 4) {
      view[i] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 1] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i + 4)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 2] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i + 8)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 3] = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((value) + (4 * i + 12)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAP32).subarray(((value) >>> 2) >>> 0, ((value + count * 16) >>> 2) >>> 0);
  }
  GLctx.uniform4iv(webglGetUniformLocation(location), view);
}

var _emscripten_glUniform4ui = (location, v0, v1, v2, v3) => {
  GLctx.uniform4ui(webglGetUniformLocation(location), v0, v1, v2, v3);
};

function _emscripten_glUniform4uiv(location, count, value) {
  value >>>= 0;
  count && GLctx.uniform4uiv(webglGetUniformLocation(location), (growMemViews(), HEAPU32), ((value) >>> 2), count * 4);
}

var _emscripten_glUniformBlockBinding = (program, uniformBlockIndex, uniformBlockBinding) => {
  program = GL.programs[program];
  GLctx.uniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
};

function _emscripten_glUniformMatrix2fv(location, count, transpose, value) {
  value >>>= 0;
  if (count <= 72) {
    // avoid allocation when uploading few enough uniforms
    count *= 4;
    var view = miniTempWebGLFloatBuffers[count];
    for (var i = 0; i < count; i += 4) {
      view[i] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 1] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 4)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 2] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 8)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 3] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 12)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAPF32).subarray(((value) >>> 2) >>> 0, ((value + count * 16) >>> 2) >>> 0);
  }
  GLctx.uniformMatrix2fv(webglGetUniformLocation(location), !!transpose, view);
}

function _emscripten_glUniformMatrix2x3fv(location, count, transpose, value) {
  value >>>= 0;
  count && GLctx.uniformMatrix2x3fv(webglGetUniformLocation(location), !!transpose, (growMemViews(), 
  HEAPF32), ((value) >>> 2), count * 6);
}

function _emscripten_glUniformMatrix2x4fv(location, count, transpose, value) {
  value >>>= 0;
  count && GLctx.uniformMatrix2x4fv(webglGetUniformLocation(location), !!transpose, (growMemViews(), 
  HEAPF32), ((value) >>> 2), count * 8);
}

function _emscripten_glUniformMatrix3fv(location, count, transpose, value) {
  value >>>= 0;
  if (count <= 32) {
    // avoid allocation when uploading few enough uniforms
    count *= 9;
    var view = miniTempWebGLFloatBuffers[count];
    for (var i = 0; i < count; i += 9) {
      view[i] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 1] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 4)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 2] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 8)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 3] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 12)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 4] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 16)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 5] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 20)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 6] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 24)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 7] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 28)) >>> 2) >>> 0, ___asan_loadN)];
      view[i + 8] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), (((value) + (4 * i + 32)) >>> 2) >>> 0, ___asan_loadN)];
    }
  } else {
    var view = (growMemViews(), HEAPF32).subarray(((value) >>> 2) >>> 0, ((value + count * 36) >>> 2) >>> 0);
  }
  GLctx.uniformMatrix3fv(webglGetUniformLocation(location), !!transpose, view);
}

function _emscripten_glUniformMatrix3x2fv(location, count, transpose, value) {
  value >>>= 0;
  count && GLctx.uniformMatrix3x2fv(webglGetUniformLocation(location), !!transpose, (growMemViews(), 
  HEAPF32), ((value) >>> 2), count * 6);
}

function _emscripten_glUniformMatrix3x4fv(location, count, transpose, value) {
  value >>>= 0;
  count && GLctx.uniformMatrix3x4fv(webglGetUniformLocation(location), !!transpose, (growMemViews(), 
  HEAPF32), ((value) >>> 2), count * 12);
}

function _emscripten_glUniformMatrix4fv(location, count, transpose, value) {
  value >>>= 0;
  if (count <= 18) {
    // avoid allocation when uploading few enough uniforms
    var view = miniTempWebGLFloatBuffers[16 * count];
    // hoist the heap out of the loop for size and for pthreads+growth.
    var heap = (growMemViews(), HEAPF32);
    value = ((value) >>> 2);
    count *= 16;
    for (var i = 0; i < count; i += 16) {
      var dst = value + i;
      view[i] = heap[dst >>> 0];
      view[i + 1] = heap[dst + 1 >>> 0];
      view[i + 2] = heap[dst + 2 >>> 0];
      view[i + 3] = heap[dst + 3 >>> 0];
      view[i + 4] = heap[dst + 4 >>> 0];
      view[i + 5] = heap[dst + 5 >>> 0];
      view[i + 6] = heap[dst + 6 >>> 0];
      view[i + 7] = heap[dst + 7 >>> 0];
      view[i + 8] = heap[dst + 8 >>> 0];
      view[i + 9] = heap[dst + 9 >>> 0];
      view[i + 10] = heap[dst + 10 >>> 0];
      view[i + 11] = heap[dst + 11 >>> 0];
      view[i + 12] = heap[dst + 12 >>> 0];
      view[i + 13] = heap[dst + 13 >>> 0];
      view[i + 14] = heap[dst + 14 >>> 0];
      view[i + 15] = heap[dst + 15 >>> 0];
    }
  } else {
    var view = (growMemViews(), HEAPF32).subarray(((value) >>> 2) >>> 0, ((value + count * 64) >>> 2) >>> 0);
  }
  GLctx.uniformMatrix4fv(webglGetUniformLocation(location), !!transpose, view);
}

function _emscripten_glUniformMatrix4x2fv(location, count, transpose, value) {
  value >>>= 0;
  count && GLctx.uniformMatrix4x2fv(webglGetUniformLocation(location), !!transpose, (growMemViews(), 
  HEAPF32), ((value) >>> 2), count * 8);
}

function _emscripten_glUniformMatrix4x3fv(location, count, transpose, value) {
  value >>>= 0;
  count && GLctx.uniformMatrix4x3fv(webglGetUniformLocation(location), !!transpose, (growMemViews(), 
  HEAPF32), ((value) >>> 2), count * 12);
}

var _emscripten_glUnmapBuffer = target => {
  if (!emscriptenWebGLValidateMapBufferTarget(target)) {
    GL.recordError(1280);
    err("GL_INVALID_ENUM in glUnmapBuffer");
    return 0;
  }
  var buffer = emscriptenWebGLGetBufferBinding(target);
  var mapping = GL.mappedBuffers[buffer];
  if (!mapping || !mapping.mem) {
    GL.recordError(1282);
    err("buffer was never mapped in glUnmapBuffer");
    return 0;
  }
  if (!(mapping.access & 16)) {
    /* GL_MAP_FLUSH_EXPLICIT_BIT */ webglBufferSubData(target, mapping.offset, mapping.length, mapping.mem);
  }
  _free(mapping.mem);
  mapping.mem = 0;
  return 1;
};

var _emscripten_glUseProgram = program => {
  program = GL.programs[program];
  GLctx.useProgram(program);
  // Record the currently active program so that we can access the uniform
  // mapping table of that program.
  GLctx.currentProgram = program;
};

var _emscripten_glValidateProgram = program => {
  GLctx.validateProgram(GL.programs[program]);
};

var _emscripten_glVertexAttrib1f = (x0, x1) => GLctx.vertexAttrib1f(x0, x1);

function _emscripten_glVertexAttrib1fv(index, v) {
  v >>>= 0;
  GLctx.vertexAttrib1f(index, (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v >>> 2, ___asan_loadN)]);
}

var _emscripten_glVertexAttrib2f = (x0, x1, x2) => GLctx.vertexAttrib2f(x0, x1, x2);

function _emscripten_glVertexAttrib2fv(index, v) {
  v >>>= 0;
  GLctx.vertexAttrib2f(index, (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v >>> 2, ___asan_loadN)], (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v + 4 >>> 2, ___asan_loadN)]);
}

var _emscripten_glVertexAttrib3f = (x0, x1, x2, x3) => GLctx.vertexAttrib3f(x0, x1, x2, x3);

function _emscripten_glVertexAttrib3fv(index, v) {
  v >>>= 0;
  GLctx.vertexAttrib3f(index, (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v >>> 2, ___asan_loadN)], (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v + 4 >>> 2, ___asan_loadN)], (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v + 8 >>> 2, ___asan_loadN)]);
}

var _emscripten_glVertexAttrib4f = (x0, x1, x2, x3, x4) => GLctx.vertexAttrib4f(x0, x1, x2, x3, x4);

function _emscripten_glVertexAttrib4fv(index, v) {
  v >>>= 0;
  GLctx.vertexAttrib4f(index, (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v >>> 2, ___asan_loadN)], (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v + 4 >>> 2, ___asan_loadN)], (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v + 8 >>> 2, ___asan_loadN)], (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
  HEAPF32), v + 12 >>> 2, ___asan_loadN)]);
}

var _emscripten_glVertexAttribDivisor = (index, divisor) => {
  GLctx.vertexAttribDivisor(index, divisor);
};

var _emscripten_glVertexAttribDivisorANGLE = _emscripten_glVertexAttribDivisor;

var _emscripten_glVertexAttribDivisorARB = _emscripten_glVertexAttribDivisor;

var _emscripten_glVertexAttribDivisorEXT = _emscripten_glVertexAttribDivisor;

var _emscripten_glVertexAttribDivisorNV = _emscripten_glVertexAttribDivisor;

var _emscripten_glVertexAttribI4i = (x0, x1, x2, x3, x4) => GLctx.vertexAttribI4i(x0, x1, x2, x3, x4);

function _emscripten_glVertexAttribI4iv(index, v) {
  v >>>= 0;
  GLctx.vertexAttribI4i(index, (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
  HEAP32), v >>> 2, ___asan_loadN)], (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
  HEAP32), v + 4 >>> 2, ___asan_loadN)], (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
  HEAP32), v + 8 >>> 2, ___asan_loadN)], (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
  HEAP32), v + 12 >>> 2, ___asan_loadN)]);
}

var _emscripten_glVertexAttribI4ui = (x0, x1, x2, x3, x4) => GLctx.vertexAttribI4ui(x0, x1, x2, x3, x4);

function _emscripten_glVertexAttribI4uiv(index, v) {
  v >>>= 0;
  GLctx.vertexAttribI4ui(index, (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
  HEAPU32), v >>> 2, ___asan_loadN)], (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
  HEAPU32), v + 4 >>> 2, ___asan_loadN)], (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
  HEAPU32), v + 8 >>> 2, ___asan_loadN)], (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
  HEAPU32), v + 12 >>> 2, ___asan_loadN)]);
}

function _emscripten_glVertexAttribIPointer(index, size, type, stride, ptr) {
  ptr >>>= 0;
  var cb = GL.currentContext.clientBuffers[index];
  if (!GLctx.currentArrayBufferBinding) {
    cb.size = size;
    cb.type = type;
    cb.normalized = false;
    cb.stride = stride;
    cb.ptr = ptr;
    cb.clientside = true;
    cb.vertexAttribPointerAdaptor = /** @this {WebGLRenderingContext} */ function(index, size, type, normalized, stride, ptr) {
      this.vertexAttribIPointer(index, size, type, stride, ptr);
    };
    return;
  }
  cb.clientside = false;
  GLctx.vertexAttribIPointer(index, size, type, stride, ptr);
}

function _emscripten_glVertexAttribPointer(index, size, type, normalized, stride, ptr) {
  ptr >>>= 0;
  var cb = GL.currentContext.clientBuffers[index];
  if (!GLctx.currentArrayBufferBinding) {
    cb.size = size;
    cb.type = type;
    cb.normalized = normalized;
    cb.stride = stride;
    cb.ptr = ptr;
    cb.clientside = true;
    cb.vertexAttribPointerAdaptor = /** @this {WebGLRenderingContext} */ function(index, size, type, normalized, stride, ptr) {
      this.vertexAttribPointer(index, size, type, normalized, stride, ptr);
    };
    return;
  }
  cb.clientside = false;
  GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
}

var _emscripten_glViewport = (x0, x1, x2, x3) => GLctx.viewport(x0, x1, x2, x3);

function _emscripten_glWaitSync(sync, flags, timeout) {
  sync >>>= 0;
  // See WebGL2 vs GLES3 difference on GL_TIMEOUT_IGNORED above (https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15)
  timeout = Number(timeout);
  GLctx.waitSync(GL.syncs[sync], flags, timeout);
}

var _emscripten_has_asyncify = () => 1;

var _emscripten_num_logical_cores = () => ENVIRONMENT_IS_NODE ? require("node:os").cpus().length : navigator["hardwareConcurrency"];

var UNWIND_CACHE = {};

var convertPCtoSourceLocation = pc => {
  if (UNWIND_CACHE.last_get_source_pc == pc) return UNWIND_CACHE.last_source;
  var match;
  var source;
  if (!source) {
    var frame = UNWIND_CACHE[pc];
    if (!frame) return null;
    // Example: at callMain (a.out.js:6335:22)
    if (match = /\((.*):(\d+):(\d+)\)$/.exec(frame)) {
      source = {
        file: match[1],
        line: match[2],
        column: match[3]
      };
    } else if (match = /@(.*):(\d+):(\d+)/.exec(frame)) {
      source = {
        file: match[1],
        line: match[2],
        column: match[3]
      };
    }
  }
  UNWIND_CACHE.last_get_source_pc = pc;
  UNWIND_CACHE.last_source = source;
  return source;
};

function _emscripten_pc_get_column(pc) {
  pc >>>= 0;
  var result = convertPCtoSourceLocation(pc);
  return result ? result.column || 0 : 0;
}

var _emscripten_pc_get_file = pc => noLeakCheck(() => {
  pc >>>= 0;
  var result = convertPCtoSourceLocation(pc);
  if (!result) return 0;
  _free(_emscripten_pc_get_file.ret ?? 0);
  _emscripten_pc_get_file.ret = stringToNewUTF8(result.file);
  return _emscripten_pc_get_file.ret;
});

/** @returns {number} */ var convertFrameToPC = frame => {
  var match;
  if (match = /\bwasm-function\[\d+\]:(0x[0-9a-f]+)/.exec(frame)) {
    // Wasm engines give the binary offset directly, so we use that as return address
    return +match[1];
  } else if (match = /:(\d+):\d+(?:\)|$)/.exec(frame)) {
    // If we are in js, we can use the js line number as the "return address".
    // This should work for wasm2js.  We tag the high bit to distinguish this
    // from wasm addresses.
    return 2147483648 | +match[1];
  }
  // return 0 if we can't find any
  return 0;
};

var saveInUnwindCache = callstack => {
  for (var line of callstack) {
    var pc = convertFrameToPC(line);
    if (pc) {
      UNWIND_CACHE[pc] = line;
    }
  }
};

function _emscripten_stack_snapshot() {
  var callstack = jsStackTrace().split("\n");
  if (callstack[0] == "Error") {
    callstack.shift();
  }
  saveInUnwindCache(callstack);
  // Caches the stack snapshot so that emscripten_stack_unwind_buffer() can
  // unwind from this spot.
  UNWIND_CACHE.last_addr = convertFrameToPC(callstack[3]);
  UNWIND_CACHE.last_stack = callstack;
  return UNWIND_CACHE.last_addr;
}

var _emscripten_pc_get_function = pc => noLeakCheck(() => {
  pc >>>= 0;
  var frame = UNWIND_CACHE[pc];
  if (!frame) return 0;
  var name;
  var match;
  // First try to match foo.wasm.sym files explcitly. e.g.
  //   at test_return_address.wasm.main (wasm://wasm/test_return_address.wasm-0012cc2a:wasm-function[26]:0x9f3
  // Then match JS symbols which don't include that module name:
  //   at invokeEntryPoint (.../test_return_address.js:1500:42)
  // Finally match firefox format:
  //   Object._main@http://server.com:4324:12'
  if (match = /^\s+at .*\.wasm\.(.*) \(.*\)$/.exec(frame)) {
    name = match[1];
  } else if (match = /^\s+at (.*) \(.*\)$/.exec(frame)) {
    name = match[1];
  } else if (match = /^(.+?)@/.exec(frame)) {
    name = match[1];
  } else {
    return 0;
  }
  _free(_emscripten_pc_get_function.ret ?? 0);
  _emscripten_pc_get_function.ret = stringToNewUTF8(name);
  return _emscripten_pc_get_function.ret;
});

function _emscripten_pc_get_line(pc) {
  pc >>>= 0;
  var result = convertPCtoSourceLocation(pc);
  return result ? result.line : 0;
}

var doRequestFullscreen = (target, strategy) => {
  if (!JSEvents.fullscreenEnabled()) return -1;
  target = findEventTarget(target);
  if (!target) return -4;
  if (!target.requestFullscreen && !target.webkitRequestFullscreen) {
    return -3;
  }
  // Queue this function call if we're not currently in an event handler and
  // the user saw it appropriate to do so.
  if (!JSEvents.canPerformEventHandlerRequests()) {
    if (strategy.deferUntilInEventHandler) {
      JSEvents.deferCall(JSEvents_requestFullscreen, 1, [ target, strategy ]);
      return 1;
    }
    return -2;
  }
  return JSEvents_requestFullscreen(target, strategy);
};

function _emscripten_request_fullscreen_strategy(target, deferUntilInEventHandler, fullscreenStrategy) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(48, 0, 1, target, deferUntilInEventHandler, fullscreenStrategy);
  target >>>= 0;
  fullscreenStrategy >>>= 0;
  var strategy = {
    scaleMode: (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((fullscreenStrategy) >>> 2) >>> 0, ___asan_loadN)],
    canvasResolutionScaleMode: (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
    HEAP32), (((fullscreenStrategy) + (4)) >>> 2) >>> 0, ___asan_loadN)],
    filteringMode: (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((fullscreenStrategy) + (8)) >>> 2) >>> 0, ___asan_loadN)],
    deferUntilInEventHandler,
    canvasResizedCallbackTargetThread: (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
    HEAP32), (((fullscreenStrategy) + (20)) >>> 2) >>> 0, ___asan_loadN)],
    canvasResizedCallback: (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
    HEAP32), (((fullscreenStrategy) + (12)) >>> 2) >>> 0, ___asan_loadN)],
    canvasResizedCallbackUserData: (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), 
    HEAP32), (((fullscreenStrategy) + (16)) >>> 2) >>> 0, ___asan_loadN)]
  };
  return doRequestFullscreen(target, strategy);
}

function _emscripten_request_pointerlock(target, deferUntilInEventHandler) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(49, 0, 1, target, deferUntilInEventHandler);
  target >>>= 0;
  target = findEventTarget(target);
  if (!target) return -4;
  if (!target.requestPointerLock) {
    return -1;
  }
  // Queue this function call if we're not currently in an event handler and
  // the user saw it appropriate to do so.
  if (!JSEvents.canPerformEventHandlerRequests()) {
    if (deferUntilInEventHandler) {
      JSEvents.deferCall(requestPointerLock, 2, [ target ]);
      return 1;
    }
    return -2;
  }
  return requestPointerLock(target);
}

var growMemory = size => {
  var oldHeapSize = wasmMemory.buffer.byteLength;
  var pages = ((size - oldHeapSize + 65535) / 65536) | 0;
  try {
    // round size grow request up to wasm page size (fixed 64KB per spec)
    wasmMemory.grow(pages);
    // .grow() takes a delta compared to the previous size
    updateMemoryViews();
    return 1;
  } catch (e) {}
};

function _emscripten_resize_heap(requestedSize) {
  requestedSize >>>= 0;
  var oldSize = (growMemViews(), HEAPU8).length;
  // With multithreaded builds, races can happen (another thread might increase the size
  // in between), so return a failure, and let the caller retry.
  if (requestedSize <= oldSize) {
    return false;
  }
  // Memory resize rules:
  // 1.  Always increase heap size to at least the requested size, rounded up
  //     to next page multiple.
  // 2a. If MEMORY_GROWTH_LINEAR_STEP == -1, excessively resize the heap
  //     geometrically: increase the heap size according to
  //     MEMORY_GROWTH_GEOMETRIC_STEP factor (default +20%), At most
  //     overreserve by MEMORY_GROWTH_GEOMETRIC_CAP bytes (default 96MB).
  // 2b. If MEMORY_GROWTH_LINEAR_STEP != -1, excessively resize the heap
  //     linearly: increase the heap size by at least
  //     MEMORY_GROWTH_LINEAR_STEP bytes.
  // 3.  Max size for the heap is capped at 2048MB-WASM_PAGE_SIZE, or by
  //     MAXIMUM_MEMORY, or by ASAN limit, depending on which is smallest
  // 4.  If we were unable to allocate as much memory, it may be due to
  //     over-eager decision to excessively reserve due to (3) above.
  //     Hence if an allocation fails, cut down on the amount of excess
  //     growth, in an attempt to succeed to perform a smaller allocation.
  // A limit is set for how much we can grow. We should not exceed that
  // (the wasm binary specifies it, so if we tried, we'd fail anyhow).
  var maxHeapSize = getHeapMax();
  if (requestedSize > maxHeapSize) {
    return false;
  }
  // Loop through potential heap size increases. If we attempt a too eager
  // reservation that fails, cut down on the attempted size and reserve a
  // smaller bump instead. (max 3 times, chosen somewhat arbitrarily)
  for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
    var overGrownHeapSize = oldSize * (1 + .2 / cutDown);
    // ensure geometric growth
    // but limit overreserving (default to capping at +96MB overgrowth at most)
    overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296);
    var newSize = Math.min(maxHeapSize, alignMemory(Math.max(requestedSize, overGrownHeapSize), 65536));
    var replacement = growMemory(newSize);
    if (replacement) {
      return true;
    }
  }
  return false;
}

function _emscripten_return_address(level) {
  var callstack = jsStackTrace().split("\n");
  if (callstack[0] == "Error") {
    callstack.shift();
  }
  // skip this function and the caller to get caller's return address
  var caller = callstack[level + 3];
  return convertFrameToPC(caller);
}

/** @suppress {checkTypes} */ function _emscripten_sample_gamepad_data() {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(50, 0, 1);
  try {
    if (navigator.getGamepads) return (JSEvents.lastGamepadState = navigator.getGamepads()) ? 0 : -1;
  } catch (e) {
    navigator.getGamepads = null;
  }
  return -1;
}

var registerBeforeUnloadEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) => {
  var beforeUnloadEventHandlerFunc = e => {
    // Note: This is always called on the main browser thread, since it needs synchronously return a value!
    var confirmationMessage = (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, 0, userData);
    if (confirmationMessage) {
      confirmationMessage = UTF8ToString(confirmationMessage);
    }
    if (confirmationMessage) {
      e.preventDefault();
      e.returnValue = confirmationMessage;
      return confirmationMessage;
    }
  };
  var eventHandler = {
    target: findEventTarget(target),
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: beforeUnloadEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
};

function _emscripten_set_beforeunload_callback_on_thread(userData, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(51, 0, 1, userData, callbackfunc, targetThread);
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  if (typeof onbeforeunload == "undefined") return -1;
  // beforeunload callback can only be registered on the main browser thread, because the page will go away immediately after returning from the handler,
  // and there is no time to start proxying it anywhere.
  if (targetThread !== 1) return -5;
  return registerBeforeUnloadEventCallback(2, userData, true, callbackfunc, 28, "beforeunload");
}

var registerFocusEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 256;
  JSEvents.focusEvent ||= _malloc(eventSize);
  var focusEventHandlerFunc = e => {
    var nodeName = JSEvents.getNodeNameForTarget(e.target);
    var id = e.target.id ?? "";
    var focusEvent = JSEvents.focusEvent;
    stringToUTF8(nodeName, focusEvent + 0, 128);
    stringToUTF8(id, focusEvent + 128, 128);
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, focusEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, focusEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target: findEventTarget(target),
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: focusEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_blur_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(52, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerFocusEventCallback(target, userData, useCapture, callbackfunc, 12, "blur", targetThread);
}

function _emscripten_set_element_css_size(target, width, height) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(53, 0, 1, target, width, height);
  target >>>= 0;
  target = findEventTarget(target);
  if (!target) return -4;
  target.style.width = width + "px";
  target.style.height = height + "px";
  return 0;
}

function _emscripten_set_focus_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(54, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerFocusEventCallback(target, userData, useCapture, callbackfunc, 13, "focus", targetThread);
}

var fillFullscreenChangeEventData = eventStruct => {
  var fullscreenElement = getFullscreenElement();
  var isFullscreen = !!fullscreenElement;
  // Assigning a boolean to HEAP32 with expected type coercion.
  /** @suppress{checkTypes} */ (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), 
  HEAP8), eventStruct >>> 0, ___asan_storeN)] = isFullscreen;
  (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), (eventStruct) + (1) >>> 0, ___asan_storeN)] = JSEvents.fullscreenEnabled();
  // If transitioning to fullscreen, report info about the element that is now fullscreen.
  // If transitioning to windowed mode, report info about the element that just was fullscreen.
  var reportedElement = isFullscreen ? fullscreenElement : JSEvents.previousFullscreenElement;
  var nodeName = JSEvents.getNodeNameForTarget(reportedElement);
  var id = reportedElement?.id ?? "";
  stringToUTF8(nodeName, eventStruct + 2, 128);
  stringToUTF8(id, eventStruct + 130, 128);
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (260)) >>> 2) >>> 0, ___asan_storeN)] = reportedElement?.clientWidth ?? 0;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (264)) >>> 2) >>> 0, ___asan_storeN)] = reportedElement?.clientHeight ?? 0;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (268)) >>> 2) >>> 0, ___asan_storeN)] = screen.width;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (272)) >>> 2) >>> 0, ___asan_storeN)] = screen.height;
  if (isFullscreen) {
    JSEvents.previousFullscreenElement = fullscreenElement;
  }
};

var registerFullscreenChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 276;
  JSEvents.fullscreenChangeEvent ||= _malloc(eventSize);
  var fullscreenChangeEventHandlerFunc = e => {
    var fullscreenChangeEvent = JSEvents.fullscreenChangeEvent;
    fillFullscreenChangeEventData(fullscreenChangeEvent);
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, fullscreenChangeEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, fullscreenChangeEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target,
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: fullscreenChangeEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_fullscreenchange_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(55, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  if (!JSEvents.fullscreenEnabled()) return -1;
  target = findEventTarget(target);
  if (!target) return -4;
  // TODO: When this block is removed, also change test/test_html5_remove_event_listener.c test expectation on emscripten_set_fullscreenchange_callback().
  registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "webkitfullscreenchange", targetThread);
  return registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "fullscreenchange", targetThread);
}

var registerGamepadEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 1240;
  JSEvents.gamepadEvent ||= _malloc(eventSize);
  var gamepadEventHandlerFunc = e => {
    var gamepadEvent = JSEvents.gamepadEvent;
    fillGamepadEventData(gamepadEvent, e["gamepad"]);
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, gamepadEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, gamepadEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target: findEventTarget(target),
    allowsDeferredCalls: true,
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: gamepadEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_gamepadconnected_callback_on_thread(userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(56, 0, 1, userData, useCapture, callbackfunc, targetThread);
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  if (_emscripten_sample_gamepad_data()) return -1;
  return registerGamepadEventCallback(2, userData, useCapture, callbackfunc, 26, "gamepadconnected", targetThread);
}

function _emscripten_set_gamepaddisconnected_callback_on_thread(userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(57, 0, 1, userData, useCapture, callbackfunc, targetThread);
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  if (_emscripten_sample_gamepad_data()) return -1;
  return registerGamepadEventCallback(2, userData, useCapture, callbackfunc, 27, "gamepaddisconnected", targetThread);
}

var registerKeyEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 160;
  JSEvents.keyEvent ||= _malloc(eventSize);
  var keyEventHandlerFunc = e => {
    var keyEventData = JSEvents.keyEvent;
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((keyEventData) >>> 3) >>> 0, ___asan_storeN)] = e.timeStamp;
    var idx = ((keyEventData) >>> 2);
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 2 >>> 0, ___asan_storeN)] = e.location;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), keyEventData + 12 >>> 0, ___asan_storeN)] = e.ctrlKey;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), keyEventData + 13 >>> 0, ___asan_storeN)] = e.shiftKey;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), keyEventData + 14 >>> 0, ___asan_storeN)] = e.altKey;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), keyEventData + 15 >>> 0, ___asan_storeN)] = e.metaKey;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), keyEventData + 16 >>> 0, ___asan_storeN)] = e.repeat;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 5 >>> 0, ___asan_storeN)] = e.charCode;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 6 >>> 0, ___asan_storeN)] = e.keyCode;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 7 >>> 0, ___asan_storeN)] = e.which;
    stringToUTF8(e.key ?? "", keyEventData + 32, 32);
    stringToUTF8(e.code ?? "", keyEventData + 64, 32);
    stringToUTF8(e.char ?? "", keyEventData + 96, 32);
    stringToUTF8(e.locale ?? "", keyEventData + 128, 32);
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, keyEventData, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, keyEventData, userData)) e.preventDefault();
  };
  var eventHandler = {
    target: findEventTarget(target),
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: keyEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_keydown_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(58, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerKeyEventCallback(target, userData, useCapture, callbackfunc, 2, "keydown", targetThread);
}

function _emscripten_set_keypress_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(59, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerKeyEventCallback(target, userData, useCapture, callbackfunc, 1, "keypress", targetThread);
}

function _emscripten_set_keyup_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(60, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerKeyEventCallback(target, userData, useCapture, callbackfunc, 3, "keyup", targetThread);
}

var fillMouseEventData = (eventStruct, e, target) => {
  (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((eventStruct) >>> 3) >>> 0, ___asan_storeN)] = e.timeStamp;
  var idx = ((eventStruct) >>> 2);
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 2 >>> 0, ___asan_storeN)] = e.screenX;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 3 >>> 0, ___asan_storeN)] = e.screenY;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 4 >>> 0, ___asan_storeN)] = e.clientX;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 5 >>> 0, ___asan_storeN)] = e.clientY;
  (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), eventStruct + 24 >>> 0, ___asan_storeN)] = e.ctrlKey;
  (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), eventStruct + 25 >>> 0, ___asan_storeN)] = e.shiftKey;
  (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), eventStruct + 26 >>> 0, ___asan_storeN)] = e.altKey;
  (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), eventStruct + 27 >>> 0, ___asan_storeN)] = e.metaKey;
  (growMemViews(), HEAP16)[_asan_js_check_index((growMemViews(), HEAP16), idx * 2 + 14 >>> 0, ___asan_storeN)] = e.button;
  (growMemViews(), HEAP16)[_asan_js_check_index((growMemViews(), HEAP16), idx * 2 + 15 >>> 0, ___asan_storeN)] = e.buttons;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 8 >>> 0, ___asan_storeN)] = e.movementX;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 9 >>> 0, ___asan_storeN)] = e.movementY;
  // Note: rect contains doubles (truncated to placate SAFE_HEAP, which is the same behaviour when writing to HEAP32 anyway)
  var rect = getBoundingClientRect(target);
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 10 >>> 0, ___asan_storeN)] = e.clientX - (rect.left | 0);
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx + 11 >>> 0, ___asan_storeN)] = e.clientY - (rect.top | 0);
};

var registerMouseEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 64;
  JSEvents.mouseEvent ||= _malloc(eventSize);
  target = findEventTarget(target);
  var mouseEventHandlerFunc = e => {
    // TODO: Make this access thread safe, or this could update live while app is reading it.
    fillMouseEventData(JSEvents.mouseEvent, e, target);
    if (targetThread) {
      __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, JSEvents.mouseEvent, eventSize, userData);
    } else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, JSEvents.mouseEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target,
    allowsDeferredCalls: eventTypeString != "mousemove" && eventTypeString != "mouseenter" && eventTypeString != "mouseleave",
    // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: mouseEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_mousedown_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(61, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerMouseEventCallback(target, userData, useCapture, callbackfunc, 5, "mousedown", targetThread);
}

function _emscripten_set_mouseenter_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(62, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerMouseEventCallback(target, userData, useCapture, callbackfunc, 33, "mouseenter", targetThread);
}

function _emscripten_set_mouseleave_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(63, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerMouseEventCallback(target, userData, useCapture, callbackfunc, 34, "mouseleave", targetThread);
}

function _emscripten_set_mousemove_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(64, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerMouseEventCallback(target, userData, useCapture, callbackfunc, 8, "mousemove", targetThread);
}

function _emscripten_set_mouseup_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(65, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerMouseEventCallback(target, userData, useCapture, callbackfunc, 6, "mouseup", targetThread);
}

var fillPointerlockChangeEventData = eventStruct => {
  var pointerLockElement = document.pointerLockElement;
  var isPointerlocked = !!pointerLockElement;
  // Assigning a boolean to HEAP32 with expected type coercion.
  /** @suppress{checkTypes} */ (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), 
  HEAP8), eventStruct >>> 0, ___asan_storeN)] = isPointerlocked;
  var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
  var id = pointerLockElement?.id ?? "";
  stringToUTF8(nodeName, eventStruct + 1, 128);
  stringToUTF8(id, eventStruct + 129, 128);
};

var registerPointerlockChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 257;
  JSEvents.pointerlockChangeEvent ||= _malloc(eventSize);
  var pointerlockChangeEventHandlerFunc = e => {
    var pointerlockChangeEvent = JSEvents.pointerlockChangeEvent;
    fillPointerlockChangeEventData(pointerlockChangeEvent);
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, pointerlockChangeEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, pointerlockChangeEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target,
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: pointerlockChangeEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_pointerlockchange_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(66, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  if (!document.body?.requestPointerLock) {
    return -1;
  }
  target = findEventTarget(target);
  if (!target) return -4;
  return registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "pointerlockchange", targetThread);
}

var registerUiEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 36;
  JSEvents.uiEvent ||= _malloc(eventSize);
  target = findEventTarget(target);
  var uiEventHandlerFunc = e => {
    if (e.target != target) {
      // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
      // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
      // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
      // causing a new scroll, etc..
      return;
    }
    var b = document.body;
    // Take document.body to a variable, Closure compiler does not outline access to it on its own.
    if (!b) {
      // During a page unload 'body' can be null, with "Cannot read property 'clientWidth' of null" being thrown
      return;
    }
    var uiEvent = JSEvents.uiEvent;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((uiEvent) >>> 2) >>> 0, ___asan_storeN)] = 0;
    // always zero for resize and scroll
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (4)) >>> 2) >>> 0, ___asan_storeN)] = b.clientWidth;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (8)) >>> 2) >>> 0, ___asan_storeN)] = b.clientHeight;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (12)) >>> 2) >>> 0, ___asan_storeN)] = innerWidth;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (16)) >>> 2) >>> 0, ___asan_storeN)] = innerHeight;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (20)) >>> 2) >>> 0, ___asan_storeN)] = outerWidth;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (24)) >>> 2) >>> 0, ___asan_storeN)] = outerHeight;
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (28)) >>> 2) >>> 0, ___asan_storeN)] = pageXOffset | 0;
    // scroll offsets are float
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((uiEvent) + (32)) >>> 2) >>> 0, ___asan_storeN)] = pageYOffset | 0;
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, uiEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, uiEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target,
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: uiEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_resize_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(67, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerUiEventCallback(target, userData, useCapture, callbackfunc, 10, "resize", targetThread);
}

var registerTouchEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 1552;
  JSEvents.touchEvent ||= _malloc(eventSize);
  target = findEventTarget(target);
  var touchEventHandlerFunc = e => {
    var t, touches = {}, et = e.touches;
    // To ease marshalling different kinds of touches that browser reports (all touches are listed in e.touches,
    // only changed touches in e.changedTouches, and touches on target at a.targetTouches), mark a boolean in
    // each Touch object so that we can later loop only once over all touches we see to marshall over to Wasm.
    for (let t of et) {
      // Browser might recycle the generated Touch objects between each frame (Firefox on Android), so reset any
      // changed/target states we may have set from previous frame.
      t.isChanged = t.onTarget = 0;
      touches[t.identifier] = t;
    }
    // Mark which touches are part of the changedTouches list.
    for (let t of e.changedTouches) {
      t.isChanged = 1;
      touches[t.identifier] = t;
    }
    // Mark which touches are part of the targetTouches list.
    for (let t of e.targetTouches) {
      touches[t.identifier].onTarget = 1;
    }
    var touchEvent = JSEvents.touchEvent;
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((touchEvent) >>> 3) >>> 0, ___asan_storeN)] = e.timeStamp;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), touchEvent + 12 >>> 0, ___asan_storeN)] = e.ctrlKey;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), touchEvent + 13 >>> 0, ___asan_storeN)] = e.shiftKey;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), touchEvent + 14 >>> 0, ___asan_storeN)] = e.altKey;
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), touchEvent + 15 >>> 0, ___asan_storeN)] = e.metaKey;
    var idx = touchEvent + 16;
    var targetRect = getBoundingClientRect(target);
    var numTouches = 0;
    for (let t of Object.values(touches)) {
      var idx32 = ((idx) >>> 2);
      // Pre-shift the ptr to index to HEAP32 to save code size
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 0 >>> 0, ___asan_storeN)] = t.identifier;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 1 >>> 0, ___asan_storeN)] = t.screenX;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 2 >>> 0, ___asan_storeN)] = t.screenY;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 3 >>> 0, ___asan_storeN)] = t.clientX;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 4 >>> 0, ___asan_storeN)] = t.clientY;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 5 >>> 0, ___asan_storeN)] = t.pageX;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 6 >>> 0, ___asan_storeN)] = t.pageY;
      (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), idx + 28 >>> 0, ___asan_storeN)] = t.isChanged;
      (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), idx + 29 >>> 0, ___asan_storeN)] = t.onTarget;
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 8 >>> 0, ___asan_storeN)] = t.clientX - (targetRect.left | 0);
      (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), idx32 + 9 >>> 0, ___asan_storeN)] = t.clientY - (targetRect.top | 0);
      idx += 48;
      if (++numTouches > 31) {
        break;
      }
    }
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((touchEvent) + (8)) >>> 2) >>> 0, ___asan_storeN)] = numTouches;
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, touchEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, touchEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target,
    allowsDeferredCalls: eventTypeString == "touchstart" || eventTypeString == "touchend",
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: touchEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_touchcancel_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(68, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerTouchEventCallback(target, userData, useCapture, callbackfunc, 25, "touchcancel", targetThread);
}

function _emscripten_set_touchend_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(69, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerTouchEventCallback(target, userData, useCapture, callbackfunc, 23, "touchend", targetThread);
}

function _emscripten_set_touchmove_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(70, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerTouchEventCallback(target, userData, useCapture, callbackfunc, 24, "touchmove", targetThread);
}

function _emscripten_set_touchstart_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(71, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  return registerTouchEventCallback(target, userData, useCapture, callbackfunc, 22, "touchstart", targetThread);
}

var fillVisibilityChangeEventData = eventStruct => {
  var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
  var visibilityState = visibilityStates.indexOf(document.visibilityState);
  // Assigning a boolean to HEAP32 with expected type coercion.
  /** @suppress{checkTypes} */ (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), 
  HEAP8), eventStruct >>> 0, ___asan_storeN)] = document.hidden;
  (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((eventStruct) + (4)) >>> 2) >>> 0, ___asan_storeN)] = visibilityState;
};

var registerVisibilityChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 8;
  JSEvents.visibilityChangeEvent ||= _malloc(eventSize);
  var visibilityChangeEventHandlerFunc = e => {
    var visibilityChangeEvent = JSEvents.visibilityChangeEvent;
    fillVisibilityChangeEventData(visibilityChangeEvent);
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, visibilityChangeEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, visibilityChangeEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target,
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: visibilityChangeEventHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_visibilitychange_callback_on_thread(userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(72, 0, 1, userData, useCapture, callbackfunc, targetThread);
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  if (!specialHTMLTargets[1]) {
    return -4;
  }
  return registerVisibilityChangeEventCallback(specialHTMLTargets[1], userData, useCapture, callbackfunc, 21, "visibilitychange", targetThread);
}

var registerWheelEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => noLeakCheck(() => {
  targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
  var eventSize = 96;
  JSEvents.wheelEvent ||= _malloc(eventSize);
  // The DOM Level 3 events spec event 'wheel'
  var wheelHandlerFunc = e => {
    var wheelEvent = JSEvents.wheelEvent;
    fillMouseEventData(wheelEvent, e, target);
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), (((wheelEvent) + (64)) >>> 3) >>> 0, ___asan_storeN)] = e["deltaX"];
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), (((wheelEvent) + (72)) >>> 3) >>> 0, ___asan_storeN)] = e["deltaY"];
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), (((wheelEvent) + (80)) >>> 3) >>> 0, ___asan_storeN)] = e["deltaZ"];
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((wheelEvent) + (88)) >>> 2) >>> 0, ___asan_storeN)] = e["deltaMode"];
    if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, wheelEvent, eventSize, userData); else if ((((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3)))(eventTypeId, wheelEvent, userData)) e.preventDefault();
  };
  var eventHandler = {
    target,
    allowsDeferredCalls: true,
    eventTypeString,
    eventTypeId,
    userData,
    callbackfunc,
    handlerFunc: wheelHandlerFunc,
    useCapture
  };
  return JSEvents.registerOrRemoveHandler(eventHandler);
});

function _emscripten_set_wheel_callback_on_thread(target, userData, useCapture, callbackfunc, targetThread) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(73, 0, 1, target, userData, useCapture, callbackfunc, targetThread);
  target >>>= 0;
  userData >>>= 0;
  callbackfunc >>>= 0;
  targetThread >>>= 0;
  target = findEventTarget(target);
  if (!target) return -4;
  if (typeof target.onwheel != "undefined") {
    return registerWheelEventCallback(target, userData, useCapture, callbackfunc, 9, "wheel", targetThread);
  } else {
    return -1;
  }
}

function _emscripten_set_window_title(title) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(74, 0, 1, title);
  title >>>= 0;
  return document.title = UTF8ToString(title);
}

var _emscripten_sleep = function(ms) {
  let innerFunc = () => new Promise(resolve => setTimeout(resolve, ms));
  return Asyncify.handleAsync(innerFunc);
};

_emscripten_sleep.isAsync = true;

function _emscripten_stack_unwind_buffer(addr, buffer, count) {
  addr >>>= 0;
  buffer >>>= 0;
  var stack;
  if (UNWIND_CACHE.last_addr == addr) {
    stack = UNWIND_CACHE.last_stack;
  } else {
    stack = jsStackTrace().split("\n");
    if (stack[0] == "Error") {
      stack.shift();
    }
    saveInUnwindCache(stack);
  }
  var offset = 3;
  while (stack[offset] && convertFrameToPC(stack[offset]) != addr) {
    ++offset;
  }
  for (var i = 0; i < count && stack[i + offset]; ++i) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((buffer) + (i * 4)) >>> 2) >>> 0, ___asan_storeN)] = convertFrameToPC(stack[i + offset]);
  }
  return i;
}

var _emscripten_unwind_to_js_event_loop = () => {
  throw "unwind";
};

var ENV = {};

var getEnvStrings = () => {
  if (!getEnvStrings.strings) {
    // Default values.
    var lang = (globalThis.navigator?.language ?? "C").replace("-", "_") + ".UTF-8";
    var env = {
      "USER": "web_user",
      "LOGNAME": "web_user",
      "PATH": "/",
      "PWD": "/",
      "HOME": "/home/web_user",
      "LANG": lang,
      "_": getExecutableName()
    };
    // Apply the user-provided values, if any.
    for (var x in ENV) {
      // x is a key in ENV; if ENV[x] is undefined, that means it was
      // explicitly set to be so. We allow user code to do that to
      // force variables with default values to remain unset.
      if (ENV[x] === undefined) delete env[x]; else env[x] = ENV[x];
    }
    var strings = [];
    for (var x in env) {
      strings.push(`${x}=${env[x]}`);
    }
    getEnvStrings.strings = strings;
  }
  return getEnvStrings.strings;
};

function _environ_get(__environ, environ_buf) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(75, 0, 1, __environ, environ_buf);
  __environ >>>= 0;
  environ_buf >>>= 0;
  var bufSize = 0;
  var envp = 0;
  for (var string of getEnvStrings()) {
    var ptr = environ_buf + bufSize;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((__environ) + (envp)) >>> 2) >>> 0, ___asan_storeN)] = ptr;
    bufSize += stringToUTF8(string, ptr, Infinity) + 1;
    envp += 4;
  }
  return 0;
}

function _environ_sizes_get(penviron_count, penviron_buf_size) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(76, 0, 1, penviron_count, penviron_buf_size);
  penviron_count >>>= 0;
  penviron_buf_size >>>= 0;
  var strings = getEnvStrings();
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((penviron_count) >>> 2) >>> 0, ___asan_storeN)] = strings.length;
  var bufSize = 0;
  for (var string of strings) {
    bufSize += lengthBytesUTF8(string) + 1;
  }
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((penviron_buf_size) >>> 2) >>> 0, ___asan_storeN)] = bufSize;
  return 0;
}

function _fd_close(fd) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(77, 0, 1, fd);
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.close(stream);
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return e.errno;
  }
}

/** @param {number=} offset */ var doReadv = (stream, iov, iovcnt, offset) => {
  var ret = 0;
  for (var i = 0; i < iovcnt; i++) {
    var ptr = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((iov) >>> 2) >>> 0, ___asan_loadN)];
    var len = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((iov) + (4)) >>> 2) >>> 0, ___asan_loadN)];
    iov += 8;
    try {
      var curr = FS.read(stream, (growMemViews(), HEAP8), ptr, len, offset);
    } catch (e) {
      // On a non-blocking stream a subsequent read may would-block after we
      // already gathered data. POSIX readv is a single gather-read: return
      // what we have rather than failing the whole call.
      if (ret > 0 && e instanceof FS.ErrnoError && (e.errno == 6 || e.errno == 6)) {
        break;
      }
      throw e;
    }
    if (curr < 0) return -1;
    ret += curr;
    if (curr < len) break;
    // nothing more to read
    if (typeof offset != "undefined") {
      offset += curr;
    }
  }
  return ret;
};

function _fd_read(fd, iov, iovcnt, pnum) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(78, 0, 1, fd, iov, iovcnt, pnum);
  iov >>>= 0;
  iovcnt >>>= 0;
  pnum >>>= 0;
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    var num = doReadv(stream, iov, iovcnt);
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((pnum) >>> 2) >>> 0, ___asan_storeN)] = num;
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return e.errno;
  }
}

function _fd_seek(fd, offset, whence, newOffset) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(79, 0, 1, fd, offset, whence, newOffset);
  offset = bigintToI53Checked(offset);
  newOffset >>>= 0;
  try {
    if (isNaN(offset)) return 22;
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.llseek(stream, offset, whence);
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), ((newOffset) >>> 3) >>> 0, ___asan_storeN)] = BigInt(stream.position);
    if (stream.getdents && !offset && whence === 0) stream.getdents = null;
    // reset readdir state
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return e.errno;
  }
}

/** @param {number=} offset */ var doWritev = (stream, iov, iovcnt, offset) => {
  // Gather all iovecs into one contiguous buffer and issue a single
  // FS.write, matching POSIX writev's single gather-write semantics (as
  // __syscall_sendmsg already does). Per-iovec writes fragment a stream
  // socket send into multiple segments, breaking stream byte semantics.
  if (iovcnt == 1) {
    // Single iovec: write directly from HEAP8, no gather buffer needed.
    return FS.write(stream, (growMemViews(), HEAP8), (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
    HEAPU32), ((iov) >>> 2) >>> 0, ___asan_loadN)], (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), 
    HEAPU32), (((iov) + (4)) >>> 2) >>> 0, ___asan_loadN)], offset);
  }
  var total = 0;
  for (var i = 0, p = iov; i < iovcnt; i++, p += 8) {
    total += (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((p) + (4)) >>> 2) >>> 0, ___asan_loadN)];
  }
  var view = new Uint8Array(total);
  var voff = 0;
  for (var i = 0; i < iovcnt; i++, iov += 8) {
    var ptr = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((iov) >>> 2) >>> 0, ___asan_loadN)];
    var len = (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((iov) + (4)) >>> 2) >>> 0, ___asan_loadN)];
    view.set((growMemViews(), HEAPU8).subarray(ptr >>> 0, ptr + len >>> 0), voff);
    voff += len;
  }
  return FS.write(stream, view, 0, total, offset);
};

function _fd_write(fd, iov, iovcnt, pnum) {
  if (ENVIRONMENT_IS_PTHREAD) return proxyToMainThread(80, 0, 1, fd, iov, iovcnt, pnum);
  iov >>>= 0;
  iovcnt >>>= 0;
  pnum >>>= 0;
  try {
    var stream = SYSCALLS.getStreamFromFD(fd);
    var num = doWritev(stream, iov, iovcnt);
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((pnum) >>> 2) >>> 0, ___asan_storeN)] = num;
    return 0;
  } catch (e) {
    if (typeof FS == "undefined" || !(e.name === "ErrnoError")) throw e;
    return e.errno;
  }
}

var _glActiveTexture = _emscripten_glActiveTexture;

var _glAttachShader = _emscripten_glAttachShader;

var _glBindAttribLocation = _emscripten_glBindAttribLocation;

var _glBindBuffer = _emscripten_glBindBuffer;

var _glBindFramebuffer = _emscripten_glBindFramebuffer;

var _glBindRenderbuffer = _emscripten_glBindRenderbuffer;

var _glBindTexture = _emscripten_glBindTexture;

var _glBindVertexArray = _emscripten_glBindVertexArray;

var _glBlendFunc = _emscripten_glBlendFunc;

var _glBlitFramebuffer = _emscripten_glBlitFramebuffer;

var _glBufferData = _emscripten_glBufferData;

var _glCheckFramebufferStatus = _emscripten_glCheckFramebufferStatus;

var _glClear = _emscripten_glClear;

var _glClearColor = _emscripten_glClearColor;

var _glClearDepthf = _emscripten_glClearDepthf;

var _glColorMask = _emscripten_glColorMask;

var _glCompileShader = _emscripten_glCompileShader;

var _glCreateProgram = _emscripten_glCreateProgram;

var _glCreateShader = _emscripten_glCreateShader;

var _glCullFace = _emscripten_glCullFace;

var _glDeleteFramebuffers = _emscripten_glDeleteFramebuffers;

var _glDeleteRenderbuffers = _emscripten_glDeleteRenderbuffers;

var _glDeleteShader = _emscripten_glDeleteShader;

var _glDeleteTextures = _emscripten_glDeleteTextures;

var _glDepthFunc = _emscripten_glDepthFunc;

var _glDepthMask = _emscripten_glDepthMask;

var _glDisable = _emscripten_glDisable;

var _glDisableVertexAttribArray = _emscripten_glDisableVertexAttribArray;

var _glDrawArrays = _emscripten_glDrawArrays;

var _glDrawBuffers = _emscripten_glDrawBuffers;

var _glEnable = _emscripten_glEnable;

var _glEnableVertexAttribArray = _emscripten_glEnableVertexAttribArray;

var _glFramebufferRenderbuffer = _emscripten_glFramebufferRenderbuffer;

var _glFramebufferTexture2D = _emscripten_glFramebufferTexture2D;

var _glFrontFace = _emscripten_glFrontFace;

var _glGenBuffers = _emscripten_glGenBuffers;

var _glGenFramebuffers = _emscripten_glGenFramebuffers;

var _glGenRenderbuffers = _emscripten_glGenRenderbuffers;

var _glGenTextures = _emscripten_glGenTextures;

var _glGenVertexArrays = _emscripten_glGenVertexArrays;

var _glGetError = _emscripten_glGetError;

var _glGetIntegerv = _emscripten_glGetIntegerv;

var _glGetProgramInfoLog = _emscripten_glGetProgramInfoLog;

var _glGetProgramiv = _emscripten_glGetProgramiv;

var _glGetShaderInfoLog = _emscripten_glGetShaderInfoLog;

var _glGetShaderiv = _emscripten_glGetShaderiv;

var _glGetString = _emscripten_glGetString;

var _glGetUniformLocation = _emscripten_glGetUniformLocation;

var _glLinkProgram = _emscripten_glLinkProgram;

var _glPixelStorei = _emscripten_glPixelStorei;

var _glPolygonOffset = _emscripten_glPolygonOffset;

var _glReadBuffer = _emscripten_glReadBuffer;

var _glReadPixels = _emscripten_glReadPixels;

var _glRenderbufferStorage = _emscripten_glRenderbufferStorage;

var _glScissor = _emscripten_glScissor;

var _glShaderSource = _emscripten_glShaderSource;

var _glTexImage2D = _emscripten_glTexImage2D;

var _glTexParameteri = _emscripten_glTexParameteri;

var _glTexSubImage2D = _emscripten_glTexSubImage2D;

var _glUniform1f = _emscripten_glUniform1f;

var _glUniform1i = _emscripten_glUniform1i;

var _glUniform1iv = _emscripten_glUniform1iv;

var _glUniform2f = _emscripten_glUniform2f;

var _glUniform3f = _emscripten_glUniform3f;

var _glUniform4f = _emscripten_glUniform4f;

var _glUniformMatrix3fv = _emscripten_glUniformMatrix3fv;

var _glUseProgram = _emscripten_glUseProgram;

var _glVertexAttrib4f = _emscripten_glVertexAttrib4f;

var _glVertexAttribPointer = _emscripten_glVertexAttribPointer;

var _glViewport = _emscripten_glViewport;

var autoResumeAudioContext = ctx => {
  for (var event of [ "keydown", "mousedown", "touchstart" ]) {
    for (var element of [ document, document.getElementById("canvas") ]) {
      element?.addEventListener(event, () => {
        if (ctx.state === "suspended") ctx.resume();
      }, {
        "once": true
      });
    }
  }
};

var wasmTableMirror = [];

var getWasmTableEntry = funcPtr => {
  var func = wasmTableMirror[funcPtr];
  if (!func) {
    /** @suppress {checkTypes} */ wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
  }
  return func;
};

var runAndAbortIfError = func => {
  try {
    return func();
  } catch (e) {
    abort(e);
  }
};

var Asyncify = {
  instrumentWasmImports(imports) {
    var importPattern = /^(invoke_.*|__asyncjs__.*)$/;
    for (let [x, original] of Object.entries(imports)) {
      if (typeof original == "function") {
        let isAsyncifyImport = original.isAsync || importPattern.test(x);
      }
    }
  },
  instrumentFunction(original) {
    var wrapper = (...args) => {
      Asyncify.exportCallStack.push(original);
      try {
        return original(...args);
      } finally {
        if (!ABORT) {
          var top = Asyncify.exportCallStack.pop();
          Asyncify.maybeStopUnwind();
        }
      }
    };
    Asyncify.funcWrappers.set(original, wrapper);
    return wrapper;
  },
  instrumentWasmExports(exports) {
    var ret = {};
    for (let [x, original] of Object.entries(exports)) {
      if (typeof original == "function") {
        var wrapper = Asyncify.instrumentFunction(original);
        ret[x] = wrapper;
      } else {
        ret[x] = original;
      }
    }
    return ret;
  },
  State: {
    Normal: 0,
    Unwinding: 1,
    Rewinding: 2,
    Disabled: 3
  },
  state: 0,
  StackSize: 1048576,
  currData: null,
  handleSleepReturnValue: 0,
  exportCallStack: [],
  callstackFuncToId: new Map,
  callStackIdToFunc: new Map,
  funcWrappers: new Map,
  callStackId: 0,
  asyncPromiseHandlers: null,
  sleepCallbacks: [],
  getCallStackId(func) {
    if (!Asyncify.callstackFuncToId.has(func)) {
      var id = Asyncify.callStackId++;
      Asyncify.callstackFuncToId.set(func, id);
      Asyncify.callStackIdToFunc.set(id, func);
    }
    return Asyncify.callstackFuncToId.get(func);
  },
  maybeStopUnwind() {
    if (Asyncify.currData && Asyncify.state === Asyncify.State.Unwinding && !Asyncify.exportCallStack.length) {
      // We just finished unwinding.
      // Be sure to set the state before calling any other functions to avoid
      // possible infinite recursion here (For example in debug pthread builds
      // the dbg() function itself can call back into WebAssembly to get the
      // current pthread_self() pointer).
      Asyncify.state = Asyncify.State.Normal;
      runtimeKeepalivePush();
      // Keep the runtime alive so that a re-wind can be done later.
      runAndAbortIfError(_asyncify_stop_unwind);
      if (typeof Fibers != "undefined") {
        Fibers.trampoline();
      }
    }
  },
  whenDone() {
    return new Promise((resolve, reject) => {
      Asyncify.asyncPromiseHandlers = {
        resolve,
        reject
      };
    });
  },
  allocateData() {
    // An asyncify data structure has three fields:
    //  0  current stack pos
    //  4  max stack pos
    //  8  id of function at bottom of the call stack (callStackIdToFunc[id] == wasm func)
    // The Asyncify ABI only interprets the first two fields, the rest is for the runtime.
    // We also embed a stack in the same memory region here, right next to the structure.
    // This struct is also defined as asyncify_data_t in emscripten/fiber.h
    var ptr = _malloc(12 + Asyncify.StackSize);
    Asyncify.setDataHeader(ptr, ptr + 12, Asyncify.StackSize);
    Asyncify.setDataRewindFunc(ptr);
    return ptr;
  },
  setDataHeader(ptr, stack, stackSize) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((ptr) >>> 2) >>> 0, ___asan_storeN)] = stack;
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), (((ptr) + (4)) >>> 2) >>> 0, ___asan_storeN)] = stack + stackSize;
  },
  setDataRewindFunc(ptr) {
    var bottomOfCallStack = Asyncify.exportCallStack[0];
    var rewindId = Asyncify.getCallStackId(bottomOfCallStack);
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((ptr) + (8)) >>> 2) >>> 0, ___asan_storeN)] = rewindId;
  },
  getDataRewindFunc(ptr) {
    var id = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), (((ptr) + (8)) >>> 2) >>> 0, ___asan_loadN)];
    var func = Asyncify.callStackIdToFunc.get(id);
    return func;
  },
  doRewind(ptr) {
    var original = Asyncify.getDataRewindFunc(ptr);
    var func = Asyncify.funcWrappers.get(original);
    // Once we have rewound and the stack we no longer need to artificially
    // keep the runtime alive.
    runtimeKeepalivePop();
    return callUserCallback(func);
  },
  handleSleep(startAsync) {
    if (ABORT) return;
    if (Asyncify.state === Asyncify.State.Normal) {
      // Prepare to sleep. Call startAsync, and see what happens:
      // if the code decided to call our callback synchronously,
      // then no async operation was in fact begun, and we don't
      // need to do anything.
      var reachedCallback = false;
      var reachedAfterCallback = false;
      startAsync((handleSleepReturnValue = 0) => {
        if (ABORT) return;
        Asyncify.handleSleepReturnValue = handleSleepReturnValue;
        reachedCallback = true;
        if (!reachedAfterCallback) {
          // We are happening synchronously, so no need for async.
          return;
        }
        Asyncify.state = Asyncify.State.Rewinding;
        runAndAbortIfError(() => _asyncify_start_rewind(Asyncify.currData));
        if (typeof MainLoop != "undefined" && MainLoop.func) {
          MainLoop.resume();
        }
        var asyncWasmReturnValue, isError = false;
        try {
          asyncWasmReturnValue = Asyncify.doRewind(Asyncify.currData);
        } catch (err) {
          asyncWasmReturnValue = err;
          isError = true;
        }
        // Track whether the return value was handled by any promise handlers.
        var handled = false;
        if (!Asyncify.currData) {
          // All asynchronous execution has finished.
          // `asyncWasmReturnValue` now contains the final
          // return value of the exported async WASM function.
          // Note: `asyncWasmReturnValue` is distinct from
          // `Asyncify.handleSleepReturnValue`.
          // `Asyncify.handleSleepReturnValue` contains the return
          // value of the last C function to have executed
          // `Asyncify.handleSleep()`, whereas `asyncWasmReturnValue`
          // contains the return value of the exported WASM function
          // that may have called C functions that
          // call `Asyncify.handleSleep()`.
          var asyncPromiseHandlers = Asyncify.asyncPromiseHandlers;
          if (asyncPromiseHandlers) {
            Asyncify.asyncPromiseHandlers = null;
            (isError ? asyncPromiseHandlers.reject : asyncPromiseHandlers.resolve)(asyncWasmReturnValue);
            handled = true;
          }
        }
        if (isError && !handled) {
          // If there was an error and it was not handled by now, we have no choice but to
          // rethrow that error into the global scope where it can be caught only by
          // `onerror` or `onunhandledpromiserejection`.
          throw asyncWasmReturnValue;
        }
      });
      reachedAfterCallback = true;
      if (!reachedCallback) {
        // A true async operation was begun; start a sleep.
        Asyncify.state = Asyncify.State.Unwinding;
        // TODO: reuse, don't alloc/free every sleep
        Asyncify.currData = Asyncify.allocateData();
        if (typeof MainLoop != "undefined" && MainLoop.func) {
          MainLoop.pause();
        }
        runAndAbortIfError(() => _asyncify_start_unwind(Asyncify.currData));
      }
    } else if (Asyncify.state === Asyncify.State.Rewinding) {
      // Stop a resume.
      Asyncify.state = Asyncify.State.Normal;
      runAndAbortIfError(_asyncify_stop_rewind);
      _free(Asyncify.currData);
      Asyncify.currData = null;
      // Call all sleep callbacks now that the sleep-resume is all done.
      for (var cb of Asyncify.sleepCallbacks) {
        callUserCallback(cb);
      }
    } else {
      abort(`invalid state: ${Asyncify.state}`);
    }
    return Asyncify.handleSleepReturnValue;
  },
  handleAsync: startAsync => Asyncify.handleSleep(async wakeUp => {
    // TODO: add error handling as a second param when handleSleep implements it.
    wakeUp(await startAsync());
  })
};

/**
   * @param {number} ptr
   * @param {number} value
   * @param {string} type
   */ function setValue(ptr, value, type = "i8") {
  if (type.endsWith("*")) type = "*";
  switch (type) {
   case "i1":
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), ptr >>> 0, ___asan_storeN)] = value;
    break;

   case "i8":
    (growMemViews(), HEAP8)[_asan_js_check_index((growMemViews(), HEAP8), ptr >>> 0, ___asan_storeN)] = value;
    break;

   case "i16":
    (growMemViews(), HEAP16)[_asan_js_check_index((growMemViews(), HEAP16), ((ptr) >>> 1) >>> 0, ___asan_storeN)] = value;
    break;

   case "i32":
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), ((ptr) >>> 2) >>> 0, ___asan_storeN)] = value;
    break;

   case "i64":
    (growMemViews(), HEAP64)[_asan_js_check_index((growMemViews(), HEAP64), ((ptr) >>> 3) >>> 0, ___asan_storeN)] = BigInt(value);
    break;

   case "float":
    (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), HEAPF32), ((ptr) >>> 2) >>> 0, ___asan_storeN)] = value;
    break;

   case "double":
    (growMemViews(), HEAPF64)[_asan_js_check_index((growMemViews(), HEAPF64), ((ptr) >>> 3) >>> 0, ___asan_storeN)] = value;
    break;

   case "*":
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((ptr) >>> 2) >>> 0, ___asan_storeN)] = value;
    break;

   default:
    abort(`invalid type for setValue: ${type}`);
  }
}

var FS_createPath = (...args) => FS.createPath(...args);

var FS_unlink = (...args) => FS.unlink(...args);

var FS_createLazyFile = (...args) => FS.createLazyFile(...args);

var FS_createDevice = (...args) => FS.createDevice(...args);

PThread.init();

FS.createPreloadedFile = FS_createPreloadedFile;

FS.preloadFile = FS_preloadFile;

FS.staticInit();

safeSetTimeout.pending = new Map;

safeSetTimeout.nextId = 1;

// Signal GL rendering layer that processing of a new frame is about to
// start. This helps it optimize VBO double-buffering and reduce GPU stalls.
registerPreMainLoop(() => GL.newRenderingFrameStarted());

Module["requestAnimationFrame"] = MainLoop.requestAnimationFrame;

Module["pauseMainLoop"] = MainLoop.pause;

Module["resumeMainLoop"] = MainLoop.resume;

MainLoop.init();

for (let i = 0; i < 32; ++i) tempFixedLengthArray.push(new Array(i));

var miniTempWebGLFloatBuffersStorage = new Float32Array(288);

// Create GL_POOL_TEMP_BUFFERS_SIZE+1 temporary buffers, for uploads of size 0 through GL_POOL_TEMP_BUFFERS_SIZE inclusive
for (/**@suppress{duplicate}*/ var i = 0; i <= 288; ++i) {
  miniTempWebGLFloatBuffers[i] = miniTempWebGLFloatBuffersStorage.subarray(0, i);
}

var miniTempWebGLIntBuffersStorage = new Int32Array(288);

// Create GL_POOL_TEMP_BUFFERS_SIZE+1 temporary buffers, for uploads of size 0 through GL_POOL_TEMP_BUFFERS_SIZE inclusive
for (/**@suppress{duplicate}*/ var i = 0; i <= 288; ++i) {
  miniTempWebGLIntBuffers[i] = miniTempWebGLIntBuffersStorage.subarray(0, i);
}

// End JS library code
// include: postlibrary.js
// This file is included after the automatically-generated JS library code
// but before the wasm module is created.
{
  // With WASM_ESM_INTEGRATION this has to happen at the top level and not
  // delayed until processModuleArgs.
  initMemory();
  // Begin ATMODULES hooks
  if (Module["noExitRuntime"]) noExitRuntime = Module["noExitRuntime"];
  if (Module["print"]) out = Module["print"];
  if (Module["printErr"]) err = Module["printErr"];
  // End ATMODULES hooks
  if (Module["arguments"]) programArgs = Module["arguments"];
  if (Module["thisProgram"]) thisProgram = Module["thisProgram"];
  var preInit = Module["preInit"];
  if (preInit) {
    if (typeof preInit == "function") Module["preInit"] = preInit = [ preInit ];
    // Written as a loop so that preInit functions that themselves add more
    // preInit functions.  Is this actually needed?
    while (preInit.length > 0) {
      preInit.shift()();
    }
  }
}

// Begin runtime exports
Module["addRunDependency"] = addRunDependency;

Module["removeRunDependency"] = removeRunDependency;

Module["FS_preloadFile"] = FS_preloadFile;

Module["FS_unlink"] = FS_unlink;

Module["FS_createPath"] = FS_createPath;

Module["FS_createDevice"] = FS_createDevice;

Module["FS"] = FS;

Module["FS_createDataFile"] = FS_createDataFile;

Module["FS_createLazyFile"] = FS_createLazyFile;

// End runtime exports
// Begin JS library exports
// End JS library exports
// end include: postlibrary.js
// proxiedFunctionTable specifies the list of functions that can be called
// either synchronously or asynchronously from other threads in postMessage()d
// or internally queued events. This way a pthread in a Worker can synchronously
// access e.g. the DOM on the main thread.
var proxiedFunctionTable = [ _proc_exit, exitOnMainThread, pthreadCreateProxied, ___syscall_chdir, ___syscall_dup, ___syscall_faccessat, ___syscall_fcntl64, ___syscall_fstat64, ___syscall_getcwd, ___syscall_getdents64, ___syscall_ioctl, ___syscall_lstat64, ___syscall_mkdirat, ___syscall_newfstatat, ___syscall_openat, ___syscall_renameat, ___syscall_rmdir, ___syscall_stat64, ___syscall_unlinkat, __mmap_js, __munmap_js, _eglBindAPI, _eglChooseConfig, _eglCreateContext, _eglCreateWindowSurface, _eglDestroyContext, _eglDestroySurface, _eglGetConfigAttrib, _eglGetDisplay, _eglGetError, _eglInitialize, _eglMakeCurrent, _eglQueryString, _eglSwapBuffers, _eglSwapInterval, _eglTerminate, _eglWaitClient, _eglWaitNative, _emscripten_exit_fullscreen, getCanvasSizeMainThread, setCanvasElementSizeMainThread, _emscripten_exit_pointerlock, _emscripten_force_exit, _emscripten_get_device_pixel_ratio, _emscripten_get_element_css_size, _emscripten_get_gamepad_status, _emscripten_get_num_gamepads, _emscripten_get_screen_size, _emscripten_request_fullscreen_strategy, _emscripten_request_pointerlock, _emscripten_sample_gamepad_data, _emscripten_set_beforeunload_callback_on_thread, _emscripten_set_blur_callback_on_thread, _emscripten_set_element_css_size, _emscripten_set_focus_callback_on_thread, _emscripten_set_fullscreenchange_callback_on_thread, _emscripten_set_gamepadconnected_callback_on_thread, _emscripten_set_gamepaddisconnected_callback_on_thread, _emscripten_set_keydown_callback_on_thread, _emscripten_set_keypress_callback_on_thread, _emscripten_set_keyup_callback_on_thread, _emscripten_set_mousedown_callback_on_thread, _emscripten_set_mouseenter_callback_on_thread, _emscripten_set_mouseleave_callback_on_thread, _emscripten_set_mousemove_callback_on_thread, _emscripten_set_mouseup_callback_on_thread, _emscripten_set_pointerlockchange_callback_on_thread, _emscripten_set_resize_callback_on_thread, _emscripten_set_touchcancel_callback_on_thread, _emscripten_set_touchend_callback_on_thread, _emscripten_set_touchmove_callback_on_thread, _emscripten_set_touchstart_callback_on_thread, _emscripten_set_visibilitychange_callback_on_thread, _emscripten_set_wheel_callback_on_thread, _emscripten_set_window_title, _environ_get, _environ_sizes_get, _fd_close, _fd_read, _fd_seek, _fd_write ];

var ASM_CONSTS = {
  621127488: () => {
    var m = /[?&]skytest=([0-9]+)/.exec(location.search);
    return m ? +m[1] : -1;
  },
  621127569: () => {},
  621127570: () => {},
  621127571: () => {},
  621127572: () => {},
  621127573: () => {},
  621127574: () => {},
  621127575: () => {},
  621127576: () => {},
  621127577: () => {},
  621127578: () => {},
  621127579: () => {},
  621127580: () => {},
  621127581: () => {},
  621127582: () => {},
  621127583: () => {},
  621127584: () => {},
  621127585: () => {},
  621127586: () => {},
  621127587: () => {},
  621127588: () => {},
  621127589: () => {},
  621127590: () => {},
  621127591: () => {},
  621127592: () => {},
  621127593: () => {},
  621127594: () => {},
  621127595: () => {},
  621127596: () => {},
  621127597: () => {},
  621127598: () => {},
  621127599: () => {},
  621127600: () => {},
  621127601: () => {},
  621127602: () => {},
  621127603: () => {},
  621127604: () => {},
  621127605: () => {},
  621127606: () => {},
  621127607: () => {},
  621127608: () => {},
  621127609: () => {},
  621127610: () => {},
  621127611: () => {},
  621127612: () => {},
  621127613: () => {},
  621127614: () => {},
  621127615: () => {},
  621127616: () => Module["ptShadows"] === false ? 0 : 1,
  621127664: () => {},
  621127665: () => {},
  621127666: () => {},
  621127667: () => {},
  621127668: () => {},
  621127669: () => {},
  621127670: () => {},
  621127671: () => {},
  621127672: () => {},
  621127673: () => {},
  621127674: () => {},
  621127675: () => {},
  621127676: () => {},
  621127677: () => {},
  621127678: () => {},
  621127679: () => {},
  621127680: () => {},
  621127681: () => {},
  621127682: () => {},
  621127683: () => {},
  621127684: () => {},
  621127685: () => {},
  621127686: () => {},
  621127687: () => {},
  621127688: () => {},
  621127689: () => {},
  621127690: () => {},
  621127691: () => {},
  621127692: () => {},
  621127693: () => {},
  621127694: () => {},
  621127695: () => {},
  621127696: () => {},
  621127697: () => {},
  621127698: () => {},
  621127699: () => {},
  621127700: () => {},
  621127701: () => {},
  621127702: () => {},
  621127703: () => {},
  621127704: () => {},
  621127705: () => {},
  621127706: () => {},
  621127707: () => {},
  621127708: () => {},
  621127709: () => {},
  621127710: () => {},
  621127711: () => {},
  621127712: () => /[?&]shadowtest=2/.test(location.search) ? 2 : /[?&]shadowtest/.test(location.search) ? 1 : 0,
  621127818: () => {},
  621127819: () => {},
  621127820: () => {},
  621127821: () => {},
  621127822: () => {},
  621127823: () => {},
  621127824: () => {},
  621127825: () => {},
  621127826: () => {},
  621127827: () => {},
  621127828: () => {},
  621127829: () => {},
  621127830: () => {},
  621127831: () => {},
  621127832: () => {},
  621127833: () => {},
  621127834: () => {},
  621127835: () => {},
  621127836: () => {},
  621127837: () => {},
  621127838: () => {},
  621127839: () => {},
  621127840: () => {},
  621127841: () => {},
  621127842: () => {},
  621127843: () => {},
  621127844: () => {},
  621127845: () => {},
  621127846: () => {},
  621127847: () => {},
  621127848: () => {},
  621127849: () => {},
  621127850: () => {},
  621127851: () => {},
  621127852: () => {},
  621127853: () => {},
  621127854: () => {},
  621127855: () => {},
  621127856: () => {},
  621127857: () => {},
  621127858: () => {},
  621127859: () => {},
  621127860: () => {},
  621127861: () => {},
  621127862: () => {},
  621127863: () => {},
  621127864: () => {},
  621127865: () => {},
  621127866: () => {},
  621127867: () => {},
  621127868: () => {},
  621127869: () => {},
  621127870: () => {},
  621127871: () => {},
  621127872: () => {
    var d = Module["ptShadowDark"];
    return typeof d == "number" ? d : .6;
  },
  621127945: () => {},
  621127946: () => {},
  621127947: () => {},
  621127948: () => {},
  621127949: () => {},
  621127950: () => {},
  621127951: () => {},
  621127952: () => {},
  621127953: () => {},
  621127954: () => {},
  621127955: () => {},
  621127956: () => {},
  621127957: () => {},
  621127958: () => {},
  621127959: () => {},
  621127960: () => {},
  621127961: () => {},
  621127962: () => {},
  621127963: () => {},
  621127964: () => {},
  621127965: () => {},
  621127966: () => {},
  621127967: () => {},
  621127968: () => {},
  621127969: () => {},
  621127970: () => {},
  621127971: () => {},
  621127972: () => {},
  621127973: () => {},
  621127974: () => {},
  621127975: () => {},
  621127976: () => {},
  621127977: () => {},
  621127978: () => {},
  621127979: () => {},
  621127980: () => {},
  621127981: () => {},
  621127982: () => {},
  621127983: () => {},
  621127984: () => {},
  621127985: () => {},
  621127986: () => {},
  621127987: () => {},
  621127988: () => {},
  621127989: () => {},
  621127990: () => {},
  621127991: () => {},
  621127992: () => {},
  621127993: () => {},
  621127994: () => {},
  621127995: () => {},
  621127996: () => {},
  621127997: () => {},
  621127998: () => {},
  621127999: () => {},
  621128e3: () => {
    var q = Module["ptShadowQuality"];
    return typeof q == "number" ? q : 0;
  },
  621128074: () => {},
  621128075: () => {},
  621128076: () => {},
  621128077: () => {},
  621128078: () => {},
  621128079: () => {},
  621128080: () => {},
  621128081: () => {},
  621128082: () => {},
  621128083: () => {},
  621128084: () => {},
  621128085: () => {},
  621128086: () => {},
  621128087: () => {},
  621128088: () => {},
  621128089: () => {},
  621128090: () => {},
  621128091: () => {},
  621128092: () => {},
  621128093: () => {},
  621128094: () => {},
  621128095: () => {},
  621128096: () => {},
  621128097: () => {},
  621128098: () => {},
  621128099: () => {},
  621128100: () => {},
  621128101: () => {},
  621128102: () => {},
  621128103: () => {},
  621128104: () => {},
  621128105: () => {},
  621128106: () => {},
  621128107: () => {},
  621128108: () => {},
  621128109: () => {},
  621128110: () => {},
  621128111: () => {},
  621128112: () => {},
  621128113: () => {},
  621128114: () => {},
  621128115: () => {},
  621128116: () => {},
  621128117: () => {},
  621128118: () => {},
  621128119: () => {},
  621128120: () => {},
  621128121: () => {},
  621128122: () => {},
  621128123: () => {},
  621128124: () => {},
  621128125: () => {},
  621128126: () => {},
  621128127: () => {},
  621128128: () => /[?&]suntest/.test(location.search) ? 1 : 0,
  621128184: () => {},
  621128185: () => {},
  621128186: () => {},
  621128187: () => {},
  621128188: () => {},
  621128189: () => {},
  621128190: () => {},
  621128191: () => {},
  621128192: () => {},
  621128193: () => {},
  621128194: () => {},
  621128195: () => {},
  621128196: () => {},
  621128197: () => {},
  621128198: () => {},
  621128199: () => {},
  621128200: () => {},
  621128201: () => {},
  621128202: () => {},
  621128203: () => {},
  621128204: () => {},
  621128205: () => {},
  621128206: () => {},
  621128207: () => {},
  621128208: () => {},
  621128209: () => {},
  621128210: () => {},
  621128211: () => {},
  621128212: () => {},
  621128213: () => {},
  621128214: () => {},
  621128215: () => {},
  621128216: () => {},
  621128217: () => {},
  621128218: () => {},
  621128219: () => {},
  621128220: () => {},
  621128221: () => {},
  621128222: () => {},
  621128223: () => {},
  621128224: () => {
    var h = Module["ptSunHour"];
    return typeof h == "number" ? h : -1;
  },
  621128293: () => {},
  621128294: () => {},
  621128295: () => {},
  621128296: () => {},
  621128297: () => {},
  621128298: () => {},
  621128299: () => {},
  621128300: () => {},
  621128301: () => {},
  621128302: () => {},
  621128303: () => {},
  621128304: () => {},
  621128305: () => {},
  621128306: () => {},
  621128307: () => {},
  621128308: () => {},
  621128309: () => {},
  621128310: () => {},
  621128311: () => {},
  621128312: () => {},
  621128313: () => {},
  621128314: () => {},
  621128315: () => {},
  621128316: () => {},
  621128317: () => {},
  621128318: () => {},
  621128319: () => {},
  621128320: () => {},
  621128321: () => {},
  621128322: () => {},
  621128323: () => {},
  621128324: () => {},
  621128325: () => {},
  621128326: () => {},
  621128327: () => {},
  621128328: () => {},
  621128329: () => {},
  621128330: () => {},
  621128331: () => {},
  621128332: () => {},
  621128333: () => {},
  621128334: () => {},
  621128335: () => {},
  621128336: () => {},
  621128337: () => {},
  621128338: () => {},
  621128339: () => {},
  621128340: () => {},
  621128341: () => {},
  621128342: () => {},
  621128343: () => {},
  621128344: () => {},
  621128345: () => {},
  621128346: () => {},
  621128347: () => {},
  621128348: () => {},
  621128349: () => {},
  621128350: () => {},
  621128351: () => {},
  621128352: () => {
    var d = Module["ptSunDay"];
    return typeof d == "number" ? d : 0;
  },
  621128419: () => {},
  621128420: () => {},
  621128421: () => {},
  621128422: () => {},
  621128423: () => {},
  621128424: () => {},
  621128425: () => {},
  621128426: () => {},
  621128427: () => {},
  621128428: () => {},
  621128429: () => {},
  621128430: () => {},
  621128431: () => {},
  621128432: () => {},
  621128433: () => {},
  621128434: () => {},
  621128435: () => {},
  621128436: () => {},
  621128437: () => {},
  621128438: () => {},
  621128439: () => {},
  621128440: () => {},
  621128441: () => {},
  621128442: () => {},
  621128443: () => {},
  621128444: () => {},
  621128445: () => {},
  621128446: () => {},
  621128447: () => {},
  621128448: () => {},
  621128449: () => {},
  621128450: () => {},
  621128451: () => {},
  621128452: () => {},
  621128453: () => {},
  621128454: () => {},
  621128455: () => {},
  621128456: () => {},
  621128457: () => {},
  621128458: () => {},
  621128459: () => {},
  621128460: () => {},
  621128461: () => {},
  621128462: () => {},
  621128463: () => {},
  621128464: () => {},
  621128465: () => {},
  621128466: () => {},
  621128467: () => {},
  621128468: () => {},
  621128469: () => {},
  621128470: () => {},
  621128471: () => {},
  621128472: () => {},
  621128473: () => {},
  621128474: () => {},
  621128475: () => {},
  621128476: () => {},
  621128477: () => {},
  621128478: () => {},
  621128479: () => {},
  621128480: () => {
    var m = /[?&]suntest=(-?[0-9.]+)/.exec(location.search);
    return m ? +m[1] : 0;
  },
  621128563: () => {},
  621128564: () => {},
  621128565: () => {},
  621128566: () => {},
  621128567: () => {},
  621128568: () => {},
  621128569: () => {},
  621128570: () => {},
  621128571: () => {},
  621128572: () => {},
  621128573: () => {},
  621128574: () => {},
  621128575: () => {},
  621128576: () => {},
  621128577: () => {},
  621128578: () => {},
  621128579: () => {},
  621128580: () => {},
  621128581: () => {},
  621128582: () => {},
  621128583: () => {},
  621128584: () => {},
  621128585: () => {},
  621128586: () => {},
  621128587: () => {},
  621128588: () => {},
  621128589: () => {},
  621128590: () => {},
  621128591: () => {},
  621128592: () => {},
  621128593: () => {},
  621128594: () => {},
  621128595: () => {},
  621128596: () => {},
  621128597: () => {},
  621128598: () => {},
  621128599: () => {},
  621128600: () => {},
  621128601: () => {},
  621128602: () => {},
  621128603: () => {},
  621128604: () => {},
  621128605: () => {},
  621128606: () => {},
  621128607: () => {},
  621128608: ($0, $1) => {
    var w = {};
    w.name = UTF8ToString($0);
    w.secs = $1;
    w.at = Date.now();
    Module["ptWeatherNow"] = w;
  },
  621128711: () => {},
  621128712: () => {},
  621128713: () => {},
  621128714: () => {},
  621128715: () => {},
  621128716: () => {},
  621128717: () => {},
  621128718: () => {},
  621128719: () => {},
  621128720: () => {},
  621128721: () => {},
  621128722: () => {},
  621128723: () => {},
  621128724: () => {},
  621128725: () => {},
  621128726: () => {},
  621128727: () => {},
  621128728: () => {},
  621128729: () => {},
  621128730: () => {},
  621128731: () => {},
  621128732: () => {},
  621128733: () => {},
  621128734: () => {},
  621128735: () => {},
  621128736: () => {},
  621128737: () => {},
  621128738: () => {},
  621128739: () => {},
  621128740: () => {},
  621128741: () => {},
  621128742: () => {},
  621128743: () => {},
  621128744: () => {},
  621128745: () => {},
  621128746: () => {},
  621128747: () => {},
  621128748: () => {},
  621128749: () => {},
  621128750: () => {},
  621128751: () => {},
  621128752: () => {},
  621128753: () => {},
  621128754: () => {},
  621128755: () => {},
  621128756: () => {},
  621128757: () => {},
  621128758: () => {},
  621128759: () => {},
  621128760: () => {},
  621128761: () => {},
  621128762: () => {},
  621128763: () => {},
  621128764: () => {},
  621128765: () => {},
  621128766: () => {},
  621128767: () => {},
  621128768: () => {
    Module["ptEvents"] = (Module["ptEvents"] | 0) + 1;
  },
  621128825: () => {},
  621128826: () => {},
  621128827: () => {},
  621128828: () => {},
  621128829: () => {},
  621128830: () => {},
  621128831: () => {},
  621128832: () => {},
  621128833: () => {},
  621128834: () => {},
  621128835: () => {},
  621128836: () => {},
  621128837: () => {},
  621128838: () => {},
  621128839: () => {},
  621128840: () => {},
  621128841: () => {},
  621128842: () => {},
  621128843: () => {},
  621128844: () => {},
  621128845: () => {},
  621128846: () => {},
  621128847: () => {},
  621128848: () => {},
  621128849: () => {},
  621128850: () => {},
  621128851: () => {},
  621128852: () => {},
  621128853: () => {},
  621128854: () => {},
  621128855: () => {},
  621128856: () => {},
  621128857: () => {},
  621128858: () => {},
  621128859: () => {},
  621128860: () => {},
  621128861: () => {},
  621128862: () => {},
  621128863: () => {},
  621128864: () => {
    Module["ptFrames"] = (Module["ptFrames"] | 0) + 1;
  },
  621128921: () => {},
  621128922: () => {},
  621128923: () => {},
  621128924: () => {},
  621128925: () => {},
  621128926: () => {},
  621128927: () => {},
  621128928: () => {},
  621128929: () => {},
  621128930: () => {},
  621128931: () => {},
  621128932: () => {},
  621128933: () => {},
  621128934: () => {},
  621128935: () => {},
  621128936: () => {},
  621128937: () => {},
  621128938: () => {},
  621128939: () => {},
  621128940: () => {},
  621128941: () => {},
  621128942: () => {},
  621128943: () => {},
  621128944: () => {},
  621128945: () => {},
  621128946: () => {},
  621128947: () => {},
  621128948: () => {},
  621128949: () => {},
  621128950: () => {},
  621128951: () => {},
  621128952: () => {},
  621128953: () => {},
  621128954: () => {},
  621128955: () => {},
  621128956: () => {},
  621128957: () => {},
  621128958: () => {},
  621128959: () => {},
  621128960: () => {
    var send = function(t) {
      if (!t) return;
      t = String(t);
      for (var i = 0; i < t.length && i < 200; i++) {
        var c = t.charCodeAt(i);
        Module["_PtWeb_PasteChar"](c < 32 ? 32 : c);
      }
    };
    var box = document.createElement("textarea");
    box.setAttribute("aria-hidden", "true");
    box.tabIndex = -1;
    box.style.cssText = "position:fixed;left:-2000px;top:0;width:20px;height:20px;opacity:0;pointer-events:none;";
    document.body.appendChild(box);
    var back = null;
    var mine = function(e) {
      var n = e.target && e.target.tagName;
      return e.target == box || (n != "INPUT" && n != "TEXTAREA");
    };
    window.addEventListener("keydown", function(e) {
      if (!(e.ctrlKey || e.metaKey) || (e.key != "v" && e.key != "V") || !mine(e)) return;
      back = document.activeElement == box ? back : document.activeElement;
      box.value = "";
      box.focus();
      e.stopImmediatePropagation();
    }, true);
    box.addEventListener("paste", function(e) {
      var t = e.clipboardData && e.clipboardData.getData("text");
      e.preventDefault();
      send(t);
      setTimeout(function() {
        box.value = "";
        if (back && back.focus && back != box) back.focus(); else box.blur();
        back = null;
      }, 0);
    });
  },
  621130122: () => {},
  621130123: () => {},
  621130124: () => {},
  621130125: () => {},
  621130126: () => {},
  621130127: () => {},
  621130128: () => {},
  621130129: () => {},
  621130130: () => {},
  621130131: () => {},
  621130132: () => {},
  621130133: () => {},
  621130134: () => {},
  621130135: () => {},
  621130136: () => {},
  621130137: () => {},
  621130138: () => {},
  621130139: () => {},
  621130140: () => {},
  621130141: () => {},
  621130142: () => {},
  621130143: () => {},
  621130144: () => {},
  621130145: () => {},
  621130146: () => {},
  621130147: () => {},
  621130148: () => {},
  621130149: () => {},
  621130150: () => {},
  621130151: () => {},
  621130152: () => {},
  621130153: () => {},
  621130154: () => {},
  621130155: () => {},
  621130156: () => {},
  621130157: () => {},
  621130158: () => {},
  621130159: () => {},
  621130160: () => {},
  621130161: () => {},
  621130162: () => {},
  621130163: () => {},
  621130164: () => {},
  621130165: () => {},
  621130166: () => {},
  621130167: () => {},
  621130168: () => {},
  621130169: () => {},
  621130170: () => {},
  621130171: () => {},
  621130172: () => {},
  621130173: () => {},
  621130174: () => {},
  621130175: () => {},
  621130176: () => {},
  621130177: () => {},
  621130178: () => {},
  621130179: () => {},
  621130180: () => {},
  621130181: () => {},
  621130182: () => {},
  621130183: () => {},
  621130184: () => {},
  621130185: () => {},
  621130186: () => {},
  621130187: () => {},
  621130188: () => {},
  621130189: () => {},
  621130190: () => {},
  621130191: () => {},
  621130192: () => {},
  621130193: () => {},
  621130194: () => {},
  621130195: () => {},
  621130196: () => {},
  621130197: () => {},
  621130198: () => {},
  621130199: () => {},
  621130200: () => {},
  621130201: () => {},
  621130202: () => {},
  621130203: () => {},
  621130204: () => {},
  621130205: () => {},
  621130206: () => {},
  621130207: () => {},
  621130208: () => {},
  621130209: () => {},
  621130210: () => {},
  621130211: () => {},
  621130212: () => {},
  621130213: () => {},
  621130214: () => {},
  621130215: () => {},
  621130216: () => {},
  621130217: () => {},
  621130218: () => {},
  621130219: () => {},
  621130220: () => {},
  621130221: () => {},
  621130222: () => {},
  621130223: () => {},
  621130224: () => {},
  621130225: () => {},
  621130226: () => {},
  621130227: () => {},
  621130228: () => {},
  621130229: () => {},
  621130230: () => {},
  621130231: () => {},
  621130232: () => {},
  621130233: () => {},
  621130234: () => {},
  621130235: () => {},
  621130236: () => {},
  621130237: () => {},
  621130238: () => {},
  621130239: () => {},
  621130240: () => {},
  621130241: () => {},
  621130242: () => {},
  621130243: () => {},
  621130244: () => {},
  621130245: () => {},
  621130246: () => {},
  621130247: () => {},
  621130248: () => {},
  621130249: () => {},
  621130250: () => {},
  621130251: () => {},
  621130252: () => {},
  621130253: () => {},
  621130254: () => {},
  621130255: () => {},
  621130256: () => {},
  621130257: () => {},
  621130258: () => {},
  621130259: () => {},
  621130260: () => {},
  621130261: () => {},
  621130262: () => {},
  621130263: () => {},
  621130264: () => {},
  621130265: () => {},
  621130266: () => {},
  621130267: () => {},
  621130268: () => {},
  621130269: () => {},
  621130270: () => {},
  621130271: () => {},
  621130272: () => {},
  621130273: () => {},
  621130274: () => {},
  621130275: () => {},
  621130276: () => {},
  621130277: () => {},
  621130278: () => {},
  621130279: () => {},
  621130280: () => {},
  621130281: () => {},
  621130282: () => {},
  621130283: () => {},
  621130284: () => {},
  621130285: () => {},
  621130286: () => {},
  621130287: () => {},
  621130288: () => {},
  621130289: () => {},
  621130290: () => {},
  621130291: () => {},
  621130292: () => {},
  621130293: () => {},
  621130294: () => {},
  621130295: () => {},
  621130296: () => {},
  621130297: () => {},
  621130298: () => {},
  621130299: () => {},
  621130300: () => {},
  621130301: () => {},
  621130302: () => {},
  621130303: () => {},
  621130304: () => {},
  621130305: () => {},
  621130306: () => {},
  621130307: () => {},
  621130308: () => {},
  621130309: () => {},
  621130310: () => {},
  621130311: () => {},
  621130312: () => {},
  621130313: () => {},
  621130314: () => {},
  621130315: () => {},
  621130316: () => {},
  621130317: () => {},
  621130318: () => {},
  621130319: () => {},
  621130320: () => {},
  621130321: () => {},
  621130322: () => {},
  621130323: () => {},
  621130324: () => {},
  621130325: () => {},
  621130326: () => {},
  621130327: () => {},
  621130328: () => {},
  621130329: () => {},
  621130330: () => {},
  621130331: () => {},
  621130332: () => {},
  621130333: () => {},
  621130334: () => {},
  621130335: () => {},
  621130336: () => {},
  621130337: () => {},
  621130338: () => {},
  621130339: () => {},
  621130340: () => {},
  621130341: () => {},
  621130342: () => {},
  621130343: () => {},
  621130344: () => {},
  621130345: () => {},
  621130346: () => {},
  621130347: () => {},
  621130348: () => {},
  621130349: () => {},
  621130350: () => {},
  621130351: () => {},
  621130352: () => {},
  621130353: () => {},
  621130354: () => {},
  621130355: () => {},
  621130356: () => {},
  621130357: () => {},
  621130358: () => {},
  621130359: () => {},
  621130360: () => {},
  621130361: () => {},
  621130362: () => {},
  621130363: () => {},
  621130364: () => {},
  621130365: () => {},
  621130366: () => {},
  621130367: () => {},
  621130368: () => {},
  621130369: () => {},
  621130370: () => {},
  621130371: () => {},
  621130372: () => {},
  621130373: () => {},
  621130374: () => {},
  621130375: () => {},
  621130376: () => {},
  621130377: () => {},
  621130378: () => {},
  621130379: () => {},
  621130380: () => {},
  621130381: () => {},
  621130382: () => {},
  621130383: () => {},
  621130384: () => {},
  621130385: () => {},
  621130386: () => {},
  621130387: () => {},
  621130388: () => {},
  621130389: () => {},
  621130390: () => {},
  621130391: () => {},
  621130392: () => {},
  621130393: () => {},
  621130394: () => {},
  621130395: () => {},
  621130396: () => {},
  621130397: () => {},
  621130398: () => {},
  621130399: () => {},
  621130400: () => {},
  621130401: () => {},
  621130402: () => {},
  621130403: () => {},
  621130404: () => {},
  621130405: () => {},
  621130406: () => {},
  621130407: () => {},
  621130408: () => {},
  621130409: () => {},
  621130410: () => {},
  621130411: () => {},
  621130412: () => {},
  621130413: () => {},
  621130414: () => {},
  621130415: () => {},
  621130416: () => {},
  621130417: () => {},
  621130418: () => {},
  621130419: () => {},
  621130420: () => {},
  621130421: () => {},
  621130422: () => {},
  621130423: () => {},
  621130424: () => {},
  621130425: () => {},
  621130426: () => {},
  621130427: () => {},
  621130428: () => {},
  621130429: () => {},
  621130430: () => {},
  621130431: () => {},
  621130432: () => Module["ptDebugInput"] ? 1 : 0,
  621130475: () => {},
  621130476: () => {},
  621130477: () => {},
  621130478: () => {},
  621130479: () => {},
  621130480: () => {},
  621130481: () => {},
  621130482: () => {},
  621130483: () => {},
  621130484: () => {},
  621130485: () => {},
  621130486: () => {},
  621130487: () => {},
  621130488: () => {},
  621130489: () => {},
  621130490: () => {},
  621130491: () => {},
  621130492: () => {},
  621130493: () => {},
  621130494: () => {},
  621130495: () => {},
  621130496: () => {},
  621130497: () => {},
  621130498: () => {},
  621130499: () => {},
  621130500: () => {},
  621130501: () => {},
  621130502: () => {},
  621130503: () => {},
  621130504: () => {},
  621130505: () => {},
  621130506: () => {},
  621130507: () => {},
  621130508: () => {},
  621130509: () => {},
  621130510: () => {},
  621130511: () => {},
  621130512: () => {},
  621130513: () => {},
  621130514: () => {},
  621130515: () => {},
  621130516: () => {},
  621130517: () => {},
  621130518: () => {},
  621130519: () => {},
  621130520: () => {},
  621130521: () => {},
  621130522: () => {},
  621130523: () => {},
  621130524: () => {},
  621130525: () => {},
  621130526: () => {},
  621130527: () => {},
  621130528: () => {
    var o = Module["ptOpts"];
    return o ? (o.v | 0) : 0;
  },
  621130582: () => {},
  621130583: () => {},
  621130584: () => {},
  621130585: () => {},
  621130586: () => {},
  621130587: () => {},
  621130588: () => {},
  621130589: () => {},
  621130590: () => {},
  621130591: () => {},
  621130592: () => {},
  621130593: () => {},
  621130594: () => {},
  621130595: () => {},
  621130596: () => {},
  621130597: () => {},
  621130598: () => {},
  621130599: () => {},
  621130600: () => {},
  621130601: () => {},
  621130602: () => {},
  621130603: () => {},
  621130604: () => {},
  621130605: () => {},
  621130606: () => {},
  621130607: () => {},
  621130608: () => {},
  621130609: () => {},
  621130610: () => {},
  621130611: () => {},
  621130612: () => {},
  621130613: () => {},
  621130614: () => {},
  621130615: () => {},
  621130616: () => {},
  621130617: () => {},
  621130618: () => {},
  621130619: () => {},
  621130620: () => {},
  621130621: () => {},
  621130622: () => {},
  621130623: () => {},
  621130624: () => {
    var o = Module["ptOpts"];
    return o && typeof o.music == "number" ? o.music : 1;
  },
  621130706: () => {},
  621130707: () => {},
  621130708: () => {},
  621130709: () => {},
  621130710: () => {},
  621130711: () => {},
  621130712: () => {},
  621130713: () => {},
  621130714: () => {},
  621130715: () => {},
  621130716: () => {},
  621130717: () => {},
  621130718: () => {},
  621130719: () => {},
  621130720: () => {},
  621130721: () => {},
  621130722: () => {},
  621130723: () => {},
  621130724: () => {},
  621130725: () => {},
  621130726: () => {},
  621130727: () => {},
  621130728: () => {},
  621130729: () => {},
  621130730: () => {},
  621130731: () => {},
  621130732: () => {},
  621130733: () => {},
  621130734: () => {},
  621130735: () => {},
  621130736: () => {},
  621130737: () => {},
  621130738: () => {},
  621130739: () => {},
  621130740: () => {},
  621130741: () => {},
  621130742: () => {},
  621130743: () => {},
  621130744: () => {},
  621130745: () => {},
  621130746: () => {},
  621130747: () => {},
  621130748: () => {},
  621130749: () => {},
  621130750: () => {},
  621130751: () => {},
  621130752: () => {
    var o = Module["ptOpts"];
    return o && typeof o.effects == "number" ? o.effects : 1;
  },
  621130838: () => {},
  621130839: () => {},
  621130840: () => {},
  621130841: () => {},
  621130842: () => {},
  621130843: () => {},
  621130844: () => {},
  621130845: () => {},
  621130846: () => {},
  621130847: () => {},
  621130848: () => {},
  621130849: () => {},
  621130850: () => {},
  621130851: () => {},
  621130852: () => {},
  621130853: () => {},
  621130854: () => {},
  621130855: () => {},
  621130856: () => {},
  621130857: () => {},
  621130858: () => {},
  621130859: () => {},
  621130860: () => {},
  621130861: () => {},
  621130862: () => {},
  621130863: () => {},
  621130864: () => {},
  621130865: () => {},
  621130866: () => {},
  621130867: () => {},
  621130868: () => {},
  621130869: () => {},
  621130870: () => {},
  621130871: () => {},
  621130872: () => {},
  621130873: () => {},
  621130874: () => {},
  621130875: () => {},
  621130876: () => {},
  621130877: () => {},
  621130878: () => {},
  621130879: () => {},
  621130880: () => {
    var o = Module["ptOpts"];
    return o && o.view ? (o.view | 0) : 0;
  },
  621130947: () => {},
  621130948: () => {},
  621130949: () => {},
  621130950: () => {},
  621130951: () => {},
  621130952: () => {},
  621130953: () => {},
  621130954: () => {},
  621130955: () => {},
  621130956: () => {},
  621130957: () => {},
  621130958: () => {},
  621130959: () => {},
  621130960: () => {},
  621130961: () => {},
  621130962: () => {},
  621130963: () => {},
  621130964: () => {},
  621130965: () => {},
  621130966: () => {},
  621130967: () => {},
  621130968: () => {},
  621130969: () => {},
  621130970: () => {},
  621130971: () => {},
  621130972: () => {},
  621130973: () => {},
  621130974: () => {},
  621130975: () => {},
  621130976: () => {},
  621130977: () => {},
  621130978: () => {},
  621130979: () => {},
  621130980: () => {},
  621130981: () => {},
  621130982: () => {},
  621130983: () => {},
  621130984: () => {},
  621130985: () => {},
  621130986: () => {},
  621130987: () => {},
  621130988: () => {},
  621130989: () => {},
  621130990: () => {},
  621130991: () => {},
  621130992: () => {},
  621130993: () => {},
  621130994: () => {},
  621130995: () => {},
  621130996: () => {},
  621130997: () => {},
  621130998: () => {},
  621130999: () => {},
  621131e3: () => {},
  621131001: () => {},
  621131002: () => {},
  621131003: () => {},
  621131004: () => {},
  621131005: () => {},
  621131006: () => {},
  621131007: () => {},
  621131008: () => {
    var o = Module["ptOpts"];
    return o && o.weather ? (o.weather | 0) : 0;
  },
  621131081: () => {},
  621131082: () => {},
  621131083: () => {},
  621131084: () => {},
  621131085: () => {},
  621131086: () => {},
  621131087: () => {},
  621131088: () => {},
  621131089: () => {},
  621131090: () => {},
  621131091: () => {},
  621131092: () => {},
  621131093: () => {},
  621131094: () => {},
  621131095: () => {},
  621131096: () => {},
  621131097: () => {},
  621131098: () => {},
  621131099: () => {},
  621131100: () => {},
  621131101: () => {},
  621131102: () => {},
  621131103: () => {},
  621131104: () => {},
  621131105: () => {},
  621131106: () => {},
  621131107: () => {},
  621131108: () => {},
  621131109: () => {},
  621131110: () => {},
  621131111: () => {},
  621131112: () => {},
  621131113: () => {},
  621131114: () => {},
  621131115: () => {},
  621131116: () => {},
  621131117: () => {},
  621131118: () => {},
  621131119: () => {},
  621131120: () => {},
  621131121: () => {},
  621131122: () => {},
  621131123: () => {},
  621131124: () => {},
  621131125: () => {},
  621131126: () => {},
  621131127: () => {},
  621131128: () => {},
  621131129: () => {},
  621131130: () => {},
  621131131: () => {},
  621131132: () => {},
  621131133: () => {},
  621131134: () => {},
  621131135: () => {},
  621131136: () => {
    var o = Module["ptOpts"];
    return o && o.lang ? (o.lang | 0) : 0;
  },
  621131203: () => {},
  621131204: () => {},
  621131205: () => {},
  621131206: () => {},
  621131207: () => {},
  621131208: () => {},
  621131209: () => {},
  621131210: () => {},
  621131211: () => {},
  621131212: () => {},
  621131213: () => {},
  621131214: () => {},
  621131215: () => {},
  621131216: () => {},
  621131217: () => {},
  621131218: () => {},
  621131219: () => {},
  621131220: () => {},
  621131221: () => {},
  621131222: () => {},
  621131223: () => {},
  621131224: () => {},
  621131225: () => {},
  621131226: () => {},
  621131227: () => {},
  621131228: () => {},
  621131229: () => {},
  621131230: () => {},
  621131231: () => {},
  621131232: () => {},
  621131233: () => {},
  621131234: () => {},
  621131235: () => {},
  621131236: () => {},
  621131237: () => {},
  621131238: () => {},
  621131239: () => {},
  621131240: () => {},
  621131241: () => {},
  621131242: () => {},
  621131243: () => {},
  621131244: () => {},
  621131245: () => {},
  621131246: () => {},
  621131247: () => {},
  621131248: () => {},
  621131249: () => {},
  621131250: () => {},
  621131251: () => {},
  621131252: () => {},
  621131253: () => {},
  621131254: () => {},
  621131255: () => {},
  621131256: () => {},
  621131257: () => {},
  621131258: () => {},
  621131259: () => {},
  621131260: () => {},
  621131261: () => {},
  621131262: () => {},
  621131263: () => {},
  621131264: ($0, $1) => {
    if (Module["ptSaveChanged"]) Module["ptSaveChanged"](UTF8ToString($0), $1);
    _free($0);
  },
  621131363: () => {},
  621131364: () => {},
  621131365: () => {},
  621131366: () => {},
  621131367: () => {},
  621131368: () => {},
  621131369: () => {},
  621131370: () => {},
  621131371: () => {},
  621131372: () => {},
  621131373: () => {},
  621131374: () => {},
  621131375: () => {},
  621131376: () => {},
  621131377: () => {},
  621131378: () => {},
  621131379: () => {},
  621131380: () => {},
  621131381: () => {},
  621131382: () => {},
  621131383: () => {},
  621131384: () => {},
  621131385: () => {},
  621131386: () => {},
  621131387: () => {},
  621131388: () => {},
  621131389: () => {},
  621131390: () => {},
  621131391: () => {},
  621131392: () => {},
  621131393: () => {},
  621131394: () => {},
  621131395: () => {},
  621131396: () => {},
  621131397: () => {},
  621131398: () => {},
  621131399: () => {},
  621131400: () => {},
  621131401: () => {},
  621131402: () => {},
  621131403: () => {},
  621131404: () => {},
  621131405: () => {},
  621131406: () => {},
  621131407: () => {},
  621131408: () => {},
  621131409: () => {},
  621131410: () => {},
  621131411: () => {},
  621131412: () => {},
  621131413: () => {},
  621131414: () => {},
  621131415: () => {},
  621131416: () => {},
  621131417: () => {},
  621131418: () => {},
  621131419: () => {},
  621131420: () => {},
  621131421: () => {},
  621131422: () => {},
  621131423: () => {},
  621131424: $0 => {
    if (Module["ptCrumb"]) Module["ptCrumb"](UTF8ToString($0));
  },
  621131494: () => {},
  621131495: () => {},
  621131496: () => {},
  621131497: () => {},
  621131498: () => {},
  621131499: () => {},
  621131500: () => {},
  621131501: () => {},
  621131502: () => {},
  621131503: () => {},
  621131504: () => {},
  621131505: () => {},
  621131506: () => {},
  621131507: () => {},
  621131508: () => {},
  621131509: () => {},
  621131510: () => {},
  621131511: () => {},
  621131512: () => {},
  621131513: () => {},
  621131514: () => {},
  621131515: () => {},
  621131516: () => {},
  621131517: () => {},
  621131518: () => {},
  621131519: () => {},
  621131520: () => {},
  621131521: () => {},
  621131522: () => {},
  621131523: () => {},
  621131524: () => {},
  621131525: () => {},
  621131526: () => {},
  621131527: () => {},
  621131528: () => {},
  621131529: () => {},
  621131530: () => {},
  621131531: () => {},
  621131532: () => {},
  621131533: () => {},
  621131534: () => {},
  621131535: () => {},
  621131536: () => {},
  621131537: () => {},
  621131538: () => {},
  621131539: () => {},
  621131540: () => {},
  621131541: () => {},
  621131542: () => {},
  621131543: () => {},
  621131544: () => {},
  621131545: () => {},
  621131546: () => {},
  621131547: () => {},
  621131548: () => {},
  621131549: () => {},
  621131550: () => {},
  621131551: () => {},
  621131552: $0 => {
    var s = UTF8ToString($0);
    _free($0);
    if (Module["ptCrumb"]) Module["ptCrumb"]("(thread) " + s);
  },
  621131660: () => {},
  621131661: () => {},
  621131662: () => {},
  621131663: () => {},
  621131664: () => {},
  621131665: () => {},
  621131666: () => {},
  621131667: () => {},
  621131668: () => {},
  621131669: () => {},
  621131670: () => {},
  621131671: () => {},
  621131672: () => {},
  621131673: () => {},
  621131674: () => {},
  621131675: () => {},
  621131676: () => {},
  621131677: () => {},
  621131678: () => {},
  621131679: () => {},
  621131680: () => {},
  621131681: () => {},
  621131682: () => {},
  621131683: () => {},
  621131684: () => {},
  621131685: () => {},
  621131686: () => {},
  621131687: () => {},
  621131688: () => {},
  621131689: () => {},
  621131690: () => {},
  621131691: () => {},
  621131692: () => {},
  621131693: () => {},
  621131694: () => {},
  621131695: () => {},
  621131696: () => {},
  621131697: () => {},
  621131698: () => {},
  621131699: () => {},
  621131700: () => {},
  621131701: () => {},
  621131702: () => {},
  621131703: () => {},
  621131704: () => {},
  621131705: () => {},
  621131706: () => {},
  621131707: () => {},
  621131708: () => {},
  621131709: () => {},
  621131710: () => {},
  621131711: () => {},
  621131712: ($0, $1, $2, $3, $4) => {
    var bytes = (growMemViews(), HEAPU8).slice($3, $3 + $4);
    _free($3);
    if (Module["ptNetPost"]) Module["ptNetPost"]($0, $1, $2, bytes);
  },
  621131839: () => {},
  621131840: () => {},
  621131841: () => {},
  621131842: () => {},
  621131843: () => {},
  621131844: () => {},
  621131845: () => {},
  621131846: () => {},
  621131847: () => {},
  621131848: () => {},
  621131849: () => {},
  621131850: () => {},
  621131851: () => {},
  621131852: () => {},
  621131853: () => {},
  621131854: () => {},
  621131855: () => {},
  621131856: () => {},
  621131857: () => {},
  621131858: () => {},
  621131859: () => {},
  621131860: () => {},
  621131861: () => {},
  621131862: () => {},
  621131863: () => {},
  621131864: () => {},
  621131865: () => {},
  621131866: () => {},
  621131867: () => {},
  621131868: () => {},
  621131869: () => {},
  621131870: () => {},
  621131871: () => {},
  621131872: () => {
    if (typeof (AudioContext) !== "undefined") {
      return true;
    } else if (typeof (webkitAudioContext) !== "undefined") {
      return true;
    }
    return false;
  },
  621132019: () => {
    if ((typeof (navigator.mediaDevices) !== "undefined") && (typeof (navigator.mediaDevices.getUserMedia) !== "undefined")) {
      return true;
    } else if (typeof (navigator.webkitGetUserMedia) !== "undefined") {
      return true;
    }
    return false;
  },
  621132253: $0 => {
    if (typeof (Module["SDL2"]) === "undefined") {
      Module["SDL2"] = {};
    }
    var SDL2 = Module["SDL2"];
    if (!$0) {
      SDL2.audio = {};
    } else {
      SDL2.capture = {};
    }
    if (!SDL2.audioContext) {
      if (typeof (AudioContext) !== "undefined") {
        SDL2.audioContext = new AudioContext;
      } else if (typeof (webkitAudioContext) !== "undefined") {
        SDL2.audioContext = new webkitAudioContext;
      }
      if (SDL2.audioContext) {
        if ((typeof navigator.userActivation) === "undefined") {
          autoResumeAudioContext(SDL2.audioContext);
        }
      }
    }
    return SDL2.audioContext === undefined ? -1 : 0;
  },
  621132805: () => {
    var SDL2 = Module["SDL2"];
    return SDL2.audioContext.sampleRate;
  },
  621132873: ($0, $1, $2, $3) => {
    var SDL2 = Module["SDL2"];
    var have_microphone = function(stream) {
      if (SDL2.capture.silenceTimer !== undefined) {
        clearInterval(SDL2.capture.silenceTimer);
        SDL2.capture.silenceTimer = undefined;
        SDL2.capture.silenceBuffer = undefined;
      }
      SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream);
      SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1);
      SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {
        if ((SDL2 === undefined) || (SDL2.capture === undefined)) {
          return;
        }
        audioProcessingEvent.outputBuffer.getChannelData(0).fill(0);
        SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer;
        dynCall("vp", $2, [ $3 ]);
      };
      SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode);
      SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination);
      SDL2.capture.stream = stream;
    };
    var no_microphone = function(error) {};
    SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate);
    SDL2.capture.silenceBuffer.getChannelData(0).fill(0);
    var silence_callback = function() {
      SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer;
      dynCall("vp", $2, [ $3 ]);
    };
    SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1e3);
    if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) {
      navigator.mediaDevices.getUserMedia({
        audio: true,
        video: false
      }).then(have_microphone).catch(no_microphone);
    } else if (navigator.webkitGetUserMedia !== undefined) {
      navigator.webkitGetUserMedia({
        audio: true,
        video: false
      }, have_microphone, no_microphone);
    }
  },
  621134566: ($0, $1, $2, $3) => {
    var SDL2 = Module["SDL2"];
    SDL2.audio.scriptProcessorNode = SDL2.audioContext["createScriptProcessor"]($1, 0, $0);
    SDL2.audio.scriptProcessorNode["onaudioprocess"] = function(e) {
      if ((SDL2 === undefined) || (SDL2.audio === undefined)) {
        return;
      }
      if (SDL2.audio.silenceTimer !== undefined) {
        clearInterval(SDL2.audio.silenceTimer);
        SDL2.audio.silenceTimer = undefined;
        SDL2.audio.silenceBuffer = undefined;
      }
      SDL2.audio.currentOutputBuffer = e["outputBuffer"];
      dynCall("vp", $2, [ $3 ]);
    };
    SDL2.audio.scriptProcessorNode["connect"](SDL2.audioContext["destination"]);
    if (SDL2.audioContext.state === "suspended") {
      SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate);
      SDL2.audio.silenceBuffer.getChannelData(0).fill(0);
      var silence_callback = function() {
        if ((typeof navigator.userActivation) !== "undefined") {
          if (navigator.userActivation.hasBeenActive) {
            SDL2.audioContext.resume();
          }
        }
        SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer;
        dynCall("vp", $2, [ $3 ]);
        SDL2.audio.currentOutputBuffer = undefined;
      };
      SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1e3);
    }
  },
  621135741: ($0, $1) => {
    var SDL2 = Module["SDL2"];
    var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels;
    for (var c = 0; c < numChannels; ++c) {
      var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c);
      if (channelData.length != $1) {
        throw "Web Audio capture buffer length mismatch! Destination size: " + channelData.length + " samples vs expected " + $1 + " samples!";
      }
      if (numChannels == 1) {
        for (var j = 0; j < $1; ++j) {
          setValue($0 + (j * 4), channelData[j], "float");
        }
      } else {
        for (var j = 0; j < $1; ++j) {
          setValue($0 + (((j * numChannels) + c) * 4), channelData[j], "float");
        }
      }
    }
  },
  621136346: ($0, $1) => {
    var SDL2 = Module["SDL2"];
    var buf = $0 >>> 2;
    var numChannels = SDL2.audio.currentOutputBuffer["numberOfChannels"];
    for (var c = 0; c < numChannels; ++c) {
      var channelData = SDL2.audio.currentOutputBuffer["getChannelData"](c);
      if (channelData.length != $1) {
        throw "Web Audio output buffer length mismatch! Destination size: " + channelData.length + " samples vs expected " + $1 + " samples!";
      }
      for (var j = 0; j < $1; ++j) {
        channelData[j] = (growMemViews(), HEAPF32)[_asan_js_check_index((growMemViews(), 
        HEAPF32), buf + (j * numChannels + c) >>> 0, ___asan_loadN)];
      }
    }
  },
  621136835: $0 => {
    var SDL2 = Module["SDL2"];
    if ($0) {
      if (SDL2.capture.silenceTimer !== undefined) {
        clearInterval(SDL2.capture.silenceTimer);
      }
      if (SDL2.capture.stream !== undefined) {
        var tracks = SDL2.capture.stream.getAudioTracks();
        for (var i = 0; i < tracks.length; i++) {
          SDL2.capture.stream.removeTrack(tracks[i]);
        }
      }
      if (SDL2.capture.scriptProcessorNode !== undefined) {
        SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {};
        SDL2.capture.scriptProcessorNode.disconnect();
      }
      if (SDL2.capture.mediaStreamNode !== undefined) {
        SDL2.capture.mediaStreamNode.disconnect();
      }
      SDL2.capture = undefined;
    } else {
      if (SDL2.audio.scriptProcessorNode != undefined) {
        SDL2.audio.scriptProcessorNode.disconnect();
      }
      if (SDL2.audio.silenceTimer !== undefined) {
        clearInterval(SDL2.audio.silenceTimer);
      }
      SDL2.audio = undefined;
    }
    if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) {
      SDL2.audioContext.close();
      SDL2.audioContext = undefined;
    }
  },
  621137841: ($0, $1, $2) => {
    var w = $0;
    var h = $1;
    var pixels = $2;
    if (!Module["SDL2"]) Module["SDL2"] = {};
    var SDL2 = Module["SDL2"];
    if (SDL2.ctxCanvas !== Module["canvas"]) {
      SDL2.ctx = Browser.createContext(Module["canvas"], false, true);
      SDL2.ctxCanvas = Module["canvas"];
    }
    if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) {
      SDL2.image = SDL2.ctx.createImageData(w, h);
      SDL2.w = w;
      SDL2.h = h;
      SDL2.imageCtx = SDL2.ctx;
    }
    var data = SDL2.image.data;
    var src = pixels / 4;
    var dst = 0;
    var num;
    if (typeof CanvasPixelArray !== "undefined" && data instanceof CanvasPixelArray) {
      num = data.length;
      while (dst < num) {
        var val = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), src >>> 0, ___asan_loadN)];
        data[dst] = val & 255;
        data[dst + 1] = (val >> 8) & 255;
        data[dst + 2] = (val >> 16) & 255;
        data[dst + 3] = 255;
        src++;
        dst += 4;
      }
    } else {
      if (SDL2.data32Data !== data) {
        SDL2.data32 = new Int32Array(data.buffer);
        SDL2.data8 = new Uint8Array(data.buffer);
        SDL2.data32Data = data;
      }
      var data32 = SDL2.data32;
      num = data32.length;
      data32.set((growMemViews(), HEAP32).subarray(src >>> 0, src + num >>> 0));
      var data8 = SDL2.data8;
      var i = 3;
      var j = i + 4 * num;
      if (num % 8 == 0) {
        while (i < j) {
          data8[i] = 255;
          i = i + 4 | 0;
          data8[i] = 255;
          i = i + 4 | 0;
          data8[i] = 255;
          i = i + 4 | 0;
          data8[i] = 255;
          i = i + 4 | 0;
          data8[i] = 255;
          i = i + 4 | 0;
          data8[i] = 255;
          i = i + 4 | 0;
          data8[i] = 255;
          i = i + 4 | 0;
          data8[i] = 255;
          i = i + 4 | 0;
        }
      } else {
        while (i < j) {
          data8[i] = 255;
          i = i + 4 | 0;
        }
      }
    }
    SDL2.ctx.putImageData(SDL2.image, 0, 0);
  },
  621139307: ($0, $1, $2, $3, $4) => {
    var w = $0;
    var h = $1;
    var hot_x = $2;
    var hot_y = $3;
    var pixels = $4;
    var canvas = document.createElement("canvas");
    canvas.width = w;
    canvas.height = h;
    var ctx = canvas.getContext("2d");
    var image = ctx.createImageData(w, h);
    var data = image.data;
    var src = pixels / 4;
    var dst = 0;
    var num;
    if (typeof CanvasPixelArray !== "undefined" && data instanceof CanvasPixelArray) {
      num = data.length;
      while (dst < num) {
        var val = (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), src >>> 0, ___asan_loadN)];
        data[dst] = val & 255;
        data[dst + 1] = (val >> 8) & 255;
        data[dst + 2] = (val >> 16) & 255;
        data[dst + 3] = (val >> 24) & 255;
        src++;
        dst += 4;
      }
    } else {
      var data32 = new Int32Array(data.buffer);
      num = data32.length;
      data32.set((growMemViews(), HEAP32).subarray(src >>> 0, src + num >>> 0));
    }
    ctx.putImageData(image, 0, 0);
    var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto";
    var urlBuf = _malloc(url.length + 1);
    stringToUTF8(url, urlBuf, url.length + 1);
    return urlBuf;
  },
  621140295: $0 => {
    if (Module["canvas"]) {
      Module["canvas"].style["cursor"] = UTF8ToString($0);
    }
  },
  621140378: () => {
    if (Module["canvas"]) {
      Module["canvas"].style["cursor"] = "none";
    }
  },
  621140447: () => window.innerWidth,
  621140477: () => window.innerHeight
};

function PtSound_HaveAudio() {
  return (typeof AudioContext !== "undefined" || typeof webkitAudioContext !== "undefined") ? 1 : 0;
}

function PtWeb_PrefetchHint(list) {
  if (Module["ptNoPrefetch"]) return;
  var body = UTF8ToString(list);
  if (typeof window != "undefined") {
    fetch("__pt_prefetch", {
      method: "POST",
      body
    }).then(function(r) {
      if (r.status != 204) Module["ptNoPrefetch"] = 1;
    }, function() {
      Module["ptNoPrefetch"] = 1;
    });
    return;
  }
  try {
    var xhr = new XMLHttpRequest;
    xhr.open("POST", "__pt_prefetch", false);
    xhr.send(body);
    if (xhr.status != 204) Module["ptNoPrefetch"] = 1;
  } catch (e) {
    Module["ptNoPrefetch"] = 1;
  }
}

function PtWin_DecodeCodePage(cp, src, len, dst, cap) {
  var names = {
    949: "euc-kr",
    932: "shift_jis",
    936: "gbk",
    950: "big5",
    874: "windows-874",
    1258: "windows-1258"
  };
  if (!names[cp]) return -1;
  var cache = Module["ptDecoders"] || (Module["ptDecoders"] = {});
  var dec = cache[cp] || (cache[cp] = new TextDecoder(names[cp]));
  var s = dec.decode((growMemViews(), HEAPU8).slice(src, src + len)).normalize("NFC");
  for (var i = 0; i < s.length && i < cap; i++) (growMemViews(), HEAPU16)[_asan_js_check_index((growMemViews(), 
  HEAPU16), (dst >> 1) + i >>> 0, ___asan_storeN)] = s.charCodeAt(i);
  return s.length;
}

function PtFs_FetchSync(url, pSize) {
  var u = UTF8ToString(url);
  for (var attempt = 0; attempt < 3; attempt++) {
    var xhr = new XMLHttpRequest;
    xhr.open("GET", u, false);
    xhr.responseType = "arraybuffer";
    try {
      xhr.timeout = 45e3;
    } catch (e) {}
    try {
      xhr.send(null);
    } catch (e) {
      continue;
    }
    if (xhr.status == 404) return 0;
    if (xhr.status != 200) continue;
    var a = new Uint8Array(xhr.response);
    var p = _malloc(a.length || 1);
    (growMemViews(), HEAPU8).set(a, p >>> 0);
    (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), pSize >>> 2, ___asan_storeN)] = a.length;
    return p;
  }
  return 0;
}

function __asyncjs__PtFs_FetchAsync(url, pSize) {
  return Asyncify.handleAsync(async () => {
    var u = UTF8ToString(url);
    for (var attempt = 0; attempt < 3; attempt++) {
      var ctl = self.AbortController ? new AbortController : null;
      var timer = ctl ? setTimeout(function() {
        ctl.abort();
      }, 45e3) : 0;
      try {
        var r = await fetch(u, ctl ? {
          signal: ctl.signal
        } : {});
        if (r.status == 404) {
          clearTimeout(timer);
          return 0;
        }
        if (!r.ok) throw new Error("HTTP " + r.status);
        var a = new Uint8Array(await r.arrayBuffer());
        clearTimeout(timer);
        var p = _malloc(a.length || 1);
        (growMemViews(), HEAPU8).set(a, p >>> 0);
        (growMemViews(), HEAP32)[_asan_js_check_index((growMemViews(), HEAP32), pSize >>> 2, ___asan_storeN)] = a.length;
        return p;
      } catch (e) {
        clearTimeout(timer);
        await new Promise(function(ok) {
          setTimeout(ok, 400 * (attempt + 1));
        });
      }
    }
    return 0;
  });
}

function PtFs_DataUrl(rel) {
  var s = "data/" + UTF8ToString(rel).split("/").map(encodeURIComponent).join("/");
  return stringToNewUTF8(s);
}

function PtFs_NoteWait(on, rel) {
  try {
    if (!self.__ptWaitChannel) self.__ptWaitChannel = new BroadcastChannel("pt-downloads");
    self.__ptWaitChannel.postMessage({
      t: "gwait",
      on,
      rel: UTF8ToString(rel)
    });
  } catch (e) {}
}

function PtWeb_SendCrashReport(kind, text) {
  try {
    var x = new XMLHttpRequest;
    x.open("POST", self.location.origin + "/crash-report.ashx?kind=" + encodeURIComponent(UTF8ToString(kind)), false);
    x.setRequestHeader("Content-Type", "text/plain;charset=utf-8");
    x.send(UTF8ToString(text));
  } catch (e) {}
}

function PtWeb_IsServerJS() {
  return Module["ptServer"] ? 1 : 0;
}

function PtWeb_StartServerJS() {
  if (Module["ptStartServer"]) Module["ptStartServer"]();
}

function PtWeb_RemoteServerJS() {
  return Module["ptServerUrl"] ? 1 : 0;
}

function PtNet_IsServerInstance() {
  return Module["ptServer"] ? 1 : 0;
}

function PtNet_DebugFlag() {
  return (Module["ptDebugNet"] || (typeof location != "undefined" && /debugnet/.test(location.search))) ? 1 : 0;
}

function PtWin_Alert(s) {
  if (typeof alert === "function") alert(UTF8ToString(s));
}

// Imports from the Wasm binary.
var _malloc, _free, _PtWeb_PasteChar, _pthread_self, _main, _PtNet_Receive, __emscripten_tls_init, _emscripten_builtin_memalign, __emscripten_run_callback_on_thread, ___funcs_on_exit, __emscripten_thread_init, ___set_thread_state, __emscripten_thread_crashed, _emscripten_stack_get_end, _emscripten_stack_get_base, __emscripten_run_js_on_main_thread_done, __emscripten_run_js_on_main_thread, __emscripten_thread_free_data, __emscripten_thread_exit, __emscripten_check_mailbox, _setThrew, _emscripten_stack_init, _emscripten_stack_set_limits, _emscripten_stack_get_free, __emscripten_stack_restore, __emscripten_stack_alloc, _emscripten_stack_get_current, __ZN6__asan9FakeStack17AddrIsInFakeStackEm, __ZN6__asan9FakeStack8AllocateEmmm, ___asan_loadN, ___asan_storeN, ___lsan_disable, ___lsan_enable, ___set_stack_limits, dynCall_vi, dynCall_ii, dynCall_v, dynCall_viiiiiii, dynCall_iii, dynCall_viiiiiiiiii, dynCall_iiiiiiiii, dynCall_iiiii, dynCall_iiii, dynCall_iiffff, dynCall_iiiiii, dynCall_iiffffii, dynCall_iiffffffffffi, dynCall_iiffif, dynCall_viiiiiiii, dynCall_viiiiii, dynCall_viiiif, dynCall_viiii, dynCall_viii, dynCall_iiiiiii, dynCall_iiiiiiii, dynCall_iiiiiiiiii, dynCall_vii, dynCall_viffiii, dynCall_iifi, dynCall_vif, dynCall_viff, dynCall_iiiif, dynCall_iiiiiiiiiii, dynCall_i, dynCall_iif, dynCall_vifff, dynCall_viiif, dynCall_viiiff, dynCall_viiff, dynCall_iiffffffffi, dynCall_iiiiiifi, dynCall_viiiii, dynCall_iiiffffii, dynCall_iiiiiiiiiiii, dynCall_fii, dynCall_viif, dynCall_iffi, dynCall_iiiiffffii, dynCall_ijii, dynCall_iiiiiiiiiiiiiii, dynCall_iiiiiiiiiiiii, dynCall_dii, dynCall_iid, dynCall_jiji, dynCall_ji, dynCall_vffff, dynCall_vf, dynCall_viiiiiiiii, dynCall_vff, dynCall_vfi, dynCall_viffff, dynCall_vfff, dynCall_viiiiiiiiiii, dynCall_viifi, dynCall_iiij, dynCall_viij, dynCall_idi, dynCall_dd, dynCall_ifi, dynCall_ff, dynCall_vd, dynCall_diiiii, dynCall_iidiiiii, dynCall_jii, _asyncify_start_unwind, _asyncify_stop_unwind, _asyncify_start_rewind, _asyncify_stop_rewind, __indirect_function_table, wasmTable;

function assignWasmExports(wasmExports) {
  _malloc = Module["_malloc"] = wasmExports["malloc"];
  _free = Module["_free"] = wasmExports["free"];
  _PtWeb_PasteChar = Module["_PtWeb_PasteChar"] = wasmExports["PtWeb_PasteChar"];
  _pthread_self = wasmExports["pthread_self"];
  _main = Module["_main"] = wasmExports["__main_argc_argv"];
  _PtNet_Receive = Module["_PtNet_Receive"] = wasmExports["PtNet_Receive"];
  __emscripten_tls_init = wasmExports["_emscripten_tls_init"];
  _emscripten_builtin_memalign = wasmExports["emscripten_builtin_memalign"];
  __emscripten_run_callback_on_thread = wasmExports["_emscripten_run_callback_on_thread"];
  ___funcs_on_exit = wasmExports["__funcs_on_exit"];
  __emscripten_thread_init = wasmExports["_emscripten_thread_init"];
  ___set_thread_state = wasmExports["__set_thread_state"];
  __emscripten_thread_crashed = wasmExports["_emscripten_thread_crashed"];
  _emscripten_stack_get_end = wasmExports["emscripten_stack_get_end"];
  _emscripten_stack_get_base = wasmExports["emscripten_stack_get_base"];
  __emscripten_run_js_on_main_thread_done = wasmExports["_emscripten_run_js_on_main_thread_done"];
  __emscripten_run_js_on_main_thread = wasmExports["_emscripten_run_js_on_main_thread"];
  __emscripten_thread_free_data = wasmExports["_emscripten_thread_free_data"];
  __emscripten_thread_exit = wasmExports["_emscripten_thread_exit"];
  __emscripten_check_mailbox = wasmExports["_emscripten_check_mailbox"];
  _setThrew = wasmExports["setThrew"];
  _emscripten_stack_init = wasmExports["emscripten_stack_init"];
  _emscripten_stack_set_limits = wasmExports["emscripten_stack_set_limits"];
  _emscripten_stack_get_free = wasmExports["emscripten_stack_get_free"];
  __emscripten_stack_restore = wasmExports["_emscripten_stack_restore"];
  __emscripten_stack_alloc = wasmExports["_emscripten_stack_alloc"];
  _emscripten_stack_get_current = wasmExports["emscripten_stack_get_current"];
  __ZN6__asan9FakeStack17AddrIsInFakeStackEm = Module["__ZN6__asan9FakeStack17AddrIsInFakeStackEm"] = wasmExports["_ZN6__asan9FakeStack17AddrIsInFakeStackEm"];
  __ZN6__asan9FakeStack8AllocateEmmm = Module["__ZN6__asan9FakeStack8AllocateEmmm"] = wasmExports["_ZN6__asan9FakeStack8AllocateEmmm"];
  ___asan_loadN = wasmExports["__asan_loadN"];
  ___asan_storeN = wasmExports["__asan_storeN"];
  ___lsan_disable = wasmExports["__lsan_disable"];
  ___lsan_enable = wasmExports["__lsan_enable"];
  ___set_stack_limits = Module["___set_stack_limits"] = wasmExports["__set_stack_limits"];
  dynCall_vi = dynCalls["vi"] = wasmExports["dynCall_vi"];
  dynCall_ii = dynCalls["ii"] = wasmExports["dynCall_ii"];
  dynCall_v = dynCalls["v"] = wasmExports["dynCall_v"];
  dynCall_viiiiiii = dynCalls["viiiiiii"] = wasmExports["dynCall_viiiiiii"];
  dynCall_iii = dynCalls["iii"] = wasmExports["dynCall_iii"];
  dynCall_viiiiiiiiii = dynCalls["viiiiiiiiii"] = wasmExports["dynCall_viiiiiiiiii"];
  dynCall_iiiiiiiii = dynCalls["iiiiiiiii"] = wasmExports["dynCall_iiiiiiiii"];
  dynCall_iiiii = dynCalls["iiiii"] = wasmExports["dynCall_iiiii"];
  dynCall_iiii = dynCalls["iiii"] = wasmExports["dynCall_iiii"];
  dynCall_iiffff = dynCalls["iiffff"] = wasmExports["dynCall_iiffff"];
  dynCall_iiiiii = dynCalls["iiiiii"] = wasmExports["dynCall_iiiiii"];
  dynCall_iiffffii = dynCalls["iiffffii"] = wasmExports["dynCall_iiffffii"];
  dynCall_iiffffffffffi = dynCalls["iiffffffffffi"] = wasmExports["dynCall_iiffffffffffi"];
  dynCall_iiffif = dynCalls["iiffif"] = wasmExports["dynCall_iiffif"];
  dynCall_viiiiiiii = dynCalls["viiiiiiii"] = wasmExports["dynCall_viiiiiiii"];
  dynCall_viiiiii = dynCalls["viiiiii"] = wasmExports["dynCall_viiiiii"];
  dynCall_viiiif = dynCalls["viiiif"] = wasmExports["dynCall_viiiif"];
  dynCall_viiii = dynCalls["viiii"] = wasmExports["dynCall_viiii"];
  dynCall_viii = dynCalls["viii"] = wasmExports["dynCall_viii"];
  dynCall_iiiiiii = dynCalls["iiiiiii"] = wasmExports["dynCall_iiiiiii"];
  dynCall_iiiiiiii = dynCalls["iiiiiiii"] = wasmExports["dynCall_iiiiiiii"];
  dynCall_iiiiiiiiii = dynCalls["iiiiiiiiii"] = wasmExports["dynCall_iiiiiiiiii"];
  dynCall_vii = dynCalls["vii"] = wasmExports["dynCall_vii"];
  dynCall_viffiii = dynCalls["viffiii"] = wasmExports["dynCall_viffiii"];
  dynCall_iifi = dynCalls["iifi"] = wasmExports["dynCall_iifi"];
  dynCall_vif = dynCalls["vif"] = wasmExports["dynCall_vif"];
  dynCall_viff = dynCalls["viff"] = wasmExports["dynCall_viff"];
  dynCall_iiiif = dynCalls["iiiif"] = wasmExports["dynCall_iiiif"];
  dynCall_iiiiiiiiiii = dynCalls["iiiiiiiiiii"] = wasmExports["dynCall_iiiiiiiiiii"];
  dynCall_i = dynCalls["i"] = wasmExports["dynCall_i"];
  dynCall_iif = dynCalls["iif"] = wasmExports["dynCall_iif"];
  dynCall_vifff = dynCalls["vifff"] = wasmExports["dynCall_vifff"];
  dynCall_viiif = dynCalls["viiif"] = wasmExports["dynCall_viiif"];
  dynCall_viiiff = dynCalls["viiiff"] = wasmExports["dynCall_viiiff"];
  dynCall_viiff = dynCalls["viiff"] = wasmExports["dynCall_viiff"];
  dynCall_iiffffffffi = dynCalls["iiffffffffi"] = wasmExports["dynCall_iiffffffffi"];
  dynCall_iiiiiifi = dynCalls["iiiiiifi"] = wasmExports["dynCall_iiiiiifi"];
  dynCall_viiiii = dynCalls["viiiii"] = wasmExports["dynCall_viiiii"];
  dynCall_iiiffffii = dynCalls["iiiffffii"] = wasmExports["dynCall_iiiffffii"];
  dynCall_iiiiiiiiiiii = dynCalls["iiiiiiiiiiii"] = wasmExports["dynCall_iiiiiiiiiiii"];
  dynCall_fii = dynCalls["fii"] = wasmExports["dynCall_fii"];
  dynCall_viif = dynCalls["viif"] = wasmExports["dynCall_viif"];
  dynCall_iffi = dynCalls["iffi"] = wasmExports["dynCall_iffi"];
  dynCall_iiiiffffii = dynCalls["iiiiffffii"] = wasmExports["dynCall_iiiiffffii"];
  dynCall_ijii = dynCalls["ijii"] = wasmExports["dynCall_ijii"];
  dynCall_iiiiiiiiiiiiiii = dynCalls["iiiiiiiiiiiiiii"] = wasmExports["dynCall_iiiiiiiiiiiiiii"];
  dynCall_iiiiiiiiiiiii = dynCalls["iiiiiiiiiiiii"] = wasmExports["dynCall_iiiiiiiiiiiii"];
  dynCall_dii = dynCalls["dii"] = wasmExports["dynCall_dii"];
  dynCall_iid = dynCalls["iid"] = wasmExports["dynCall_iid"];
  dynCall_jiji = dynCalls["jiji"] = wasmExports["dynCall_jiji"];
  dynCall_ji = dynCalls["ji"] = wasmExports["dynCall_ji"];
  dynCall_vffff = dynCalls["vffff"] = wasmExports["dynCall_vffff"];
  dynCall_vf = dynCalls["vf"] = wasmExports["dynCall_vf"];
  dynCall_viiiiiiiii = dynCalls["viiiiiiiii"] = wasmExports["dynCall_viiiiiiiii"];
  dynCall_vff = dynCalls["vff"] = wasmExports["dynCall_vff"];
  dynCall_vfi = dynCalls["vfi"] = wasmExports["dynCall_vfi"];
  dynCall_viffff = dynCalls["viffff"] = wasmExports["dynCall_viffff"];
  dynCall_vfff = dynCalls["vfff"] = wasmExports["dynCall_vfff"];
  dynCall_viiiiiiiiiii = dynCalls["viiiiiiiiiii"] = wasmExports["dynCall_viiiiiiiiiii"];
  dynCall_viifi = dynCalls["viifi"] = wasmExports["dynCall_viifi"];
  dynCall_iiij = dynCalls["iiij"] = wasmExports["dynCall_iiij"];
  dynCall_viij = dynCalls["viij"] = wasmExports["dynCall_viij"];
  dynCall_idi = dynCalls["idi"] = wasmExports["dynCall_idi"];
  dynCall_dd = dynCalls["dd"] = wasmExports["dynCall_dd"];
  dynCall_ifi = dynCalls["ifi"] = wasmExports["dynCall_ifi"];
  dynCall_ff = dynCalls["ff"] = wasmExports["dynCall_ff"];
  dynCall_vd = dynCalls["vd"] = wasmExports["dynCall_vd"];
  dynCall_diiiii = dynCalls["diiiii"] = wasmExports["dynCall_diiiii"];
  dynCall_iidiiiii = dynCalls["iidiiiii"] = wasmExports["dynCall_iidiiiii"];
  dynCall_jii = dynCalls["jii"] = wasmExports["dynCall_jii"];
  _asyncify_start_unwind = wasmExports["asyncify_start_unwind"];
  _asyncify_stop_unwind = wasmExports["asyncify_stop_unwind"];
  _asyncify_start_rewind = wasmExports["asyncify_start_rewind"];
  _asyncify_stop_rewind = wasmExports["asyncify_stop_rewind"];
  __indirect_function_table = wasmTable = wasmExports["__indirect_function_table"];
}

var wasmImports;

function assignWasmImports() {
  wasmImports = {
    /** @export */ PtFs_DataUrl,
    /** @export */ PtFs_FetchSync,
    /** @export */ PtFs_NoteWait,
    /** @export */ PtNet_DebugFlag,
    /** @export */ PtNet_IsServerInstance,
    /** @export */ PtSound_HaveAudio,
    /** @export */ PtWeb_IsServerJS,
    /** @export */ PtWeb_PrefetchHint,
    /** @export */ PtWeb_RemoteServerJS,
    /** @export */ PtWeb_SendCrashReport,
    /** @export */ PtWeb_StartServerJS,
    /** @export */ PtWin_Alert,
    /** @export */ PtWin_DecodeCodePage,
    /** @export */ __assert_fail: ___assert_fail,
    /** @export */ __asyncjs__PtFs_FetchAsync,
    /** @export */ __call_sighandler: ___call_sighandler,
    /** @export */ __cxa_throw: ___cxa_throw,
    /** @export */ __handle_stack_overflow: ___handle_stack_overflow,
    /** @export */ __pthread_create_js: ___pthread_create_js,
    /** @export */ __syscall_chdir: ___syscall_chdir,
    /** @export */ __syscall_dup: ___syscall_dup,
    /** @export */ __syscall_faccessat: ___syscall_faccessat,
    /** @export */ __syscall_fcntl64: ___syscall_fcntl64,
    /** @export */ __syscall_fstat64: ___syscall_fstat64,
    /** @export */ __syscall_getcwd: ___syscall_getcwd,
    /** @export */ __syscall_getdents64: ___syscall_getdents64,
    /** @export */ __syscall_ioctl: ___syscall_ioctl,
    /** @export */ __syscall_lstat64: ___syscall_lstat64,
    /** @export */ __syscall_mkdirat: ___syscall_mkdirat,
    /** @export */ __syscall_newfstatat: ___syscall_newfstatat,
    /** @export */ __syscall_openat: ___syscall_openat,
    /** @export */ __syscall_renameat: ___syscall_renameat,
    /** @export */ __syscall_rmdir: ___syscall_rmdir,
    /** @export */ __syscall_stat64: ___syscall_stat64,
    /** @export */ __syscall_unlinkat: ___syscall_unlinkat,
    /** @export */ _abort_js: __abort_js,
    /** @export */ _emscripten_get_progname: __emscripten_get_progname,
    /** @export */ _emscripten_init_main_thread_js: __emscripten_init_main_thread_js,
    /** @export */ _emscripten_log_formatted: __emscripten_log_formatted,
    /** @export */ _emscripten_notify_mailbox_postmessage: __emscripten_notify_mailbox_postmessage,
    /** @export */ _emscripten_receive_on_main_thread_js: __emscripten_receive_on_main_thread_js,
    /** @export */ _emscripten_runtime_keepalive_clear: __emscripten_runtime_keepalive_clear,
    /** @export */ _emscripten_sanitizer_get_option: __emscripten_sanitizer_get_option,
    /** @export */ _emscripten_sanitizer_use_colors: __emscripten_sanitizer_use_colors,
    /** @export */ _emscripten_system: __emscripten_system,
    /** @export */ _emscripten_thread_cleanup: __emscripten_thread_cleanup,
    /** @export */ _emscripten_thread_mailbox_await: __emscripten_thread_mailbox_await,
    /** @export */ _emscripten_thread_set_strongref: __emscripten_thread_set_strongref,
    /** @export */ _emscripten_throw_longjmp: __emscripten_throw_longjmp,
    /** @export */ _gmtime_js: __gmtime_js,
    /** @export */ _localtime_js: __localtime_js,
    /** @export */ _mktime_js: __mktime_js,
    /** @export */ _mmap_js: __mmap_js,
    /** @export */ _munmap_js: __munmap_js,
    /** @export */ _tzset_js: __tzset_js,
    /** @export */ clock_time_get: _clock_time_get,
    /** @export */ eglBindAPI: _eglBindAPI,
    /** @export */ eglChooseConfig: _eglChooseConfig,
    /** @export */ eglCreateContext: _eglCreateContext,
    /** @export */ eglCreateWindowSurface: _eglCreateWindowSurface,
    /** @export */ eglDestroyContext: _eglDestroyContext,
    /** @export */ eglDestroySurface: _eglDestroySurface,
    /** @export */ eglGetConfigAttrib: _eglGetConfigAttrib,
    /** @export */ eglGetDisplay: _eglGetDisplay,
    /** @export */ eglGetError: _eglGetError,
    /** @export */ eglInitialize: _eglInitialize,
    /** @export */ eglMakeCurrent: _eglMakeCurrent,
    /** @export */ eglQueryString: _eglQueryString,
    /** @export */ eglSwapBuffers: _eglSwapBuffers,
    /** @export */ eglSwapInterval: _eglSwapInterval,
    /** @export */ eglTerminate: _eglTerminate,
    /** @export */ eglWaitGL: _eglWaitGL,
    /** @export */ eglWaitNative: _eglWaitNative,
    /** @export */ emscripten_asm_const_async_on_main_thread: _emscripten_asm_const_async_on_main_thread,
    /** @export */ emscripten_asm_const_double: _emscripten_asm_const_double,
    /** @export */ emscripten_asm_const_int: _emscripten_asm_const_int,
    /** @export */ emscripten_asm_const_int_sync_on_main_thread: _emscripten_asm_const_int_sync_on_main_thread,
    /** @export */ emscripten_asm_const_ptr_sync_on_main_thread: _emscripten_asm_const_ptr_sync_on_main_thread,
    /** @export */ emscripten_check_blocking_allowed: _emscripten_check_blocking_allowed,
    /** @export */ emscripten_date_now: _emscripten_date_now,
    /** @export */ emscripten_exit_fullscreen: _emscripten_exit_fullscreen,
    /** @export */ emscripten_exit_pointerlock: _emscripten_exit_pointerlock,
    /** @export */ emscripten_exit_with_live_runtime: _emscripten_exit_with_live_runtime,
    /** @export */ emscripten_force_exit: _emscripten_force_exit,
    /** @export */ emscripten_get_callstack: _emscripten_get_callstack,
    /** @export */ emscripten_get_device_pixel_ratio: _emscripten_get_device_pixel_ratio,
    /** @export */ emscripten_get_element_css_size: _emscripten_get_element_css_size,
    /** @export */ emscripten_get_gamepad_status: _emscripten_get_gamepad_status,
    /** @export */ emscripten_get_heap_max: _emscripten_get_heap_max,
    /** @export */ emscripten_get_now: _emscripten_get_now,
    /** @export */ emscripten_get_num_gamepads: _emscripten_get_num_gamepads,
    /** @export */ emscripten_get_screen_size: _emscripten_get_screen_size,
    /** @export */ emscripten_glActiveTexture: _emscripten_glActiveTexture,
    /** @export */ emscripten_glAttachShader: _emscripten_glAttachShader,
    /** @export */ emscripten_glBeginQuery: _emscripten_glBeginQuery,
    /** @export */ emscripten_glBeginQueryEXT: _emscripten_glBeginQueryEXT,
    /** @export */ emscripten_glBeginTransformFeedback: _emscripten_glBeginTransformFeedback,
    /** @export */ emscripten_glBindAttribLocation: _emscripten_glBindAttribLocation,
    /** @export */ emscripten_glBindBuffer: _emscripten_glBindBuffer,
    /** @export */ emscripten_glBindBufferBase: _emscripten_glBindBufferBase,
    /** @export */ emscripten_glBindBufferRange: _emscripten_glBindBufferRange,
    /** @export */ emscripten_glBindFramebuffer: _emscripten_glBindFramebuffer,
    /** @export */ emscripten_glBindRenderbuffer: _emscripten_glBindRenderbuffer,
    /** @export */ emscripten_glBindSampler: _emscripten_glBindSampler,
    /** @export */ emscripten_glBindTexture: _emscripten_glBindTexture,
    /** @export */ emscripten_glBindTransformFeedback: _emscripten_glBindTransformFeedback,
    /** @export */ emscripten_glBindVertexArray: _emscripten_glBindVertexArray,
    /** @export */ emscripten_glBindVertexArrayOES: _emscripten_glBindVertexArrayOES,
    /** @export */ emscripten_glBlendColor: _emscripten_glBlendColor,
    /** @export */ emscripten_glBlendEquation: _emscripten_glBlendEquation,
    /** @export */ emscripten_glBlendEquationSeparate: _emscripten_glBlendEquationSeparate,
    /** @export */ emscripten_glBlendFunc: _emscripten_glBlendFunc,
    /** @export */ emscripten_glBlendFuncSeparate: _emscripten_glBlendFuncSeparate,
    /** @export */ emscripten_glBlitFramebuffer: _emscripten_glBlitFramebuffer,
    /** @export */ emscripten_glBufferData: _emscripten_glBufferData,
    /** @export */ emscripten_glBufferSubData: _emscripten_glBufferSubData,
    /** @export */ emscripten_glCheckFramebufferStatus: _emscripten_glCheckFramebufferStatus,
    /** @export */ emscripten_glClear: _emscripten_glClear,
    /** @export */ emscripten_glClearBufferfi: _emscripten_glClearBufferfi,
    /** @export */ emscripten_glClearBufferfv: _emscripten_glClearBufferfv,
    /** @export */ emscripten_glClearBufferiv: _emscripten_glClearBufferiv,
    /** @export */ emscripten_glClearBufferuiv: _emscripten_glClearBufferuiv,
    /** @export */ emscripten_glClearColor: _emscripten_glClearColor,
    /** @export */ emscripten_glClearDepthf: _emscripten_glClearDepthf,
    /** @export */ emscripten_glClearStencil: _emscripten_glClearStencil,
    /** @export */ emscripten_glClientWaitSync: _emscripten_glClientWaitSync,
    /** @export */ emscripten_glClipControlEXT: _emscripten_glClipControlEXT,
    /** @export */ emscripten_glColorMask: _emscripten_glColorMask,
    /** @export */ emscripten_glCompileShader: _emscripten_glCompileShader,
    /** @export */ emscripten_glCompressedTexImage2D: _emscripten_glCompressedTexImage2D,
    /** @export */ emscripten_glCompressedTexImage3D: _emscripten_glCompressedTexImage3D,
    /** @export */ emscripten_glCompressedTexSubImage2D: _emscripten_glCompressedTexSubImage2D,
    /** @export */ emscripten_glCompressedTexSubImage3D: _emscripten_glCompressedTexSubImage3D,
    /** @export */ emscripten_glCopyBufferSubData: _emscripten_glCopyBufferSubData,
    /** @export */ emscripten_glCopyTexImage2D: _emscripten_glCopyTexImage2D,
    /** @export */ emscripten_glCopyTexSubImage2D: _emscripten_glCopyTexSubImage2D,
    /** @export */ emscripten_glCopyTexSubImage3D: _emscripten_glCopyTexSubImage3D,
    /** @export */ emscripten_glCreateProgram: _emscripten_glCreateProgram,
    /** @export */ emscripten_glCreateShader: _emscripten_glCreateShader,
    /** @export */ emscripten_glCullFace: _emscripten_glCullFace,
    /** @export */ emscripten_glDeleteBuffers: _emscripten_glDeleteBuffers,
    /** @export */ emscripten_glDeleteFramebuffers: _emscripten_glDeleteFramebuffers,
    /** @export */ emscripten_glDeleteProgram: _emscripten_glDeleteProgram,
    /** @export */ emscripten_glDeleteQueries: _emscripten_glDeleteQueries,
    /** @export */ emscripten_glDeleteQueriesEXT: _emscripten_glDeleteQueriesEXT,
    /** @export */ emscripten_glDeleteRenderbuffers: _emscripten_glDeleteRenderbuffers,
    /** @export */ emscripten_glDeleteSamplers: _emscripten_glDeleteSamplers,
    /** @export */ emscripten_glDeleteShader: _emscripten_glDeleteShader,
    /** @export */ emscripten_glDeleteSync: _emscripten_glDeleteSync,
    /** @export */ emscripten_glDeleteTextures: _emscripten_glDeleteTextures,
    /** @export */ emscripten_glDeleteTransformFeedbacks: _emscripten_glDeleteTransformFeedbacks,
    /** @export */ emscripten_glDeleteVertexArrays: _emscripten_glDeleteVertexArrays,
    /** @export */ emscripten_glDeleteVertexArraysOES: _emscripten_glDeleteVertexArraysOES,
    /** @export */ emscripten_glDepthFunc: _emscripten_glDepthFunc,
    /** @export */ emscripten_glDepthMask: _emscripten_glDepthMask,
    /** @export */ emscripten_glDepthRangef: _emscripten_glDepthRangef,
    /** @export */ emscripten_glDetachShader: _emscripten_glDetachShader,
    /** @export */ emscripten_glDisable: _emscripten_glDisable,
    /** @export */ emscripten_glDisableVertexAttribArray: _emscripten_glDisableVertexAttribArray,
    /** @export */ emscripten_glDrawArrays: _emscripten_glDrawArrays,
    /** @export */ emscripten_glDrawArraysInstanced: _emscripten_glDrawArraysInstanced,
    /** @export */ emscripten_glDrawArraysInstancedANGLE: _emscripten_glDrawArraysInstancedANGLE,
    /** @export */ emscripten_glDrawArraysInstancedARB: _emscripten_glDrawArraysInstancedARB,
    /** @export */ emscripten_glDrawArraysInstancedEXT: _emscripten_glDrawArraysInstancedEXT,
    /** @export */ emscripten_glDrawArraysInstancedNV: _emscripten_glDrawArraysInstancedNV,
    /** @export */ emscripten_glDrawBuffers: _emscripten_glDrawBuffers,
    /** @export */ emscripten_glDrawBuffersEXT: _emscripten_glDrawBuffersEXT,
    /** @export */ emscripten_glDrawBuffersWEBGL: _emscripten_glDrawBuffersWEBGL,
    /** @export */ emscripten_glDrawElements: _emscripten_glDrawElements,
    /** @export */ emscripten_glDrawElementsInstanced: _emscripten_glDrawElementsInstanced,
    /** @export */ emscripten_glDrawElementsInstancedANGLE: _emscripten_glDrawElementsInstancedANGLE,
    /** @export */ emscripten_glDrawElementsInstancedARB: _emscripten_glDrawElementsInstancedARB,
    /** @export */ emscripten_glDrawElementsInstancedEXT: _emscripten_glDrawElementsInstancedEXT,
    /** @export */ emscripten_glDrawElementsInstancedNV: _emscripten_glDrawElementsInstancedNV,
    /** @export */ emscripten_glDrawRangeElements: _emscripten_glDrawRangeElements,
    /** @export */ emscripten_glEnable: _emscripten_glEnable,
    /** @export */ emscripten_glEnableVertexAttribArray: _emscripten_glEnableVertexAttribArray,
    /** @export */ emscripten_glEndQuery: _emscripten_glEndQuery,
    /** @export */ emscripten_glEndQueryEXT: _emscripten_glEndQueryEXT,
    /** @export */ emscripten_glEndTransformFeedback: _emscripten_glEndTransformFeedback,
    /** @export */ emscripten_glFenceSync: _emscripten_glFenceSync,
    /** @export */ emscripten_glFinish: _emscripten_glFinish,
    /** @export */ emscripten_glFlush: _emscripten_glFlush,
    /** @export */ emscripten_glFlushMappedBufferRange: _emscripten_glFlushMappedBufferRange,
    /** @export */ emscripten_glFramebufferRenderbuffer: _emscripten_glFramebufferRenderbuffer,
    /** @export */ emscripten_glFramebufferTexture2D: _emscripten_glFramebufferTexture2D,
    /** @export */ emscripten_glFramebufferTextureLayer: _emscripten_glFramebufferTextureLayer,
    /** @export */ emscripten_glFrontFace: _emscripten_glFrontFace,
    /** @export */ emscripten_glGenBuffers: _emscripten_glGenBuffers,
    /** @export */ emscripten_glGenFramebuffers: _emscripten_glGenFramebuffers,
    /** @export */ emscripten_glGenQueries: _emscripten_glGenQueries,
    /** @export */ emscripten_glGenQueriesEXT: _emscripten_glGenQueriesEXT,
    /** @export */ emscripten_glGenRenderbuffers: _emscripten_glGenRenderbuffers,
    /** @export */ emscripten_glGenSamplers: _emscripten_glGenSamplers,
    /** @export */ emscripten_glGenTextures: _emscripten_glGenTextures,
    /** @export */ emscripten_glGenTransformFeedbacks: _emscripten_glGenTransformFeedbacks,
    /** @export */ emscripten_glGenVertexArrays: _emscripten_glGenVertexArrays,
    /** @export */ emscripten_glGenVertexArraysOES: _emscripten_glGenVertexArraysOES,
    /** @export */ emscripten_glGenerateMipmap: _emscripten_glGenerateMipmap,
    /** @export */ emscripten_glGetActiveAttrib: _emscripten_glGetActiveAttrib,
    /** @export */ emscripten_glGetActiveUniform: _emscripten_glGetActiveUniform,
    /** @export */ emscripten_glGetActiveUniformBlockName: _emscripten_glGetActiveUniformBlockName,
    /** @export */ emscripten_glGetActiveUniformBlockiv: _emscripten_glGetActiveUniformBlockiv,
    /** @export */ emscripten_glGetActiveUniformsiv: _emscripten_glGetActiveUniformsiv,
    /** @export */ emscripten_glGetAttachedShaders: _emscripten_glGetAttachedShaders,
    /** @export */ emscripten_glGetAttribLocation: _emscripten_glGetAttribLocation,
    /** @export */ emscripten_glGetBooleanv: _emscripten_glGetBooleanv,
    /** @export */ emscripten_glGetBufferParameteri64v: _emscripten_glGetBufferParameteri64v,
    /** @export */ emscripten_glGetBufferParameteriv: _emscripten_glGetBufferParameteriv,
    /** @export */ emscripten_glGetBufferPointerv: _emscripten_glGetBufferPointerv,
    /** @export */ emscripten_glGetError: _emscripten_glGetError,
    /** @export */ emscripten_glGetFloatv: _emscripten_glGetFloatv,
    /** @export */ emscripten_glGetFragDataLocation: _emscripten_glGetFragDataLocation,
    /** @export */ emscripten_glGetFramebufferAttachmentParameteriv: _emscripten_glGetFramebufferAttachmentParameteriv,
    /** @export */ emscripten_glGetInteger64i_v: _emscripten_glGetInteger64i_v,
    /** @export */ emscripten_glGetInteger64v: _emscripten_glGetInteger64v,
    /** @export */ emscripten_glGetIntegeri_v: _emscripten_glGetIntegeri_v,
    /** @export */ emscripten_glGetIntegerv: _emscripten_glGetIntegerv,
    /** @export */ emscripten_glGetInternalformativ: _emscripten_glGetInternalformativ,
    /** @export */ emscripten_glGetProgramBinary: _emscripten_glGetProgramBinary,
    /** @export */ emscripten_glGetProgramInfoLog: _emscripten_glGetProgramInfoLog,
    /** @export */ emscripten_glGetProgramiv: _emscripten_glGetProgramiv,
    /** @export */ emscripten_glGetQueryObjecti64vEXT: _emscripten_glGetQueryObjecti64vEXT,
    /** @export */ emscripten_glGetQueryObjectivEXT: _emscripten_glGetQueryObjectivEXT,
    /** @export */ emscripten_glGetQueryObjectui64vEXT: _emscripten_glGetQueryObjectui64vEXT,
    /** @export */ emscripten_glGetQueryObjectuiv: _emscripten_glGetQueryObjectuiv,
    /** @export */ emscripten_glGetQueryObjectuivEXT: _emscripten_glGetQueryObjectuivEXT,
    /** @export */ emscripten_glGetQueryiv: _emscripten_glGetQueryiv,
    /** @export */ emscripten_glGetQueryivEXT: _emscripten_glGetQueryivEXT,
    /** @export */ emscripten_glGetRenderbufferParameteriv: _emscripten_glGetRenderbufferParameteriv,
    /** @export */ emscripten_glGetSamplerParameterfv: _emscripten_glGetSamplerParameterfv,
    /** @export */ emscripten_glGetSamplerParameteriv: _emscripten_glGetSamplerParameteriv,
    /** @export */ emscripten_glGetShaderInfoLog: _emscripten_glGetShaderInfoLog,
    /** @export */ emscripten_glGetShaderPrecisionFormat: _emscripten_glGetShaderPrecisionFormat,
    /** @export */ emscripten_glGetShaderSource: _emscripten_glGetShaderSource,
    /** @export */ emscripten_glGetShaderiv: _emscripten_glGetShaderiv,
    /** @export */ emscripten_glGetString: _emscripten_glGetString,
    /** @export */ emscripten_glGetStringi: _emscripten_glGetStringi,
    /** @export */ emscripten_glGetSynciv: _emscripten_glGetSynciv,
    /** @export */ emscripten_glGetTexParameterfv: _emscripten_glGetTexParameterfv,
    /** @export */ emscripten_glGetTexParameteriv: _emscripten_glGetTexParameteriv,
    /** @export */ emscripten_glGetTransformFeedbackVarying: _emscripten_glGetTransformFeedbackVarying,
    /** @export */ emscripten_glGetUniformBlockIndex: _emscripten_glGetUniformBlockIndex,
    /** @export */ emscripten_glGetUniformIndices: _emscripten_glGetUniformIndices,
    /** @export */ emscripten_glGetUniformLocation: _emscripten_glGetUniformLocation,
    /** @export */ emscripten_glGetUniformfv: _emscripten_glGetUniformfv,
    /** @export */ emscripten_glGetUniformiv: _emscripten_glGetUniformiv,
    /** @export */ emscripten_glGetUniformuiv: _emscripten_glGetUniformuiv,
    /** @export */ emscripten_glGetVertexAttribIiv: _emscripten_glGetVertexAttribIiv,
    /** @export */ emscripten_glGetVertexAttribIuiv: _emscripten_glGetVertexAttribIuiv,
    /** @export */ emscripten_glGetVertexAttribPointerv: _emscripten_glGetVertexAttribPointerv,
    /** @export */ emscripten_glGetVertexAttribfv: _emscripten_glGetVertexAttribfv,
    /** @export */ emscripten_glGetVertexAttribiv: _emscripten_glGetVertexAttribiv,
    /** @export */ emscripten_glHint: _emscripten_glHint,
    /** @export */ emscripten_glInvalidateFramebuffer: _emscripten_glInvalidateFramebuffer,
    /** @export */ emscripten_glInvalidateSubFramebuffer: _emscripten_glInvalidateSubFramebuffer,
    /** @export */ emscripten_glIsBuffer: _emscripten_glIsBuffer,
    /** @export */ emscripten_glIsEnabled: _emscripten_glIsEnabled,
    /** @export */ emscripten_glIsFramebuffer: _emscripten_glIsFramebuffer,
    /** @export */ emscripten_glIsProgram: _emscripten_glIsProgram,
    /** @export */ emscripten_glIsQuery: _emscripten_glIsQuery,
    /** @export */ emscripten_glIsQueryEXT: _emscripten_glIsQueryEXT,
    /** @export */ emscripten_glIsRenderbuffer: _emscripten_glIsRenderbuffer,
    /** @export */ emscripten_glIsSampler: _emscripten_glIsSampler,
    /** @export */ emscripten_glIsShader: _emscripten_glIsShader,
    /** @export */ emscripten_glIsSync: _emscripten_glIsSync,
    /** @export */ emscripten_glIsTexture: _emscripten_glIsTexture,
    /** @export */ emscripten_glIsTransformFeedback: _emscripten_glIsTransformFeedback,
    /** @export */ emscripten_glIsVertexArray: _emscripten_glIsVertexArray,
    /** @export */ emscripten_glIsVertexArrayOES: _emscripten_glIsVertexArrayOES,
    /** @export */ emscripten_glLineWidth: _emscripten_glLineWidth,
    /** @export */ emscripten_glLinkProgram: _emscripten_glLinkProgram,
    /** @export */ emscripten_glMapBufferRange: _emscripten_glMapBufferRange,
    /** @export */ emscripten_glPauseTransformFeedback: _emscripten_glPauseTransformFeedback,
    /** @export */ emscripten_glPixelStorei: _emscripten_glPixelStorei,
    /** @export */ emscripten_glPolygonModeWEBGL: _emscripten_glPolygonModeWEBGL,
    /** @export */ emscripten_glPolygonOffset: _emscripten_glPolygonOffset,
    /** @export */ emscripten_glPolygonOffsetClampEXT: _emscripten_glPolygonOffsetClampEXT,
    /** @export */ emscripten_glProgramBinary: _emscripten_glProgramBinary,
    /** @export */ emscripten_glProgramParameteri: _emscripten_glProgramParameteri,
    /** @export */ emscripten_glQueryCounterEXT: _emscripten_glQueryCounterEXT,
    /** @export */ emscripten_glReadBuffer: _emscripten_glReadBuffer,
    /** @export */ emscripten_glReadPixels: _emscripten_glReadPixels,
    /** @export */ emscripten_glReleaseShaderCompiler: _emscripten_glReleaseShaderCompiler,
    /** @export */ emscripten_glRenderbufferStorage: _emscripten_glRenderbufferStorage,
    /** @export */ emscripten_glRenderbufferStorageMultisample: _emscripten_glRenderbufferStorageMultisample,
    /** @export */ emscripten_glResumeTransformFeedback: _emscripten_glResumeTransformFeedback,
    /** @export */ emscripten_glSampleCoverage: _emscripten_glSampleCoverage,
    /** @export */ emscripten_glSamplerParameterf: _emscripten_glSamplerParameterf,
    /** @export */ emscripten_glSamplerParameterfv: _emscripten_glSamplerParameterfv,
    /** @export */ emscripten_glSamplerParameteri: _emscripten_glSamplerParameteri,
    /** @export */ emscripten_glSamplerParameteriv: _emscripten_glSamplerParameteriv,
    /** @export */ emscripten_glScissor: _emscripten_glScissor,
    /** @export */ emscripten_glShaderBinary: _emscripten_glShaderBinary,
    /** @export */ emscripten_glShaderSource: _emscripten_glShaderSource,
    /** @export */ emscripten_glStencilFunc: _emscripten_glStencilFunc,
    /** @export */ emscripten_glStencilFuncSeparate: _emscripten_glStencilFuncSeparate,
    /** @export */ emscripten_glStencilMask: _emscripten_glStencilMask,
    /** @export */ emscripten_glStencilMaskSeparate: _emscripten_glStencilMaskSeparate,
    /** @export */ emscripten_glStencilOp: _emscripten_glStencilOp,
    /** @export */ emscripten_glStencilOpSeparate: _emscripten_glStencilOpSeparate,
    /** @export */ emscripten_glTexImage2D: _emscripten_glTexImage2D,
    /** @export */ emscripten_glTexImage3D: _emscripten_glTexImage3D,
    /** @export */ emscripten_glTexParameterf: _emscripten_glTexParameterf,
    /** @export */ emscripten_glTexParameterfv: _emscripten_glTexParameterfv,
    /** @export */ emscripten_glTexParameteri: _emscripten_glTexParameteri,
    /** @export */ emscripten_glTexParameteriv: _emscripten_glTexParameteriv,
    /** @export */ emscripten_glTexStorage2D: _emscripten_glTexStorage2D,
    /** @export */ emscripten_glTexStorage3D: _emscripten_glTexStorage3D,
    /** @export */ emscripten_glTexSubImage2D: _emscripten_glTexSubImage2D,
    /** @export */ emscripten_glTexSubImage3D: _emscripten_glTexSubImage3D,
    /** @export */ emscripten_glTransformFeedbackVaryings: _emscripten_glTransformFeedbackVaryings,
    /** @export */ emscripten_glUniform1f: _emscripten_glUniform1f,
    /** @export */ emscripten_glUniform1fv: _emscripten_glUniform1fv,
    /** @export */ emscripten_glUniform1i: _emscripten_glUniform1i,
    /** @export */ emscripten_glUniform1iv: _emscripten_glUniform1iv,
    /** @export */ emscripten_glUniform1ui: _emscripten_glUniform1ui,
    /** @export */ emscripten_glUniform1uiv: _emscripten_glUniform1uiv,
    /** @export */ emscripten_glUniform2f: _emscripten_glUniform2f,
    /** @export */ emscripten_glUniform2fv: _emscripten_glUniform2fv,
    /** @export */ emscripten_glUniform2i: _emscripten_glUniform2i,
    /** @export */ emscripten_glUniform2iv: _emscripten_glUniform2iv,
    /** @export */ emscripten_glUniform2ui: _emscripten_glUniform2ui,
    /** @export */ emscripten_glUniform2uiv: _emscripten_glUniform2uiv,
    /** @export */ emscripten_glUniform3f: _emscripten_glUniform3f,
    /** @export */ emscripten_glUniform3fv: _emscripten_glUniform3fv,
    /** @export */ emscripten_glUniform3i: _emscripten_glUniform3i,
    /** @export */ emscripten_glUniform3iv: _emscripten_glUniform3iv,
    /** @export */ emscripten_glUniform3ui: _emscripten_glUniform3ui,
    /** @export */ emscripten_glUniform3uiv: _emscripten_glUniform3uiv,
    /** @export */ emscripten_glUniform4f: _emscripten_glUniform4f,
    /** @export */ emscripten_glUniform4fv: _emscripten_glUniform4fv,
    /** @export */ emscripten_glUniform4i: _emscripten_glUniform4i,
    /** @export */ emscripten_glUniform4iv: _emscripten_glUniform4iv,
    /** @export */ emscripten_glUniform4ui: _emscripten_glUniform4ui,
    /** @export */ emscripten_glUniform4uiv: _emscripten_glUniform4uiv,
    /** @export */ emscripten_glUniformBlockBinding: _emscripten_glUniformBlockBinding,
    /** @export */ emscripten_glUniformMatrix2fv: _emscripten_glUniformMatrix2fv,
    /** @export */ emscripten_glUniformMatrix2x3fv: _emscripten_glUniformMatrix2x3fv,
    /** @export */ emscripten_glUniformMatrix2x4fv: _emscripten_glUniformMatrix2x4fv,
    /** @export */ emscripten_glUniformMatrix3fv: _emscripten_glUniformMatrix3fv,
    /** @export */ emscripten_glUniformMatrix3x2fv: _emscripten_glUniformMatrix3x2fv,
    /** @export */ emscripten_glUniformMatrix3x4fv: _emscripten_glUniformMatrix3x4fv,
    /** @export */ emscripten_glUniformMatrix4fv: _emscripten_glUniformMatrix4fv,
    /** @export */ emscripten_glUniformMatrix4x2fv: _emscripten_glUniformMatrix4x2fv,
    /** @export */ emscripten_glUniformMatrix4x3fv: _emscripten_glUniformMatrix4x3fv,
    /** @export */ emscripten_glUnmapBuffer: _emscripten_glUnmapBuffer,
    /** @export */ emscripten_glUseProgram: _emscripten_glUseProgram,
    /** @export */ emscripten_glValidateProgram: _emscripten_glValidateProgram,
    /** @export */ emscripten_glVertexAttrib1f: _emscripten_glVertexAttrib1f,
    /** @export */ emscripten_glVertexAttrib1fv: _emscripten_glVertexAttrib1fv,
    /** @export */ emscripten_glVertexAttrib2f: _emscripten_glVertexAttrib2f,
    /** @export */ emscripten_glVertexAttrib2fv: _emscripten_glVertexAttrib2fv,
    /** @export */ emscripten_glVertexAttrib3f: _emscripten_glVertexAttrib3f,
    /** @export */ emscripten_glVertexAttrib3fv: _emscripten_glVertexAttrib3fv,
    /** @export */ emscripten_glVertexAttrib4f: _emscripten_glVertexAttrib4f,
    /** @export */ emscripten_glVertexAttrib4fv: _emscripten_glVertexAttrib4fv,
    /** @export */ emscripten_glVertexAttribDivisor: _emscripten_glVertexAttribDivisor,
    /** @export */ emscripten_glVertexAttribDivisorANGLE: _emscripten_glVertexAttribDivisorANGLE,
    /** @export */ emscripten_glVertexAttribDivisorARB: _emscripten_glVertexAttribDivisorARB,
    /** @export */ emscripten_glVertexAttribDivisorEXT: _emscripten_glVertexAttribDivisorEXT,
    /** @export */ emscripten_glVertexAttribDivisorNV: _emscripten_glVertexAttribDivisorNV,
    /** @export */ emscripten_glVertexAttribI4i: _emscripten_glVertexAttribI4i,
    /** @export */ emscripten_glVertexAttribI4iv: _emscripten_glVertexAttribI4iv,
    /** @export */ emscripten_glVertexAttribI4ui: _emscripten_glVertexAttribI4ui,
    /** @export */ emscripten_glVertexAttribI4uiv: _emscripten_glVertexAttribI4uiv,
    /** @export */ emscripten_glVertexAttribIPointer: _emscripten_glVertexAttribIPointer,
    /** @export */ emscripten_glVertexAttribPointer: _emscripten_glVertexAttribPointer,
    /** @export */ emscripten_glViewport: _emscripten_glViewport,
    /** @export */ emscripten_glWaitSync: _emscripten_glWaitSync,
    /** @export */ emscripten_has_asyncify: _emscripten_has_asyncify,
    /** @export */ emscripten_num_logical_cores: _emscripten_num_logical_cores,
    /** @export */ emscripten_pc_get_column: _emscripten_pc_get_column,
    /** @export */ emscripten_pc_get_file: _emscripten_pc_get_file,
    /** @export */ emscripten_pc_get_function: _emscripten_pc_get_function,
    /** @export */ emscripten_pc_get_line: _emscripten_pc_get_line,
    /** @export */ emscripten_request_fullscreen_strategy: _emscripten_request_fullscreen_strategy,
    /** @export */ emscripten_request_pointerlock: _emscripten_request_pointerlock,
    /** @export */ emscripten_resize_heap: _emscripten_resize_heap,
    /** @export */ emscripten_return_address: _emscripten_return_address,
    /** @export */ emscripten_sample_gamepad_data: _emscripten_sample_gamepad_data,
    /** @export */ emscripten_set_beforeunload_callback_on_thread: _emscripten_set_beforeunload_callback_on_thread,
    /** @export */ emscripten_set_blur_callback_on_thread: _emscripten_set_blur_callback_on_thread,
    /** @export */ emscripten_set_canvas_element_size: _emscripten_set_canvas_element_size,
    /** @export */ emscripten_set_element_css_size: _emscripten_set_element_css_size,
    /** @export */ emscripten_set_focus_callback_on_thread: _emscripten_set_focus_callback_on_thread,
    /** @export */ emscripten_set_fullscreenchange_callback_on_thread: _emscripten_set_fullscreenchange_callback_on_thread,
    /** @export */ emscripten_set_gamepadconnected_callback_on_thread: _emscripten_set_gamepadconnected_callback_on_thread,
    /** @export */ emscripten_set_gamepaddisconnected_callback_on_thread: _emscripten_set_gamepaddisconnected_callback_on_thread,
    /** @export */ emscripten_set_keydown_callback_on_thread: _emscripten_set_keydown_callback_on_thread,
    /** @export */ emscripten_set_keypress_callback_on_thread: _emscripten_set_keypress_callback_on_thread,
    /** @export */ emscripten_set_keyup_callback_on_thread: _emscripten_set_keyup_callback_on_thread,
    /** @export */ emscripten_set_mousedown_callback_on_thread: _emscripten_set_mousedown_callback_on_thread,
    /** @export */ emscripten_set_mouseenter_callback_on_thread: _emscripten_set_mouseenter_callback_on_thread,
    /** @export */ emscripten_set_mouseleave_callback_on_thread: _emscripten_set_mouseleave_callback_on_thread,
    /** @export */ emscripten_set_mousemove_callback_on_thread: _emscripten_set_mousemove_callback_on_thread,
    /** @export */ emscripten_set_mouseup_callback_on_thread: _emscripten_set_mouseup_callback_on_thread,
    /** @export */ emscripten_set_pointerlockchange_callback_on_thread: _emscripten_set_pointerlockchange_callback_on_thread,
    /** @export */ emscripten_set_resize_callback_on_thread: _emscripten_set_resize_callback_on_thread,
    /** @export */ emscripten_set_touchcancel_callback_on_thread: _emscripten_set_touchcancel_callback_on_thread,
    /** @export */ emscripten_set_touchend_callback_on_thread: _emscripten_set_touchend_callback_on_thread,
    /** @export */ emscripten_set_touchmove_callback_on_thread: _emscripten_set_touchmove_callback_on_thread,
    /** @export */ emscripten_set_touchstart_callback_on_thread: _emscripten_set_touchstart_callback_on_thread,
    /** @export */ emscripten_set_visibilitychange_callback_on_thread: _emscripten_set_visibilitychange_callback_on_thread,
    /** @export */ emscripten_set_wheel_callback_on_thread: _emscripten_set_wheel_callback_on_thread,
    /** @export */ emscripten_set_window_title: _emscripten_set_window_title,
    /** @export */ emscripten_sleep: _emscripten_sleep,
    /** @export */ emscripten_stack_snapshot: _emscripten_stack_snapshot,
    /** @export */ emscripten_stack_unwind_buffer: _emscripten_stack_unwind_buffer,
    /** @export */ emscripten_unwind_to_js_event_loop: _emscripten_unwind_to_js_event_loop,
    /** @export */ environ_get: _environ_get,
    /** @export */ environ_sizes_get: _environ_sizes_get,
    /** @export */ exit: _exit,
    /** @export */ fd_close: _fd_close,
    /** @export */ fd_read: _fd_read,
    /** @export */ fd_seek: _fd_seek,
    /** @export */ fd_write: _fd_write,
    /** @export */ glActiveTexture: _glActiveTexture,
    /** @export */ glAttachShader: _glAttachShader,
    /** @export */ glBindAttribLocation: _glBindAttribLocation,
    /** @export */ glBindBuffer: _glBindBuffer,
    /** @export */ glBindFramebuffer: _glBindFramebuffer,
    /** @export */ glBindRenderbuffer: _glBindRenderbuffer,
    /** @export */ glBindTexture: _glBindTexture,
    /** @export */ glBindVertexArray: _glBindVertexArray,
    /** @export */ glBlendFunc: _glBlendFunc,
    /** @export */ glBlitFramebuffer: _glBlitFramebuffer,
    /** @export */ glBufferData: _glBufferData,
    /** @export */ glCheckFramebufferStatus: _glCheckFramebufferStatus,
    /** @export */ glClear: _glClear,
    /** @export */ glClearColor: _glClearColor,
    /** @export */ glClearDepthf: _glClearDepthf,
    /** @export */ glColorMask: _glColorMask,
    /** @export */ glCompileShader: _glCompileShader,
    /** @export */ glCreateProgram: _glCreateProgram,
    /** @export */ glCreateShader: _glCreateShader,
    /** @export */ glCullFace: _glCullFace,
    /** @export */ glDeleteFramebuffers: _glDeleteFramebuffers,
    /** @export */ glDeleteRenderbuffers: _glDeleteRenderbuffers,
    /** @export */ glDeleteShader: _glDeleteShader,
    /** @export */ glDeleteTextures: _glDeleteTextures,
    /** @export */ glDepthFunc: _glDepthFunc,
    /** @export */ glDepthMask: _glDepthMask,
    /** @export */ glDisable: _glDisable,
    /** @export */ glDisableVertexAttribArray: _glDisableVertexAttribArray,
    /** @export */ glDrawArrays: _glDrawArrays,
    /** @export */ glDrawBuffers: _glDrawBuffers,
    /** @export */ glEnable: _glEnable,
    /** @export */ glEnableVertexAttribArray: _glEnableVertexAttribArray,
    /** @export */ glFramebufferRenderbuffer: _glFramebufferRenderbuffer,
    /** @export */ glFramebufferTexture2D: _glFramebufferTexture2D,
    /** @export */ glFrontFace: _glFrontFace,
    /** @export */ glGenBuffers: _glGenBuffers,
    /** @export */ glGenFramebuffers: _glGenFramebuffers,
    /** @export */ glGenRenderbuffers: _glGenRenderbuffers,
    /** @export */ glGenTextures: _glGenTextures,
    /** @export */ glGenVertexArrays: _glGenVertexArrays,
    /** @export */ glGetError: _glGetError,
    /** @export */ glGetIntegerv: _glGetIntegerv,
    /** @export */ glGetProgramInfoLog: _glGetProgramInfoLog,
    /** @export */ glGetProgramiv: _glGetProgramiv,
    /** @export */ glGetShaderInfoLog: _glGetShaderInfoLog,
    /** @export */ glGetShaderiv: _glGetShaderiv,
    /** @export */ glGetString: _glGetString,
    /** @export */ glGetUniformLocation: _glGetUniformLocation,
    /** @export */ glLinkProgram: _glLinkProgram,
    /** @export */ glPixelStorei: _glPixelStorei,
    /** @export */ glPolygonOffset: _glPolygonOffset,
    /** @export */ glReadBuffer: _glReadBuffer,
    /** @export */ glReadPixels: _glReadPixels,
    /** @export */ glRenderbufferStorage: _glRenderbufferStorage,
    /** @export */ glScissor: _glScissor,
    /** @export */ glShaderSource: _glShaderSource,
    /** @export */ glTexImage2D: _glTexImage2D,
    /** @export */ glTexParameteri: _glTexParameteri,
    /** @export */ glTexSubImage2D: _glTexSubImage2D,
    /** @export */ glUniform1f: _glUniform1f,
    /** @export */ glUniform1i: _glUniform1i,
    /** @export */ glUniform1iv: _glUniform1iv,
    /** @export */ glUniform2f: _glUniform2f,
    /** @export */ glUniform3f: _glUniform3f,
    /** @export */ glUniform4f: _glUniform4f,
    /** @export */ glUniformMatrix3fv: _glUniformMatrix3fv,
    /** @export */ glUseProgram: _glUseProgram,
    /** @export */ glVertexAttrib4f: _glVertexAttrib4f,
    /** @export */ glVertexAttribPointer: _glVertexAttribPointer,
    /** @export */ glViewport: _glViewport,
    /** @export */ invoke_vi,
    /** @export */ invoke_vii,
    /** @export */ memory: wasmMemory,
    /** @export */ proc_exit: _proc_exit
  };
}

function invoke_vi(index, a1) {
  var sp = stackSave();
  try {
    dynCall_vi(index, a1);
  } catch (e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_vii(index, a1, a2) {
  var sp = stackSave();
  try {
    dynCall_vii(index, a1, a2);
  } catch (e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

// Argument name here must shadow the `wasmExports` global so
// that it is recognised by metadce and minify-import-export-names
// passes.
function applySignatureConversions(wasmExports) {
  // First, make a copy of the incoming exports object
  wasmExports = Object.assign({}, wasmExports);
  var makeWrapper_pp = f => a0 => f(a0) >>> 0;
  var makeWrapper_p = f => () => f() >>> 0;
  var makeWrapper_ppp = f => (a0, a1) => f(a0, a1) >>> 0;
  wasmExports["malloc"] = makeWrapper_pp(wasmExports["malloc"]);
  wasmExports["pthread_self"] = makeWrapper_p(wasmExports["pthread_self"]);
  wasmExports["_emscripten_tls_init"] = makeWrapper_p(wasmExports["_emscripten_tls_init"]);
  wasmExports["emscripten_builtin_memalign"] = makeWrapper_ppp(wasmExports["emscripten_builtin_memalign"]);
  wasmExports["emscripten_stack_get_end"] = makeWrapper_p(wasmExports["emscripten_stack_get_end"]);
  wasmExports["emscripten_stack_get_base"] = makeWrapper_p(wasmExports["emscripten_stack_get_base"]);
  wasmExports["emscripten_stack_get_free"] = makeWrapper_p(wasmExports["emscripten_stack_get_free"]);
  wasmExports["_emscripten_stack_alloc"] = makeWrapper_pp(wasmExports["_emscripten_stack_alloc"]);
  wasmExports["emscripten_stack_get_current"] = makeWrapper_p(wasmExports["emscripten_stack_get_current"]);
  return wasmExports;
}

// include: postamble.js
// === Auto-generated postamble setup entry stuff ===
function callMain(args = []) {
  var entryFunction = _main;
  args.unshift(thisProgram);
  var argc = args.length;
  var argv = stackAlloc((argc + 1) * 4);
  var argv_ptr = argv;
  for (var arg of args) {
    (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((argv_ptr) >>> 2) >>> 0, ___asan_storeN)] = stringToUTF8OnStack(arg);
    argv_ptr += 4;
  }
  (growMemViews(), HEAPU32)[_asan_js_check_index((growMemViews(), HEAPU32), ((argv_ptr) >>> 2) >>> 0, ___asan_storeN)] = 0;
  try {
    var ret = entryFunction(argc, argv);
    // if we're not running an evented main loop, it's time to exit
    exitJS(ret, /* implicit = */ true);
    return ret;
  } catch (e) {
    return handleException(e);
  }
}

function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  _emscripten_stack_init();
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  writeStackCookie();
}

async function run(args = programArgs) {
  if ((ENVIRONMENT_IS_PTHREAD)) {
    initRuntime();
    return;
  }
  stackCheckInit();
  preRun();
  if (runDependencies) {
    await resolveRunDependencies();
  }
  var setStatus = Module["setStatus"];
  if (setStatus) {
    setStatus("Running...");
    // Yield to the event loop to allow the browser to paint "Running..."
    await new Promise(resolve => setTimeout(resolve, 1));
    // Then we want to clear the status text, but only after the rest of this function runs.
    setTimeout(setStatus, 1, "");
  }
  if (ABORT) return;
  initRuntime();
  // No ATMAINS hooks
  Module["onRuntimeInitialized"]?.();
  var noInitialRun = Module["noInitialRun"] || false;
  if (!noInitialRun) callMain(args);
  postRun();
}

var wasmExports;

if ((!(ENVIRONMENT_IS_PTHREAD))) {
  // Call createWasm on startup if we are the main thread.
  // Worker threads call this once they receive the module via postMessage
  // With async instantation wasmExports is assigned asynchronously when the
  // instance is received.
  createWasm().then(() => run());
}
