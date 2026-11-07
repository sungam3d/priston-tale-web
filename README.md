# Priston Tale in the Browser

Host your own Priston Tale world and play it straight from a web page.
The whole game client runs in the browser (WebAssembly); the world runs in `server.exe` on your Windows PC.

**You only need Windows and its built-in web server (IIS).** No SQL, no database, nothing for players to install.

```
Player's browser  <== https / wss ==>  IIS website (ws.ashx bridge)  <== 127.0.0.1:10007 ==>  server.exe
```

- Everyone on one shared world, from any modern browser (Chrome, Edge, Firefox)
- The first login to an ID creates the account, so there's no registration page to set up
- Players' browsers keep the game files: after the first visit (~90 MB) they only download files that changed
- The 3.7 GB of game data loads on demand while playing, never all at once

---

## What's in this repo

This repo **is** the website folder, with the game server and source kept inside it.

```
priston-tale-web\
├─ index.html, game.js, game.wasm, sw.js, progress.js, config.js   the web page and game
├─ ws.ashx, web.config                                              bridge to server.exe, IIS settings
├─ data_startup.pack, data_manifest.txt                             start-up files, list of game files
├─ data\                                                            game data (3.7 GB)
└─ [project]\
   ├─ dist\server\    the game server: server.exe, server.ini, world data
   └─ src\            client and server source, browser port (src\web), build scripts
```

`web.config` stops IIS from serving `[project]` and `.git`, so the server files and source stay private even though they sit in the website folder.

Not included: player accounts (server.exe creates `DataServer\` on first login), logs, and build output.

## What you need

- **Windows 10, 11 or Server**: the PC that stays on while people play
- **About 9 GB of disk** for the download
- **A domain name**, only if friends will play from other places (a free dynamic-DNS name works)

---

## Setup

### 1. Download

Click **Code → Download ZIP** on GitHub and unzip it, or clone it:

```
git clone https://github.com/sungam3d/priston-tale-web.git C:\inetpub\priston
```

It's about 2.2 GB. The examples below use `C:\inetpub\priston`.

### 2. Turn on IIS

Right-click Start, open **Windows PowerShell (Admin)** or **Terminal (Admin)**, and run:

```
Enable-WindowsOptionalFeature -Online -All -FeatureName IIS-WebServerRole,IIS-WebServer,IIS-StaticContent,IIS-DefaultDocument,IIS-WebSockets,IIS-ASPNET45,IIS-NetFxExtensibility45,IIS-ISAPIExtensions,IIS-ISAPIFilter,IIS-ManagementConsole
```

Prefer clicking? *Turn Windows features on or off* → *Internet Information Services* → *World Wide Web Services* → *Application Development Features*: tick **WebSocket Protocol** and **ASP.NET 4.8**. Also tick *Web Management Tools* → **IIS Management Console**.

> **Don't skip WebSocket Protocol.** Without it the game loads, but every login says it can't reach the server.

### 3. Point IIS at the folder

Open **IIS Manager** (search the Start menu for "IIS") → *Sites* → *Default Web Site* → *Basic Settings…*, and set **Physical path** to `C:\inetpub\priston`.

The application pool's defaults (.NET CLR v4.0, Integrated) are right.
If you put the folder outside `C:\inetpub`, give **IIS_IUSRS** Read access to it (right-click → Properties → Security).

### 4. Start the game server

Double-click `C:\inetpub\priston\[project]\dist\server\server.exe` and leave its window open.
Give it **1–2 minutes** to load the world. Closing the window shuts the world down and saves everyone.

Port 10007 is only used inside your PC. **Don't** open it in your firewall or router.

### 5. Play

On the same PC, open **http://localhost/**. The intro plays, then the login screen appears.
Type any ID and password: the first login creates the account, and later logins need the same password.

### 6. Let friends play (HTTPS)

Browsers only run the game on secure pages, so other PCs need an `https://` address:

1. Point a domain name (or a free dynamic-DNS name) at your internet connection.
2. Forward ports **80** and **443** on your router to this PC.
3. IIS Manager → *Default Web Site* → *Bindings…* → edit the **http** binding and set **Host name**, for example `play.example.com`.
4. Download [win-acme](https://www.win-acme.com), run `wacs.exe` as Administrator, choose **N: Create certificate (default settings)** and pick your site. It adds HTTPS and renews the certificate by itself.

Friends can now play at `https://play.example.com/`.

### Optional: keep the server running

To start server.exe with Windows and restart it if it ever stops, run this once in an admin PowerShell in the server folder:

```
powershell -ExecutionPolicy Bypass -File .\install_autostart.ps1
```

---

## Running your world

- **Back up** `[project]\dist\server\DataServer`. It holds every account and character.
- **Forgotten password:** delete `DataServer\password\<ID>.pw`. The next login sets a new one.
- **Rates and settings:** edit `[project]\dist\server\server.ini`, then restart server.exe.
- **Changed game files?** Players' browsers keep their copies until the file list changes, so rebuild it:
  `python [project]\src\web\make_manifest.py data data_manifest.txt`

## If something goes wrong

Add `?debugnet` to the address (`http://localhost/?debugnet`). The log under the game then shows every connection attempt.

| What you see | What to do |
|---|---|
| Login can't connect to the server | Is server.exe running, and has it had 2 minutes? Is **WebSocket Protocol** on (step 2)? Run `iisreset` after turning it on. |
| Black page or "SharedArrayBuffer" error | You're on `http://` from another PC. Use the `https://` address (step 6), or `http://localhost/` on the server PC. |
| `/ws.ashx` shows 404 or 500 instead of "Bad Request" | ASP.NET 4.8 is missing (step 2), or the app pool isn't .NET v4.0. "Bad Request" is the normal answer to a plain visit. |
| Game files fail to load (404s in F12 → Console) | The `data` folder or its `web.config` is missing. |
| "Please re-enter your password" | That ID already exists with another password. |

---

## Building from source

See **[BUILDING.md](BUILDING.md)**: tools to install, where Emscripten must go, the `[project]` folder-name
workaround, and which files to copy into place. In short:

- **Server:** Visual Studio 2022 C++ with ATL/MFC, then `build.bat Release GL` in `[project]\src` → `bin\ReleaseGL\server.exe`.
- **Browser client:** Emscripten 6.0.12 in `[project]\emsdk`, then `python web\build_web.py --release` from a bracket-free link to `[project]\src` → `web\out\game.js` + `game.wasm`.
- **Test without IIS:** `python [project]\src\web\serve.py 8080 .` serves this folder on http://localhost:8080/ with the same WebSocket bridge.

Priston Tale and its game data belong to their original publishers. This project is a fan-made web port for hosting your own world.
