"""Builds the browser version of the game with Emscripten.

Usage: python web\\build_web.py [-j N] [--release] [--link-only]
Objects go to web\\obj\\<config>\\, the result to web\\out\\game.{html,js,wasm}.
Only changed sources are recompiled (by timestamp of the .cpp; headers not tracked:
delete web\\obj to force a full rebuild).
"""
import os, re, shutil, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.dirname(HERE)
EMSDK = os.path.join(os.path.dirname(SRC), 'emsdk')
MINGW = os.path.join(SRC, 'thirdparty', 'mingw', 'include')
PTWIN = os.path.join(HERE, 'ptwin')

def emsdk_env():
    out = subprocess.run('call "%s" >nul 2>nul && set' % os.path.join(EMSDK, 'emsdk_env.bat'),
                         shell=True, capture_output=True, text=True).stdout
    env = dict(os.environ)
    for line in out.splitlines():
        if '=' in line:
            k, v = line.split('=', 1)
            env[k] = v
    return env

def game_sources():
    proj = open(os.path.join(SRC, 'PristonTale.vcxproj'), encoding='utf-8').read()
    files = re.findall(r'<ClCompile Include="([^"]+)"', proj)
    # Windows-only pieces replaced in the web build
    skip = ()
    return [f for f in files if f not in skip]

def ptwin_sources():
    d = os.path.join(PTWIN, 'src')
    return [os.path.relpath(os.path.join(d, f), SRC) for f in sorted(os.listdir(d)) if f.endswith('.cpp')] if os.path.isdir(d) else []

