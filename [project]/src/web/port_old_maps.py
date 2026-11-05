"""Ports the maps of an old client (e.g. the 1753 client's Field folder) into the
current game as extra fields, moved to an empty part of the world.

An old map sits at the same world position as today's version of it, so every
old stage is shifted by OFFSET (whole multiples of 16384 world units: the
stage's collision grid is 256 cells of 64 units and wraps, so a shift by
16384*k leaves the stored grid valid and only the positions change).

Per stage .smd (written to Field\\old\\<folder>\\o_<name>.smd):
  * vertices, lights and StageMapRect moved by the offset;
  * 16-bit area (collision grid) lists of the oldest files widened to the
    32-bit entries the engine reads, and the stale tail that old SaveFile left
    behind (it opened files with OPEN_ALWAYS, which does not truncate) dropped;
  * texture names: a texture that the current game has with the same contents
    keeps its name; one that differs or is missing from the current game is
    copied to Field\\old\\... and the material points there.
Animated props (model .smd files) keep their own world position; the game
draws them shifted by the field's offset (sFIELD::OffsetX/Z). Only their
texture names are rewritten, and each object's file pointer is moved by the
change in size of the material block.

Texture search order: the old client, then the extra old clients given with
--also (closest era first), then the current game.

Usage:
  python web\\port_old_maps.py <old client folder> <current data folder> <out data folder>...
         [--also <old client folder>]... [--server <server folder>] [--report <json>]
The data folders get Field\\old\\..., Field\\map\\o_*.tga, Field\\Title\\o_*t.tga;
the server folder gets only the stage files (Field\\old\\...).
"""
import hashlib, json, os, shutil, struct, sys

HDR_STAGE = b'SMD Stage data Ver 0.72'
HDR_MODEL = b'SMD Model data Ver 0.62'
FILE_HEADER = 0x22C
ST_AREA = FILE_HEADER + 4               # smSTAGE3D::StageArea[256][256] (only "non-null" matters)
ST_NV, ST_NF, ST_NT, ST_NL = 0x40260, 0x40264, 0x40268, 0x4026C
ST_RECT = 0x40290                       # StageMapRect left, top, right, bottom (fixed point)
ST_MATGRP = 0x402A0
MATGRP_SIZE, MAT_SIZE = 88, 320
OBJINFO_SIZE = 40                       # smDFILE_OBJINFO: name[32], length, file pointer
OB_PHYSIQUE, OB_TM = 16, 240
GRID = 16384

# folder, old file, new base name, current field of the same map (guide map / settings), extra shift
MAPS = [
    ('forest', 'fore-3', 'o_fore-3', 0, (0, 0)),
    ('forest', 'fore-2', 'o_fore-2', 1, (0, 0)),
    ('forest', 'fore-1', 'o_fore-1', 2, (0, 0)),
    ('Ricarten', 'village-2', 'o_village-2', 3, (0, 0)),
    ('Ruin', 'ruin-4', 'o_ruin-4', 4, (0, 0)),
    ('Ruin', 'ruin-3', 'o_ruin-3', 5, (0, 0)),
    ('Ruin', 'ruin-2', 'o_ruin-2', 6, (0, 0)),
    ('Ruin', 'ruin-1', 'o_ruin-1', 7, (0, 0)),
    ('desert', 'De-1', 'o_De-1', 8, (0, 0)),
    ('forest', 'village-1', 'o_village-1', 9, (0, 0)),
    ('desert', 'De-2', 'o_De-2', 10, (0, 0)),
    ('desert', 'De-3', 'o_De-3', 11, (0, 0)),
    ('desert', 'De-4', 'o_De-4', 12, (0, 0)),
    ('dungeon', 'dun-1', 'o_dun-1', 13, (0, 0)),
    ('dungeon', 'dun-2', 'o_dun-2', 14, (0, 0)),
    ('dungeon', 'dun-3', 'o_dun-3', 15, (0, 0)),
    ('Room', 'office', 'o_office', 16, (131072, -65536)),
    ('forever-fall', 'forever-fall-04', 'o_forever-fall-04', 17, (0, 0)),
    ('forever-fall', 'forever-fall-03', 'o_forever-fall-03', 18, (0, 0)),
    ('forever-fall', 'forever-fall-02', 'o_forever-fall-02', 19, (0, 0)),
    ('forever-fall', 'forever-fall-01', 'o_forever-fall-01', 20, (0, 0)),
    ('forever-fall', 'pilai', 'o_pilai', 21, (0, 0)),
    ('Ice', 'ice_ura', 'o_ice_ura', 29, (0, 0)),
    ('castle', 'castle', 'o_castle', 33, (0, 0)),
    ('SeaA', 'SeaA', 'o_SeaA', -1, (0, 0)),
    ('SeaA', 'Stemple', 'o_Stemple', -1, (0, 0)),
    # an earlier Ricarten from the forest folder: same place as Ricarten\village-2, so moved again
    ('forest', 'village-2', 'o_village-2f', 3, (0, -65536)),
]
OFFSET = (-8 * GRID, 0)                 # x, z (world units) for the whole old world

