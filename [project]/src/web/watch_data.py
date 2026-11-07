"""Keeps the browser build's download lists up to date: copy a file into the site's
data folder and players get it the next time they load the page.

Browsers only fetch a file again when its hash in data_manifest.txt changes (web\\sw.js),
and files in data_startup.pack are never fetched at all (web\\pre.js unpacks them first).
So this looks at the data folder every few seconds, and when a file changed:
  - updates, adds or removes just that file's line in data_manifest.txt
  - if the file is also in data_startup.pack, replaces it there (players then download
    the whole pack again, about 84 MB)

A file counts as changed once it looks the same on two looks in a row, so a copy still
in progress is left alone. Folders named like "Field - Copy" are never published.

Usage: python web\\watch_data.py <site folder> [--once] [--dry-run] [--interval SECONDS]
  --once      one full check against the files' contents, then quit
  --dry-run   say what would change and write nothing (implies --once)
Started by install_data_watch.ps1. State and log are in <site>\\[project]\\data_watch\\.
"""
import json, os, re, struct, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make_manifest import SKIP_EXT, SKIP_NAMES, SKIP_PREFIX, file_hash

SKIP_DIR = re.compile(r' - copy( \(\d+\))?$', re.I)     # what Explorer calls a copied folder


def skipped(name):
    n = name.lower()
    return n.endswith(SKIP_EXT) or n.startswith(SKIP_PREFIX) or n in SKIP_NAMES


def scan(data):
    """path -> (size, modified time) of every file that may be published"""
    found = {}
    for d, dirs, files in os.walk(data):
        dirs[:] = [x for x in dirs if not SKIP_DIR.search(x)]
        for f in files:
            if skipped(f):
                continue
            p = os.path.join(d, f)
            try:
                st = os.stat(p)
            except OSError:
                continue                                # deleted while looking
            found[os.path.relpath(p, data).replace(os.sep, '/')] = (st.st_size, st.st_mtime_ns)
    return found


def replace_file(tmp, path):
    """IIS may be sending the old file for a moment: keep trying"""
    for attempt in range(30):
        try:
            os.replace(tmp, path)
            return
        except PermissionError:
            if attempt == 29:
                raise
            time.sleep(1)


def read_pack(path):
    """(path, size, offset of its data) per file in the pack"""
    entries = []
    with open(path, 'rb') as fp:
        if fp.read(4) != b'PTPK':
            raise ValueError('not a pack: ' + path)
        count = struct.unpack('<I', fp.read(4))[0]
        for _ in range(count):
            n = struct.unpack('<H', fp.read(2))[0]
            rel = fp.read(n).decode('utf-8')
            size = struct.unpack('<I', fp.read(4))[0]
            entries.append((rel, size, fp.tell()))
            fp.seek(size, 1)
    return entries