def main():
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    args = sys.argv[1:]
    jobs = os.cpu_count() or 8
    release = '--release' in args
    link_only = '--link-only' in args
    # --asan: a test build with AddressSanitizer (web\out\game_asan.*, beside the normal game):
    # every bad memory access is reported with where it happened, and the game keeps going.
    # Slower and bigger; for hunting memory corruption, not for players.
    asan = '--asan' in args
    if '-j' in args:
        jobs = int(args[args.index('-j') + 1])
    config = 'asan' if asan else 'release' if release else 'debug'
    objdir = os.path.join(HERE, 'obj', config)
    env = emsdk_env()
    emxx = os.path.join(EMSDK, 'upstream', 'emscripten', 'em++.exe')
    flags = open(os.path.join(HERE, 'emflags.txt')).read().split()
    # extra defines for a test build, e.g. PT_DEFINES=PT_SERVER_PORT=10008 (a server beside the live one);
    # a changed define is not a changed file: delete web\obj to rebuild with it
    flags += ['-D' + d for d in os.environ.get('PT_DEFINES', '').split()]
    opt = ['-O1', '-fsanitize=address', '-fsanitize-recover=address'] if asan else ['-O2'] if release else ['-O1', '-g2']
    out_name = 'game_asan' if asan else 'game'
    incs = ['-I' + os.path.join(PTWIN, 'include'), '-I' + SRC] + ['-I' + os.path.join(SRC, d) for d in
            ['smLib3d', 'sinbaram', 'HoBaram', r'HoBaram\NewEffect\HoEffect\Script']] + ['-idirafter', MINGW]

    # Lua 5.0.2 (the effect scripts; a prebuilt .lib on Windows), compiled as C
    lua = os.path.join('thirdparty', 'lua-5.0.2', 'src')
    lua_sources = [os.path.join(lua, f) for f in sorted(os.listdir(os.path.join(SRC, lua))) if f.endswith('.c') and f != 'ltests.c'] + \
                  [os.path.join(lua, 'lib', f) for f in sorted(os.listdir(os.path.join(SRC, lua, 'lib'))) if f.endswith('.c')]
    emcc = os.path.join(EMSDK, 'upstream', 'emscripten', 'emcc.exe')
    c_flags = ['-pthread', '-I' + os.path.join(SRC, 'thirdparty', 'lua-5.0.2', 'include'),
               '-include', os.path.join(PTWIN, 'include', 'ptwin_fs.h')]      # Windows paths in luaL_loadfile

    sources = game_sources() + ptwin_sources() + lua_sources

    def up_to_date(o, s):
        # object newer than the source and every header it included (from the .d file)
        if not os.path.exists(o) or not os.path.exists(o + '.d'):
            return False
        t = os.path.getmtime(o)
        if os.path.getmtime(s) > t:
            return False
        deps = open(o + '.d', encoding='utf-8', errors='replace').read().replace('\\\n', ' ')
        deps = deps.split(':', 1)[1] if ': ' in deps else deps
        for d in re.findall(r'(?:[^\s\\]|\\ |\\(?!\s))+', deps):
            d = d.replace('\\ ', ' ')
            if os.path.exists(d) and os.path.getmtime(d) > t:
                return False
        return True
    def obj_of(f):
        return os.path.join(objdir, re.sub(r'[\\/ ]', '_', f) + '.o')

    def compile_one(f):
        o = obj_of(f)
        s = os.path.join(SRC, f)
        if up_to_date(o, s):
            return f, None
        if f in lua_sources:
            cmd = [emcc, '-c', '-Wno-everything', '-MD', '-MF', o + '.d'] + opt + c_flags + [s, '-o', o]
        else:
            cmd = [emxx, '-c', '-Wno-everything', '-MD', '-MF', o + '.d'] + opt + flags + incs + \
                  ['-include', os.path.join(SRC, 'compat.h'), s, '-o', o]
        r = subprocess.run(cmd, capture_output=True, text=True, errors='replace', env=env, cwd=SRC)
        return f, (r.stderr if r.returncode else None)

    os.makedirs(objdir, exist_ok=True)
    failed = []
    if not link_only:
        with ThreadPoolExecutor(jobs) as ex:
            for i, (f, err) in enumerate(ex.map(compile_one, sources)):
                print('%3d/%d %s %s' % (i + 1, len(sources), 'FAIL' if err else 'ok  ', f), flush=True)
                if err:
                    failed.append((f, err))
    if failed:
        for f, err in failed[:10]:
            print('\n== %s ==\n%s' % (f, err[:3000]))
        sys.exit(1)

    outdir = os.path.join(HERE, 'out')
    os.makedirs(outdir, exist_ok=True)
    link = [emxx] + opt + flags + [obj_of(f) for f in sources] + [
        '-sMAX_WEBGL_VERSION=2', '-sMIN_WEBGL_VERSION=2', '-sFULL_ES3',
        '-sASYNCIFY', '-sASYNCIFY_STACK_SIZE=1048576',
        '-sPTHREAD_POOL_SIZE=8', '-sALLOW_MEMORY_GROWTH', '-sINITIAL_MEMORY=256MB', '-sMAXIMUM_MEMORY=4GB',
        '-sSTACK_SIZE=4MB', '--pre-js', os.path.join(HERE, 'pre.js'), '--shell-file', os.path.join(HERE, 'shell.html'),
        '-sEXPORTED_RUNTIME_METHODS=addRunDependency,removeRunDependency,FS', '-sEXPORTED_FUNCTIONS=_main,_malloc,_free,_PtNet_Receive', '-sFORCE_FILESYSTEM', '-sERROR_ON_UNDEFINED_SYMBOLS=1', '-Wl,--error-limit=0',
        '-o', os.path.join(outdir, out_name + '.html')] + ([] if release or asan else ['-sASSERTIONS=1']) + [
        # crash reports players can read: function names in the stack (not wasm-function[1234]),
        # a stack overflow stops with "stack overflow" instead of silently wrecking memory,
        # and a name list per build (web\out\game.html.symbols) for older reports
        '--profiling-funcs', '-sSTACK_OVERFLOW_CHECK=2', '--emit-symbol-map'] + \
        (['-fsanitize=address'] if asan else [])
    # extra link flags for a test build, e.g. PT_LINK_FLAGS=--profiling-funcs (function names in crash stacks)
    link += os.environ.get('PT_LINK_FLAGS', '').split()
    rsp = os.path.join(objdir, 'link.rsp')
    open(rsp, 'w').write('\n'.join('"%s"' % a.replace('\\', '/') if ' ' in a else a.replace('\\', '/') for a in link[1:]))
    r = subprocess.run([emxx, '@' + rsp], capture_output=True, text=True, errors='replace', env=env, cwd=SRC)
    shutil.copy(os.path.join(HERE, 'server_worker.js'), outdir)      # the offline server's worker
    if not os.path.exists(os.path.join(outdir, 'config.js')):         # site settings (kept if edited)
        shutil.copy(os.path.join(HERE, 'config.js'), outdir)
    log = os.path.join(outdir, 'link.log')
    open(log, 'w', encoding='utf-8').write(r.stdout + r.stderr)
    undefined = sorted(set(re.findall(r'undefined symbol: (\S+)', r.stderr)))
    if undefined:
        open(os.path.join(outdir, 'undefined.txt'), 'w').write('\n'.join(undefined) + '\n')
        print('link: %d undefined symbols -> web\\out\\undefined.txt' % len(undefined))
        sys.exit(1)
    print('link: %s' % ('OK -> web\\out\\game.html' if r.returncode == 0 else 'FAILED, see web\\out\\link.log'))
    sys.exit(r.returncode)

if __name__ == '__main__':
    main()