# centre (warp-in point, field.cpp SetCenterPos) of today's version of a map, tried first
CENTRES = {0: (-16419, -7054), 1: (-5427, -9915), 2: (4184, -11016), 3: (2596, -18738), 4: (1384, -918),
           5: (4953, 10922), 6: (7459, 23984), 7: (16362, 14959), 8: (15005, -1421), 9: (22214, -1182),
           10: (15887, -11161), 11: (34758, -1323), 12: (43988, 12053), 13: (-15384, -24310),
           14: (-6108, -26880), 15: (1827, -28586), 16: (-200000, 200000), 17: (-1583, 37905),
           18: (-5823, 43244), 19: (1691, 52599), 20: (2032, 71183), 21: (2981, 75486),
           29: (32621, 23865), 33: (35225, -23847)}

# props in the old folders that are not placed on a map
SKIP_MODELS = {'ani\\ani.smd', 'ani\\test.smd'}
# props whose map is known from their name (overlapping maps: forever-fall-01 / Pilai)
PROP_HOME = {'piani-': 'o_pilai', '1ani-': 'o_forever-fall-01'}


def I(b, o):
    return struct.unpack_from('<i', b, o)[0]


def blake(path):
    h = hashlib.blake2b(digest_size=8)
    with open(path, 'rb') as fp:
        for chunk in iter(lambda: fp.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


class Finder:
    """Case-insensitive file lookup below one game folder."""
    def __init__(self, root):
        self.root = root
        self.dirs = {}

    def _list(self, d):
        if d not in self.dirs:
            try:
                self.dirs[d] = {e.lower(): e for e in os.listdir(d)}
            except OSError:
                self.dirs[d] = {}
        return self.dirs[d]

    def find(self, rel):
        cur = self.root
        for part in rel.replace('/', '\\').split('\\'):
            if not part:
                continue
            e = self._list(cur).get(part.lower())
            if e is None:
                return None
            cur = os.path.join(cur, e)
        return cur if os.path.isfile(cur) else None


class Textures:
    def __init__(self, old_roots, cur_root, outs):
        self.old = [Finder(r) for r in old_roots]
        self.cur = Finder(cur_root)
        self.outs = outs
        self.map = {}               # lower name -> name to store
        self.copied = {}            # new rel path -> source
        self.missing = set()
        self.kept = 0

    def resolve(self, name):
        key = name.lower()
        if key in self.map:
            return self.map[key]
        src = None
        for f in self.old:
            src = f.find(name)
            if src:
                break
        cur = self.cur.find(name)
        if not src:
            new = name
            if not cur:
                self.missing.add(key)
            else:
                self.kept += 1
        elif cur and os.path.getsize(cur) == os.path.getsize(src) and blake(cur) == blake(src):
            new = name
            self.kept += 1
        else:
            parts = name.split('\\')
            if parts[0].lower() == 'field':
                new = '\\'.join([parts[0], 'old'] + parts[1:-1] + [os.path.basename(src)])
            else:
                new = '\\'.join(['field', 'old', '_other'] + parts[:-1] + [os.path.basename(src)])
            self.copied[new.lower()] = (new, src)
        self.map[key] = new
        return new

    def write(self):
        for new, src in self.copied.values():
            for out in self.outs:
                dst = os.path.join(out, new)
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                if not (os.path.exists(dst) and os.path.getsize(dst) == os.path.getsize(src)):
                    shutil.copyfile(src, dst)


def valid_name(s):
    return bool(s) and b'.' in s and all(32 <= c < 127 for c in s)


def rewrite_materials(b, off, count, tex):
    """Returns (new material block bytes, end offset in b)."""
    out = bytearray()
    for _ in range(count):
        raw = b[off:off + MAT_SIZE]
        out += raw
        inuse = I(b, off)
        off += MAT_SIZE
        if inuse:
            size = I(b, off)
            blob = b[off + 4:off + 4 + size]
            off += 4 + size
            ntex, nanim = I(raw, 4), I(raw, 76 * 4)
            ntex = max(0, min(ntex, 8)) if inuse else 0
            nanim = max(0, min(nanim, 32)) if inuse else 0
            parts = blob.split(b'\0')
            want = (ntex + nanim) * 2
            names, rest = parts[:want], parts[want:]
            new = []
            for s in names:
                if valid_name(s):
                    s = tex.resolve(s.decode('latin-1')).encode('latin-1')
                new.append(s)
            # anything after the used names (normally nothing) is kept as it was
            nb = b'\0'.join(new) + b'\0'
            if want < len(parts) - 1:
                nb += b'\0'.join(rest[:-1]) + b'\0'
            elif len(names) < want:
                nb = blob
            out += struct.pack('<i', len(nb)) + nb
    return bytes(out), off


def port_stage(src, off_x, off_z, tex):
    b = open(src, 'rb').read()
    if not b.startswith(HDR_STAGE):
        raise ValueError('%s is not a stage file' % src)
    nv, nf, nt, nl = I(b, ST_NV), I(b, ST_NF), I(b, ST_NT), I(b, ST_NL)
    nmat = I(b, ST_MATGRP + 8)
    mats, off = rewrite_materials(b, ST_MATGRP + MATGRP_SIZE, nmat, tex)
    head = bytearray(b[:ST_MATGRP + MATGRP_SIZE])
    dx, dz = off_x * 256, off_z * 256
    l, t, r, btm = struct.unpack_from('<4i', head, ST_RECT)
    struct.pack_into('<4i', head, ST_RECT, l + dx, t + dz, r + dx, btm + dz)

    verts = bytearray(b[off:off + nv * 28])
    for k in range(nv):
        x, y, z = struct.unpack_from('<3i', verts, k * 28 + 8)
        struct.pack_into('<3i', verts, k * 28 + 8, x + dx, y, z + dz)
    off += nv * 28
    faces = b[off:off + nf * 28 + nt * 32]
    off += nf * 28 + nt * 32
    lights = bytearray(b[off:off + nl * 28])
    for k in range(nl):
        x, y, z = struct.unpack_from('<3i', lights, k * 28 + 4)
        struct.pack_into('<3i', lights, k * 28 + 4, x + dx, y, z + dz)
    off += nl * 28

    cells = sum(1 for k in range(256 * 256) if I(b, ST_AREA + k * 4))

    def walk(width):
        o, lists = off, []
        for _ in range(cells):
            if o + 4 > len(b):
                return None
            s = I(b, o)
            if s < 1 or o + 4 + s * width > len(b):
                return None
            vals = struct.unpack_from('<%d%s' % (s, 'H' if width == 2 else 'I'), b, o + 4)
            if vals[0] + 1 != s or max(vals[1:] or (0,)) >= nf:
                return None
            lists.append(vals)
            o += 4 + s * width
        return lists, o
    w, width = walk(4), 4
    if not w:
        w, width = walk(2), 2
    if not w:
        raise ValueError('%s: area lists not understood' % src)
    lists, end = w
    area = bytearray()
    for vals in lists:
        area += struct.pack('<i%dI' % len(vals), len(vals), *vals)
    info = dict(vertices=nv, faces=nf, lights=nl, materials=nmat, area_width=width * 8,
                stale_tail=len(b) - end,
                rect=[(l + dx) / 256, (t + dz) / 256, (r + dx) / 256, (btm + dz) / 256])
    data = bytes(head) + mats + bytes(verts) + faces + bytes(lights) + bytes(area)
    return data, info, (ST_MATGRP + MATGRP_SIZE + len(mats), nv, nf)


def floor_point(b, voff, nv, nf, want_x, want_z):
    """A point on an upward face near (want_x, want_z) (world units of the file b)."""
    foff = voff + nv * 28
    best, bestd = None, None
    for k in range(nf):
        v0, v1, v2 = struct.unpack_from('<3H', b, foff + k * 28 + 8)
        ny = struct.unpack_from('<h', b, foff + k * 28 + 22)[0]
        if abs(ny) < 30000 or max(v0, v1, v2) >= nv:
            continue
        p = [struct.unpack_from('<3i', b, voff + v * 28 + 8) for v in (v0, v1, v2)]
        # inside test on x/z
        (ax, _, az), (bx, _, bz), (cx, _, cz) = [(q[0] / 256, q[1] / 256, q[2] / 256) for q in p]
        wx, wz = want_x, want_z
        d1 = (wx - bx) * (az - bz) - (ax - bx) * (wz - bz)
        d2 = (wx - cx) * (bz - cz) - (bx - cx) * (wz - cz)
        d3 = (wx - ax) * (cz - az) - (cx - ax) * (wz - az)
        if (d1 >= 0 and d2 >= 0 and d3 >= 0) or (d1 <= 0 and d2 <= 0 and d3 <= 0):
            return int(want_x), int(want_z), True
        mx, mz = (ax + bx + cx) / 3, (az + bz + cz) / 3
        d = (mx - want_x) ** 2 + (mz - want_z) ** 2
        if bestd is None or d < bestd:
            best, bestd = (int(mx), int(mz)), d
    return (best[0], best[1], False) if best else (int(want_x), int(want_z), False)


def port_model(src, tex):
    b = open(src, 'rb').read()
    if not b.startswith(HDR_MODEL):
        return None
    nobj, nmat = I(b, 0x18), I(b, 0x1C)
    infos = FILE_HEADER
    mat_at = infos + nobj * OBJINFO_SIZE
    if nmat > 0:
        mats, end = rewrite_materials(b, mat_at + MATGRP_SIZE, I(b, mat_at + 8), tex)
        block = b[mat_at:mat_at + MATGRP_SIZE] + mats
    else:
        end, block = mat_at, b''
    delta = (mat_at + len(block)) - end
    head = bytearray(b[:mat_at])
    for k in range(nobj):
        p = infos + k * OBJINFO_SIZE + 36
        struct.pack_into('<i', head, p, I(head, p) + delta)
    first = I(b, infos + 36)
    tm41, tm42 = I(b, first + OB_TM + 48), I(b, first + OB_TM + 52)
    physique = I(b, first + OB_PHYSIQUE)
    return bytes(head) + block + b[end:], (tm41 / 256, tm42 / 256), bool(physique)


def main():
    args = sys.argv[1:]
    also, server, report = [], None, None
    pos = []
    k = 0
    while k < len(args):
        if args[k] == '--also':
            also.append(args[k + 1]); k += 2
        elif args[k] == '--server':
            server = args[k + 1]; k += 2
        elif args[k] == '--report':
            report = args[k + 1]; k += 2
        else:
            pos.append(args[k]); k += 1
    old_root, cur_root, outs = pos[0], pos[1], pos[2:]
    old = Finder(old_root)
    tex = Textures([old_root] + also, cur_root, outs)
    curf = Finder(cur_root)

    fields = []
    stage_rects = []
    for folder, name, new, origin, extra in MAPS:
        src = old.find('Field\\%s\\%s.smd' % (folder, name))
        if not src:
            print('missing', folder, name)
            continue
        ox, oz = OFFSET[0] + extra[0], OFFSET[1] + extra[1]
        data, info, (voff, nv, nf) = port_stage(src, ox, oz, tex)
        rel = 'Field\\old\\%s\\%s.smd' % (folder, new)
        for out in outs + ([server] if server else []):
            dst = os.path.join(out, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            with open(dst, 'wb') as fp:
                fp.write(data)
        l, t, r, btm = [v - (ox if i % 2 == 0 else oz) for i, v in enumerate(info['rect'])]
        stage_rects.append((len(fields), l, t, r, btm, folder.lower()))
        # minimap and title images: the old client's own, else today's map of the same place
        for kind, pat in (('map', 'Field\\map\\%s.tga'), ('title', 'Field\\Title\\%st.tga')):
            img = old.find(pat % name)
            for a in also:
                if not img:
                    img = Finder(a).find(pat % name)
            if not img:
                img = curf.find(pat % name)
            if img:
                for out in outs:
                    dst = os.path.join(out, pat % new)
                    os.makedirs(os.path.dirname(dst), exist_ok=True)
                    shutil.copyfile(img, dst)
            info[kind] = bool(img)
        info.update(folder=folder, old=name, name=new, origin=origin, offset=[ox, oz], objects=[])
        info['want'] = [(l + r) / 2, (t + btm) / 2]
        info['_data'] = (data, voff, nv, nf)
        fields.append(info)
        print('%-20s %-6s v%-6d f%-6d mats%-4d area%d tail%-7d rect %s' % (
            new, folder, nv, nf, info['materials'], info['area_width'], info['stale_tail'],
            [int(v) for v in info['rect']]))

    # props: every model .smd in the old map folders goes to the map that contains it
    for root, _, files in os.walk(os.path.join(old_root, 'Field')):
        for f in sorted(files):
            rel = os.path.relpath(os.path.join(root, f), os.path.join(old_root, 'Field'))
            if not f.lower().endswith('.smd') or rel.lower() in SKIP_MODELS:
                continue
            m = port_model(os.path.join(root, f), tex)
            if not m:
                continue
            data, (x, z), physique = m
            home = [s for s in stage_rects if s[1] <= x <= s[3] and s[2] <= z <= s[4]]
            home.sort(key=lambda s: s[5] != rel.split('\\')[0].lower())     # a map of its own folder first
            for pre, mapname in PROP_HOME.items():
                if f.lower().startswith(pre):
                    home.sort(key=lambda s: fields[s[0]]['name'] != mapname)
            if not home:
                print('prop not on a map:', rel, int(x), int(z))
                continue
            if physique and not os.path.exists(os.path.join(root, f[:-4] + '.smb')):
                print('prop needs bones it does not have:', rel)
                continue
            fi = fields[home[0][0]]
            relout = 'Field\\old\\' + rel
            for out in outs:
                dst = os.path.join(out, relout)
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                with open(dst, 'wb') as fp:
                    fp.write(data)
                if physique:
                    shutil.copyfile(os.path.join(root, f[:-4] + '.smb'), dst[:-4] + '.smb')
            fi['objects'].append(['old\\' + rel[:-4] + '.ASE', 1 if physique else 0])

    tex.write()

    for fi in fields:
        data, voff, nv, nf = fi.pop('_data')
        ox, oz = fi['offset']
        want = CENTRES.get(fi['origin'], fi.pop('want'))
        fi.pop('want', None)
        cx, cz, exact = floor_point(data, voff, nv, nf, want[0] + ox, want[1] + oz)
        fi['centre'] = [cx, cz]
        fi['centre_on_floor'] = exact
    print('textures: %d copied, %d kept, %d missing' % (len(tex.copied), tex.kept, len(tex.missing)))
    if report:
        with open(report, 'w', encoding='utf-8') as fp:
            json.dump(dict(fields=fields, missing=sorted(tex.missing)), fp, indent=1)


if __name__ == '__main__':
    main()
