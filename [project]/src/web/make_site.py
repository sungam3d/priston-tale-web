"""Makes the hosting package for the shared server: PristonWeb\\dist\\

  dist\\site\\     - copy into the IIS site folder (the web page, game data, the
                    WebSocket bridge ws.ashx, web.config)
  dist\\server\\   - run server.exe from here, on the same computer as IIS
  dist\\README.txt

Build first: python web\\build_web.py --release, and build.bat Release GL (server.exe).
Usage: python web\\make_site.py [--out folder] [--keep-saves]
  --keep-saves  also copy the test install's DataServer (accounts and characters);
                by default the server starts with no accounts.
Run it again after an update: only changed files are copied (robocopy).
"""
import os, shutil, subprocess, sys
from make_manifest import file_hash

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.dirname(HERE)
WEB = os.path.dirname(SRC)
GAME = os.path.join(WEB, 'game')
OUTDIR = os.path.join(HERE, 'out')
SERVER_EXE = os.path.join(SRC, 'bin', 'ReleaseGL', 'server.exe')

# client data: everything in the test install except the server, saves and diagnostics
SKIP_DIRS = {'server', 'savedata'}
SKIP_EXT = ('.exe', '.dll', '.pdb', '.dmp', '.log', '.lib', '.ilk')
SKIP_PREFIX = ('ptgfx_frame', 'ptgfx_trace', 'disconnect')
SERVER_SKIP_DIRS = ['LogFile']          # plus DataServer unless --keep-saves


def robocopy(src, dst, xd=(), xf=()):
    cmd = ['robocopy', src, dst, '/E', '/MT:16', '/R:1', '/W:1', '/NFL', '/NDL', '/NJH', '/NJS', '/NP']
    if xd: cmd += ['/XD'] + list(xd)
    if xf: cmd += ['/XF'] + list(xf)
    r = subprocess.run(cmd)
    if r.returncode >= 8:
        sys.exit('robocopy failed (%d): %s -> %s' % (r.returncode, src, dst))


def client_files():
    for d, dirs, files in os.walk(GAME):
        rel_d = os.path.relpath(d, GAME)
        if rel_d == '.':
            dirs[:] = [x for x in dirs if x.lower() not in SKIP_DIRS]
        dirs.sort()
        for f in sorted(files):
            if f.lower().endswith(SKIP_EXT) or f.lower().startswith(SKIP_PREFIX):
                continue
            p = os.path.join(d, f)
            yield os.path.relpath(p, GAME).replace(os.sep, '/'), os.path.getsize(p), file_hash(p)


def data_web_config(exts):
    maps = '\n'.join('\t\t\t<remove fileExtension="%s" />\n\t\t\t<mimeMap fileExtension="%s" mimeType="application/octet-stream" />'
                     % (e, e) for e in sorted(exts))
    return """<?xml version="1.0" encoding="UTF-8"?>
<!-- Game data: IIS only serves file types it knows, so every data file type is listed;
     some file names contain '+', and request filtering must not hide any folder. -->
<configuration>
	<system.webServer>
		<staticContent>
%s
			<clientCache cacheControlMode="UseMaxAge" cacheControlMaxAge="7.00:00:00" />
		</staticContent>
		<security>
			<requestFiltering allowDoubleEscaping="true">
				<fileExtensions allowUnlisted="true">
					<clear />
				</fileExtensions>
				<hiddenSegments>
					<clear />
				</hiddenSegments>
			</requestFiltering>
		</security>
	</system.webServer>
</configuration>
""" % maps


def main():
    args = sys.argv[1:]
    dist = args[args.index('--out') + 1] if '--out' in args else os.path.join(WEB, 'dist')
    keep_saves = '--keep-saves' in args
    site, server = os.path.join(dist, 'site'), os.path.join(dist, 'server')
    for f in ('game.html', 'game.js', 'game.wasm', 'data_startup.pack'):
        if not os.path.exists(os.path.join(OUTDIR, f)):
            sys.exit('missing web\\out\\%s - run python web\\build_web.py --release first' % f)
    if not os.path.exists(SERVER_EXE):
        sys.exit('missing %s - run build.bat Release GL first' % SERVER_EXE)
    os.makedirs(site, exist_ok=True)
    os.makedirs(server, exist_ok=True)

    # the page
    print('site: page and bridge')
    shutil.copy(os.path.join(OUTDIR, 'game.html'), os.path.join(site, 'index.html'))
    for f in ('game.html', 'game.js', 'game.wasm', 'server_worker.js', 'data_startup.pack'):
        shutil.copy(os.path.join(OUTDIR, f), site)
    for f in ('sw.js', 'progress.js', 'settings.js', 'admin.js'):
        shutil.copy(os.path.join(HERE, f), site)
    for f in ('ws.ashx', 'web.config'):
        shutil.copy(os.path.join(HERE, 'site', f), site)
    if not os.path.exists(os.path.join(site, 'config.js')):            # kept if you edited it
        with open(os.path.join(site, 'config.js'), 'w', newline='\n') as fp:
            fp.write(open(os.path.join(HERE, 'config.js')).read().replace("serverUrl: ''", "serverUrl: 'ws.ashx'"))

    # game data and its list
    print('site: game data (first run copies ~4 GB)')
    files = list(client_files())
    with open(os.path.join(site, 'data_manifest.txt'), 'w', encoding='utf-8', newline='\n') as fp:
        fp.write(''.join('%s\t%d\t%s\n' % f for f in files))          # path, size, hash (web\sw.js)
    data = os.path.join(site, 'data')
    robocopy(GAME, data, xd=[os.path.join(GAME, d) for d in SKIP_DIRS],
             xf=['*.exe', '*.dll', '*.pdb', '*.dmp', '*.log', '*.lib', '*.ilk', 'ptgfx_frame*', 'ptgfx_trace*', 'disconnect*'])
    exts = set(os.path.splitext(r)[1].lower() for r, _, _ in files if os.path.splitext(r)[1])
    with open(os.path.join(data, 'web.config'), 'w', encoding='utf-8', newline='\r\n') as fp:
        fp.write(data_web_config(exts))

    # the server
    print('server: server.exe and its data (first run copies ~3.5 GB)')
    gs = os.path.join(GAME, 'server')
    skip = SERVER_SKIP_DIRS + ([] if keep_saves else ['DataServer'])
    robocopy(gs, server, xd=[os.path.join(gs, d) for d in skip], xf=['*.log', 'missing_models.txt', 'server.ini'])
    shutil.copy(SERVER_EXE, server)
    shutil.copy(os.path.join(HERE, 'site', 'install_autostart.ps1'), server)
    if os.path.exists(os.path.join(GAME, 'SDL2.dll')):
        shutil.copy(os.path.join(GAME, 'SDL2.dll'), server)
    ini = os.path.join(server, 'server.ini')
    if not os.path.exists(ini):                                          # kept if you edited it
        text = open(os.path.join(gs, 'server.ini'), encoding='cp949', errors='replace').read()
        if '*OFFLINE_PASSWORDS' not in text:
            text = text.replace('*OFFLINE_MODE', '*OFFLINE_MODE\n// shared web server: the first login to an ID sets its password\n*OFFLINE_PASSWORDS', 1)
        with open(ini, 'w', encoding='cp949', newline='\r\n') as fp:
            fp.write(text)

    shutil.copy(os.path.join(HERE, 'site', 'README.txt'), dist)
    print('done -> %s' % dist)


if __name__ == '__main__':
    main()
