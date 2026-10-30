"""Makes smaller copies of the big start-up pictures and music for the browser build.

The browser downloads every data file the game opens, so size is loading time:
- Loading screens (StartImage/Login/Loading/*.bmp): the game shows only the
  top-left 800x600 of each (HoLogin.cpp: DrawSprite(..., 0, 0, 800, 600)), but most
  are 1920x1280 (7 MB). Cropped to 800x600 they look exactly the same (1.4 MB).
  The game's BMPs may have a scrambled file header ("A8": byte i + i*i); the copy
  gets a plain "BM" header, which the game also reads (smTexture.cpp new_LoadDib).
- Music (Intro.wav, CharacterSelect.wav): 16-bit PCM made mono at 22050 Hz.

Usage: python web\\shrink_media.py <data folder> <output folder>
The output folder gets the same relative paths; copy them over the data folder,
then rebuild the pack (web\\make_pack.py) and the manifest (web\\make_manifest.py).
"""
import os, struct, sys, array

LOADING = ['Map.bmp', 'map-all-all.bmp', 'map-all-mrin.bmp', 'map-all-tscr.bmp', 'map-ch-ac.bmp', 'map-ch-at.bmp',
           'map-ch-ft.bmp', 'map-ch-knight.bmp', 'map-ch-meca.bmp', 'map-ch-mg.bmp', 'map-ch-pk.bmp', 'map-ch-pt.bmp',
           'map-logo-RNS2.bmp', 'map-logo-RNS.bmp', 'map-SOD.bmp', 'map-tw-nvsc.bmp', 'map-tw-pillai.bmp',
           'map-tw-rica.bmp', 'map-user_001.bmp']
MUSIC = ['StartImage/Opening/Intro.wav', 'StartImage/login/CharacterSelect.wav']
SHOWN_W, SHOWN_H = 800, 600


def find(data, rel):
    """Path of rel in data, matching names without regard to case (the game's are mixed)."""
    cur = data
    for part in rel.split('/'):
        names = {n.lower(): n for n in os.listdir(cur)}
        if part.lower() not in names:
            return None
        cur = os.path.join(cur, names[part.lower()])
    return cur


def crop_bmp(src, dst):
    b = bytearray(open(src, 'rb').read())
    if b[:2] == b'A8':                                  # scrambled file header (its type bytes stay scrambled)
        for i in range(2, 14):
            b[i] = (b[i] - i * i) & 0xFF
    elif b[:2] not in (b'BM', b'C8'):
        return 'not a BMP'
    off = struct.unpack_from('<I', b, 10)[0]
    hsize, w, h, planes, bits, comp = struct.unpack_from('<IiiHHI', b, 14)
    if bits != 24 or comp != 0 or h <= 0:
        return 'skipped (%d-bit, compression %d, height %d)' % (bits, comp, h)
    if w <= SHOWN_W and h <= SHOWN_H:
        return 'already %dx%d' % (w, h)
    stride = (w * 3 + 3) & ~3
    nw, nh = min(w, SHOWN_W), min(h, SHOWN_H)
    nstride = (nw * 3 + 3) & ~3
    rows = []
    # bottom-up rows: the top nh rows of the picture are the last nh rows in the file
    for r in range(h - nh, h):
        start = off + r * stride
        rows.append(bytes(b[start:start + nw * 3]) + b'\0' * (nstride - nw * 3))
    pixels = b''.join(rows)
    info = bytearray(b[14:14 + hsize])
    struct.pack_into('<ii', info, 4, nw, nh)
    if hsize >= 24:
        struct.pack_into('<I', info, 20, len(pixels))    # biSizeImage
    noff = 14 + len(info)
    out = b'BM' + struct.pack('<IHHI', noff + len(pixels), 0, 0, noff) + bytes(info) + pixels
    open(dst, 'wb').write(out)
    return '%dx%d -> %dx%d, %.1f -> %.1f MB' % (w, h, nw, nh, len(b) / 1e6, len(out) / 1e6)


def mono22(src, dst):
    b = open(src, 'rb').read()
    i, fmt, data = 12, None, None
    while i + 8 <= len(b):
        cid, size = b[i:i + 4], struct.unpack_from('<I', b, i + 4)[0]
        if cid == b'fmt ':
            fmt = struct.unpack_from('<HHIIHH', b, i + 8)
        elif cid == b'data':
            data = b[i + 8:i + 8 + size]
        i += 8 + size + (size & 1)
    if not fmt or data is None:
        return 'no fmt/data chunk'
    tag, ch, rate, _, _, bits = fmt
    if tag != 1 or bits != 16:
        return 'skipped (format %d, %d-bit)' % (tag, bits)
    s = array.array('h')
    s.frombytes(data[:len(data) - len(data) % (2 * ch)])
    if sys.byteorder != 'little':
        s.byteswap()
    if ch == 2:
        m = [(s[k] + s[k + 1]) >> 1 for k in range(0, len(s), 2)]
    else:
        m = list(s)
    step = 1
    while rate // step > 22050 and rate % (step * 2) == 0:
        step *= 2
    if step > 1:                                        # average neighbours: a simple low-pass before dropping samples
        m = [sum(m[k:k + step]) // len(m[k:k + step]) for k in range(0, len(m), step)]
    out_rate = rate // step
    pcm = array.array('h', m)
    if sys.byteorder != 'little':
        pcm.byteswap()
    pcm = pcm.tobytes()
    fmt_chunk = b'fmt ' + struct.pack('<IHHIIHH', 16, 1, 1, out_rate, out_rate * 2, 2, 16)
    data_chunk = b'data' + struct.pack('<I', len(pcm)) + pcm
    body = b'WAVE' + fmt_chunk + data_chunk
    open(dst, 'wb').write(b'RIFF' + struct.pack('<I', len(body)) + body)
    return '%d Hz %d ch -> %d Hz mono, %.1f -> %.1f MB' % (rate, ch, out_rate, len(b) / 1e6, (len(body) + 8) / 1e6)


def main():
    data, out = sys.argv[1], sys.argv[2]
    jobs = [('StartImage/login/Loading/' + n, crop_bmp) for n in LOADING] + [(m, mono22) for m in MUSIC]
    for rel, fn in jobs:
        src = find(data, rel)
        if not src:
            print('%-45s missing' % rel)
            continue
        relr = os.path.relpath(src, data)
        dst = os.path.join(out, relr)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        print('%-45s %s' % (relr.replace('\\', '/'), fn(src, dst)))


if __name__ == '__main__':
    main()