class Watcher:
    def __init__(self, site, dry):
        self.data = os.path.join(site, 'data')
        self.manifest = os.path.join(site, 'data_manifest.txt')
        self.pack = os.path.join(site, 'data_startup.pack')
        self.dry = dry
        self.dir = os.path.join(site, '[project]', 'data_watch')
        self.state_file = os.path.join(self.dir, 'state.json')
        self.state = {}                                 # path -> [size, modified time, hash]
        self.pending = {}                               # changed on the last look: path -> (size, modified time)
        if not dry:
            os.makedirs(self.dir, exist_ok=True)
            try:
                with open(self.state_file, encoding='utf-8') as fp:
                    self.state = json.load(fp)
            except (OSError, ValueError):
                self.state = {}

    def log(self, text):
        line = '%s %s' % (time.strftime('%Y-%m-%d %H:%M:%S'), text)
        if sys.stdout:
            print(line, flush=True)
        if not self.dry:
            with open(os.path.join(self.dir, 'watch.log'), 'a', encoding='utf-8') as fp:
                fp.write(line + '\n')

    def save_state(self):
        if self.dry:
            return
        tmp = self.state_file + '.tmp'
        with open(tmp, 'w', encoding='utf-8') as fp:
            json.dump(self.state, fp)
        os.replace(tmp, self.state_file)

    def write_manifest(self, changed, removed):
        """Changes only these files' lines, so nothing else lying in the folder gets published"""
        with open(self.manifest, encoding='utf-8', newline='\n') as fp:
            lines = fp.read().rstrip('\n').split('\n')
        index = {ln.split('\t')[0].lower(): i for i, ln in enumerate(lines)}
        edits = 0
        for rel in changed:
            size, _, h = self.state[rel]
            i = index.get(rel.lower())
            # a file's line keeps its spelling, which is the name the browser stored it under
            line = '%s\t%d\t%s' % (lines[i].split('\t')[0] if i is not None else rel, size, h)
            if i is None:
                lines.append(line)
                index[rel.lower()] = len(lines) - 1
                self.log('added   ' + rel)
                edits += 1
            elif lines[i] != line:
                lines[i] = line
                self.log('changed ' + rel)
                edits += 1
        gone = set()
        for rel in removed:
            if rel.lower() in index and not os.path.exists(os.path.join(self.data, rel.replace('/', os.sep))):
                gone.add(index[rel.lower()])
                self.log('removed ' + rel)
        if gone:
            lines = [ln for i, ln in enumerate(lines) if i not in gone]
            edits += len(gone)
        if edits and not self.dry:
            tmp = self.manifest + '.tmp'
            with open(tmp, 'w', encoding='utf-8', newline='\n') as fp:
                fp.write('\n'.join(lines) + '\n')
            replace_file(tmp, self.manifest)
        return edits

    def write_pack(self, touched):
        """Replaces the given files inside the pack if they differ from the data folder"""
        if not os.path.exists(self.pack):
            return 0
        touched = {t.lower() for t in touched}
        entries = read_pack(self.pack)
        new = {}                                        # index -> file contents, None to drop it
        with open(self.pack, 'rb') as fp:
            for i, (rel, size, off) in enumerate(entries):
                if rel.lower() not in touched:
                    continue
                p = os.path.join(self.data, rel.replace('/', os.sep))
                if not os.path.exists(p):
                    new[i] = None
                    continue
                with open(p, 'rb') as src:
                    now = src.read()
                fp.seek(off)
                if len(now) != size or fp.read(size) != now:
                    new[i] = now
        if not new:
            return 0
        for i in new:
            self.log('pack    ' + entries[i][0])
        if self.dry:
            return len(new)
        tmp = self.pack + '.tmp'
        with open(self.pack, 'rb') as fp, open(tmp, 'wb') as out:
            out.write(b'PTPK' + struct.pack('<I', len(entries) - sum(1 for v in new.values() if v is None)))
            for i, (rel, size, off) in enumerate(entries):
                if i in new and new[i] is None:
                    continue
                if i in new:
                    body = new[i]
                else:
                    fp.seek(off)
                    body = fp.read(size)
                name = rel.encode('utf-8')
                out.write(struct.pack('<H', len(name)) + name + struct.pack('<I', len(body)) + body)
        replace_file(tmp, self.pack)
        return len(new)

    def apply(self, changed, removed):
        hashed = []
        for rel, sig in changed:
            try:
                self.state[rel] = [sig[0], sig[1], file_hash(os.path.join(self.data, rel.replace('/', os.sep)))]
                hashed.append(rel)
            except OSError as e:                        # still being written: the next look sees it again
                self.state.pop(rel, None)
                self.log('busy    %s (%s)' % (rel, e))
        for rel in removed:
            self.state.pop(rel, None)
        edits = self.write_manifest(hashed, removed)
        edits += self.write_pack(hashed + removed)
        self.save_state()
        return edits

    def full_check(self):
        """Every file against the lists by content, so what changed while this wasn't running is found too"""
        found = scan(self.data)
        known = {k.lower() for k in found}
        with open(self.manifest, encoding='utf-8', newline='\n') as fp:
            listed = [ln.split('\t')[0] for ln in fp.read().split('\n') if '\t' in ln]
        removed = [k for k in listed if k.lower() not in known]
        self.state = {}
        edits = self.apply(list(found.items()), removed)
        self.log('full check: %d files, %d updates' % (len(found), edits))

    def look(self):
        found = scan(self.data)
        changed = [(k, v) for k, v in found.items() if k not in self.state or tuple(self.state[k][:2]) != v]
        removed = [k for k in self.state if k not in found]
        ready = [(k, v) for k, v in changed if self.pending.get(k) == v]
        self.pending = dict(changed)
        for k, _ in ready:
            del self.pending[k]
        if ready or removed:
            self.apply(ready, removed)


def main():
    args = sys.argv[1:]
    site = args[0]
    dry = '--dry-run' in args
    interval = float(args[args.index('--interval') + 1]) if '--interval' in args else 10.0
    w = Watcher(site, dry)
    w.full_check()
    if dry or '--once' in args:
        return
    while True:
        time.sleep(interval)
        try:
            w.look()
        except Exception as e:                          # a bad moment must not end the watching
            w.log('error   %r' % (e,))


if __name__ == '__main__':
    main()
