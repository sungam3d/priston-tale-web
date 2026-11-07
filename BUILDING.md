# Building from source

How to rebuild the game server (`server.exe`), the Windows client (`game.exe`) and the browser client
(`game.js` + `game.wasm`) from this repo. Everything the builds read is in the repo except the two
toolchains in step 1.

Running the world doesn't need any of this: the repo already holds a built server and client (see
[README.md](README.md)). Build only if you change the source.

| You build | Output | Needs |
|---|---|---|
| Game server | `[project]\src\bin\ReleaseGL\server.exe` | Visual Studio 2022 C++ |
| Windows client (optional) | `[project]\src\bin\ReleaseGL\game.exe` | Visual Studio 2022 C++ |
| Browser client | `[project]\src\web\out\game.js`, `game.wasm` | Emscripten 6.0.12, Python 3 |

## 1. Install the tools

**Visual Studio 2022** (Community, or the free *Build Tools for Visual Studio 2022*) with:
- *Desktop development with C++*: it includes the MSVC v143 compiler and a Windows 10/11 SDK
- *C++ ATL for latest v143 build tools* and *C++ MFC for latest v143 build tools* (under Individual components)

**Python 3** from [python.org](https://www.python.org), with *Add python.exe to PATH* ticked.

**Emscripten 6.0.12**, installed **inside** the repo at `[project]\emsdk` (the build looks for it there):

```
cd C:\inetpub\priston\[project]
git clone https://github.com/emscripten-core/emsdk.git emsdk
cd emsdk
emsdk install 6.0.12
emsdk activate 6.0.12
```

Use exactly 6.0.12: that is the version the current `game.wasm` was built and tested with.

## 2. Get the source with git

```
git clone https://github.com/sungam3d/priston-tale-web.git C:\inetpub\priston
```

Use `git clone`, not *Download ZIP*. A few source files have Korean names (for example
`SrcCopyItems\공병부코드.h`, which `playsub.cpp` includes), and some unzip tools garble them, which
breaks the build. If you must use the ZIP, extract it with 7-Zip.

## 3. Avoid the `[project]` folder name

Emscripten can't handle the `[` `]` in `[project]`, so the browser build fails if you run it from the
real path. Make a link without brackets once, from a Command Prompt:

```
mklink /J C:\PristonBuild "C:\inetpub\priston\[project]"
```

Then always build from `C:\PristonBuild\src`. It is the same folder, so the output still lands in the
repo. Visual Studio doesn't mind the brackets, but using the link for both builds keeps things simple.

## 4. Build the server and the Windows client

```
cd C:\PristonBuild\src
build.bat Release GL
```

- `GL` is the OpenGL renderer that the browser port is built on. `build.bat Release DX6` builds the
  original DirectX 6 renderer instead.
- Output: `bin\ReleaseGL\server.exe` and `bin\ReleaseGL\game.exe`; the log is `build.log`.
- Only the server, from a Developer Command Prompt:
  `msbuild PristonServer.vcxproj /m /p:Configuration=Release /p:Platform=Win32 /p:PtGfx=GL /p:SolutionDir=C:\PristonBuild\src\`
- It is a 32-bit (x86) build on purpose: the game's 2002-era code relies on 32-bit layouts.

## 5. Build the browser client

```
cd C:\PristonBuild\src
python web\build_web.py --release
```

- About 200 files compile, then it links `web\out\game.html`, `game.js` and `game.wasm`.
- Later runs only recompile what changed. Delete `web\obj` to force a full rebuild.
- If the link fails, the missing names are in `web\out\undefined.txt` and the full log in `web\out\link.log`.

The game compiles against Windows headers from **mingw-w64 v12.0.0**, kept in
`[project]\src\thirdparty\mingw`. They are **changed** for WebAssembly, so a fresh mingw-w64 download
won't work in their place. The changes are listed in `thirdparty\mingw\PATCHES.md`.

## 6. Put the new build in place

**Server:** close the server window (it saves everyone), copy `bin\ReleaseGL\server.exe` over
`[project]\dist\server\server.exe`, and start it again. With the autostart task installed, first run
`Disable-ScheduledTask PristonServer` in an admin PowerShell so it can't restart the old exe, then close
the window, copy the exe, and run `Enable-ScheduledTask PristonServer; Start-ScheduledTask PristonServer`.

**Browser client:** copy **only** `web\out\game.js` and `web\out\game.wasm` to the repo's top folder.
Don't copy `web\out\game.html` over `index.html` or `game.html`: the site's pages have their own
additions. Players get the new client on their next page load.

**Game data:** after changing files in `data\`, players only download them once `data_manifest.txt`
lists their new hash: `python [project]\src\web\make_manifest.py data data_manifest.txt`.
A file that is also in `data_startup.pack` (every `.part` effect script, for example) is never
downloaded on its own, so changing one needs the pack rebuilt too (`web\make_pack.py`).

## What is in `[project]\src`

```
*.cpp, *.h, sinbaram\, HoBaram\, smLib3d\ ...   the original game source (client and server)
PristonTale.sln, *.vcxproj, build.bat          Visual Studio projects
ptgfx\                                         OpenGL renderer that replaces DirectX 6
ptplatform\, ptsound\                          window, input and sound on SDL2
web\                                           browser port: build_web.py, the Win32 layer (web\ptwin), page scripts
thirdparty\SDL2, lua-5.0.2, stb                libraries used by the builds
thirdparty\mingw                               patched Windows headers for the browser build
README.md, ROADMAP.md                          how the port was made, and what is left
```

Not in git: build output (`bin\`, `build\`, `web\obj\`, `web\out\`), Emscripten (`[project]\emsdk`,
step 1), and `thirdparty\dl`, the original download archives (nothing reads them).

## Common problems

| What you see | Cause | Fix |
|---|---|---|
| Browser build: errors mentioning paths with `[project]`, or *no such file* for `emsdk_env.bat` | Building from the real path, or Emscripten not in `[project]\emsdk` | Steps 1 and 3 |
| Browser build: hundreds of errors in `winnt.h`, `windows.h` or `d3dtypes.h` | `thirdparty\mingw` missing, or replaced with unpatched headers | Get it from the repo again |
| `playsub.cpp`: cannot open `SrcCopyItems\...코드.h` | Korean file names garbled by unzipping | Step 2: use `git clone` |
| `MSB8020: build tools for v143 cannot be found` | Visual Studio 2022 C++ not installed | Step 1 |
| `cannot open include file 'atlbase.h'` or `afxres.h` | ATL/MFC components missing | Step 1 |
| `LNK1104: cannot open file 'server.exe'` | The server is running and locks the file | Close the server window first |
