#!/usr/bin/env python3
"""
png2tim - converts a PNG to a PlayStation TIM.

Indexed (palette) PNGs keep their palette order, so CLUT indices stay the same. 
Full colour PNGs are converted to 16bpp, or to 4/8bpp with -bpp if they use few enough colours.

Examples:
  png2tim.py grass_1.png -org 384 0 -plt 320 400                 # 4/8bpp from palette size
  png2tim.py car2.png -bpp 8 -org 576 256 -plt 320 480 -o car2.tim
  png2tim.py sky2.png -bpp 16 -org 512 0
  png2tim.py grass_1.png --like grass_1.tim                     # same settings as an existing TIM
  png2tim.py green.png --like green.tim -pal green.tim          # ...and keep its exact CLUT
  png2tim.py green.png -bpp 8 -org 448 256 -plt 320 487 -pal cars.act

Palettes: an indexed PNG's palette is used as-is. To keep a fixed CLUT layout
(e.g. the car body colour ranges used by vehicle_colour.c), pass -pal with a
TIM or a Photoshop colour table (.ACT, from Image > Mode > Color Table > Save);
each pixel is mapped to the (last) entry with the same colour.

Transparency (on the PlayStation, colour 0 = pure black is transparent unless
its STP bit is set):
  default         pure black is made opaque (STP set), as the plugin exports
  -black-transp   leave pure black transparent
  -semi           set STP on all non-black colours (semi-transparent when drawn
                  with semi-transparency enabled)
  -tindex N       palette index N is transparent
  -tcol R G B     colour R,G,B is transparent
  -alpha [T]      pixels with alpha < T (default 128) are transparent
"""
import argparse, struct, sys, zlib

# ---------------------------------------------------------------- PNG reading

def read_png(path):
    """Returns (width, height, pixels, palette) where pixels are palette indices
    (palette = list of (r,g,b,a)) or (r,g,b,a) tuples when palette is None."""
    data = open(path, 'rb').read()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        sys.exit('%s: not a PNG file' % path)
    pos, idat, plte, trns = 8, b'', None, None
    while pos < len(data):
        ln, typ = struct.unpack_from('>I4s', data, pos)
        body = data[pos + 8:pos + 8 + ln]
        pos += 12 + ln
        if typ == b'IHDR':
            w, h, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', body)
        elif typ == b'PLTE':
            plte = [tuple(body[i:i + 3]) for i in range(0, len(body), 3)]
        elif typ == b'tRNS':
            trns = body
        elif typ == b'IDAT':
            idat += body
        elif typ == b'IEND':
            break
    if interlace:
        sys.exit('%s: interlaced PNGs are not supported' % path)
    if depth == 16:
        sys.exit('%s: 16-bit-per-channel PNGs are not supported' % path)

    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}[ctype]
    bits = depth * channels
    stride = (w * bits + 7) // 8
    bpp = max(1, bits // 8)
    raw = zlib.decompress(idat)
    rows, prev = [], bytearray(stride)
    for y in range(h):
        f = raw[y * (stride + 1)]
        line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if f == 1: line[i] = (line[i] + a) & 255
            elif f == 2: line[i] = (line[i] + b) & 255
            elif f == 3: line[i] = (line[i] + (a + b) // 2) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        rows.append(line)
        prev = line

    def samples(line):
        if depth >= 8:
            return list(line)
        per = 8 // depth
        mask = (1 << depth) - 1
        return [(line[i // per] >> (8 - depth * (i % per + 1))) & mask for i in range(w * channels)]

    pixels = []
    for line in rows:
        s = samples(line)[:w * channels]
        if ctype == 3:
            pixels += s
        else:
            scale = 255 // ((1 << depth) - 1) if depth < 8 else 1
            for x in range(w):
                v = s[x * channels:(x + 1) * channels]
                if ctype == 0: px = (v[0] * scale,) * 3 + (255,)
                elif ctype == 4: px = (v[0],) * 3 + (v[1],)
                elif ctype == 2: px = tuple(v) + (255,)
                else: px = tuple(v)
                if trns and ctype == 2 and len(trns) == 6 and px[:3] == struct.unpack('>HHH', trns):
                    px = px[:3] + (0,)
                pixels.append(px)

    palette = None
    if ctype == 3:
        alpha = list(trns or b'')
        palette = [c + ((alpha[i] if i < len(alpha) else 255),) for i, c in enumerate(plte)]
    return w, h, pixels, palette

# ---------------------------------------------------------------- conversion

def to_psx(r, g, b, stp):
    return (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10 | (0x8000 if stp else 0)


def colour_word(rgba, transparent, opts):
    """PlayStation 15-bit colour + STP bit for one colour."""
    if transparent:
        return 0x0000
    c = to_psx(*rgba[:3], stp=False)
    if c == 0:
        return 0x0000 if opts.black_transp else 0x8000
    return c | (0x8000 if opts.semi else 0)


def is_transparent(rgba, index, opts):
    if opts.tindex is not None and index == opts.tindex:
        return True
    if opts.tcol is not None and tuple(rgba[:3]) == tuple(opts.tcol):
        return True
    if opts.alpha is not None and rgba[3] < opts.alpha:
        return True
    return False


def read_palette(path):
    """Palette as a list of (r,g,b,a) from a TIM's CLUT or a Photoshop .ACT colour table.
    TIM entries keep their STP bit (as alpha 254/255) so it can be restored exactly."""
    d = open(path, 'rb').read()
    if d[:4] == b'\x10\0\0\0':
        flags = struct.unpack_from('<I', d, 4)[0]
        if not flags & 8:
            sys.exit('%s: TIM has no CLUT' % path)
        ln, cx, cy, cw, ch = struct.unpack_from('<IHHHH', d, 8)
        words = struct.unpack_from('<%dH' % (cw * ch), d, 20)
        return [((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3, 254 if c & 0x8000 else 255)
                for c in words], list(words)
    if len(d) in (768, 772):
        n = struct.unpack_from('>H', d, 768)[0] if len(d) == 772 else 256
        return [tuple(d[i * 3:i * 3 + 3]) + (255,) for i in range(n or 256)], None
    sys.exit('%s: not a TIM or .ACT colour table' % path)


def remap(pixels, palette, target, nearest=False):
    """Maps pixels (indices into palette, or RGBA) onto the target palette by
    15-bit colour. A colour listed more than once maps to its last entry, as
    the Photoshop plugin does (so image colours win over fixed colours at the
    start of the palette), except that padding (the run of identical entries
    filling the end of the palette) is only used if nothing earlier matches."""
    keys = [to_psx(*c[:3], stp=False) for c in target]
    pad = len(keys)
    while pad > 1 and keys[pad - 1] == keys[pad - 2]:
        pad -= 1
    if pad < len(keys):
        pad -= 1  # the run starts one entry earlier than the last mismatch
    lookup = {}
    for i in range(len(keys) - 1, pad - 1, -1):
        lookup[keys[i]] = pad          # first padding entry, as a fallback
    for i in range(pad):
        lookup[keys[i]] = i
    out = []
    for p in pixels:
        c = palette[p] if palette is not None else p
        k = to_psx(*c[:3], stp=False)
        if k not in lookup:
            if not nearest:
                sys.exit('colour %02x%02x%02x is not in the -pal palette (use -nearest to map it '
                         'to the closest one)' % tuple(c[:3]))
            lookup[k] = min(range(pad), key=lambda i: sum((a - b) ** 2 for a, b in zip(c[:3], target[i][:3])))
        out.append(lookup[k])
    return out


def convert(opts):
    w, h, pixels, palette = read_png(opts.png)
    fixed_clut = None

    if opts.pal:
        target, fixed_clut = read_palette(opts.pal)
        pixels = remap(pixels, palette, target, opts.nearest)
        palette = target
        if opts.bpp is None:
            opts.bpp = 4 if len(target) <= 16 else 8

    if palette is None and opts.bpp in (4, 8):
        # Full colour to 4/8bpp: build a palette in order of first appearance.
        colours = []
        seen = {}
        for px in pixels:
            if px not in seen:
                seen[px] = len(colours)
                colours.append(px)
        limit = 16 if opts.bpp == 4 else 256
        if len(colours) > limit:
            sys.exit('%s: %d colours, too many for %dbpp (save it as an indexed PNG, '
                     'or use -pal with -nearest)' % (opts.png, len(colours), opts.bpp))
        palette = colours
        pixels = [seen[px] for px in pixels]

    if opts.bpp is None:
        opts.bpp = 16 if palette is None else (4 if max(pixels) < 16 else 8)

    if opts.bpp in (4, 8):
        if palette is None:
            sys.exit('%s: no palette for %dbpp' % (opts.png, opts.bpp))
        if opts.plt is None:
            sys.exit('%s: -plt X Y (CLUT position) is required for %dbpp' % (opts.png, opts.bpp))
        n = 16 if opts.bpp == 4 else 256
        if max(pixels) >= n:
            sys.exit('%s: uses palette index %d, too high for %dbpp' % (opts.png, max(pixels), opts.bpp))
        if fixed_clut is not None and len(fixed_clut) == n:
            clut = list(fixed_clut)  # CLUT from a TIM is reused exactly, STP bits included
        else:
            clut = [colour_word(c, is_transparent(c, i, opts), opts) for i, c in enumerate(palette[:n])]
            clut += [colour_word((0, 0, 0, 255), False, opts)] * (n - len(clut))
        per = 16 // opts.bpp
        if w % per:
            sys.exit('%s: width %d must be a multiple of %d for %dbpp' % (opts.png, w, per, opts.bpp))
        if opts.bpp == 4:
            img = bytes(pixels[i] | pixels[i + 1] << 4 for i in range(0, len(pixels), 2))
        else:
            img = bytes(pixels)
        words_w = w // per
    else:
        if palette is not None:
            cols = [colour_word(palette[i], is_transparent(palette[i], i, opts), opts) for i in pixels]
        else:
            cols = [colour_word(px, is_transparent(px, None, opts), opts) for px in pixels]
        img = struct.pack('<%dH' % len(cols), *cols)
        clut = None
        words_w = w

    flags = {4: 0, 8: 1, 16: 2}[opts.bpp] | (8 if clut else 0)
    out = struct.pack('<II', 0x10, flags)
    if clut:
        cw = len(clut)
        out += struct.pack('<IHHHH', 12 + cw * 2, opts.plt[0], opts.plt[1], cw, 1)
        out += struct.pack('<%dH' % cw, *clut)
    out += struct.pack('<IHHHH', 12 + len(img), opts.org[0], opts.org[1], words_w, h) + img
    open(opts.output, 'wb').write(out)
    print('%s: %dx%d %dbpp at (%d,%d)%s -> %s' % (
        opts.png, w, h, opts.bpp, opts.org[0], opts.org[1],
        ', CLUT at (%d,%d)' % tuple(opts.plt) if clut else '', opts.output))


def settings_from_tim(path):
    """bpp, image position and CLUT position of an existing TIM."""
    d = open(path, 'rb').read()
    magic, flags = struct.unpack_from('<II', d, 0)
    if magic != 0x10:
        sys.exit('%s: not a TIM file' % path)
    p, plt = 8, None
    if flags & 8:
        ln, cx, cy = struct.unpack_from('<IHH', d, p)
        plt = [cx, cy]
        p += ln
    x, y = struct.unpack_from('<HH', d, p + 4)
    return {0: 4, 1: 8, 2: 16}[flags & 7], [x, y], plt


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[1],
                                 formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog='\n'.join(__doc__.split('\n')[2:]))
    ap.add_argument('png')
    ap.add_argument('-o', dest='output', help='output file (default: PNG name with .tim)')
    ap.add_argument('-bpp', type=int, choices=(4, 8, 16), help='colour depth (default: from the image)')
    ap.add_argument('-org', type=int, nargs=2, metavar=('X', 'Y'), help='image position in VRAM')
    ap.add_argument('-plt', type=int, nargs=2, metavar=('X', 'Y'), help='CLUT position in VRAM')
    ap.add_argument('--like', metavar='TIM', help='take bpp and positions from an existing TIM')
    ap.add_argument('-pal', metavar='FILE',
                    help='use this palette (a TIM\'s CLUT, or a Photoshop .ACT colour table) '
                         'and map the image onto it')
    ap.add_argument('-nearest', action='store_true',
                    help='with -pal: map colours not in the palette to the closest entry')
    ap.add_argument('-black-transp', action='store_true', help='leave pure black transparent')
    ap.add_argument('-semi', action='store_true', help='set STP on non-black colours')
    ap.add_argument('-tindex', type=int, metavar='N', help='palette index to make transparent')
    ap.add_argument('-tcol', type=int, nargs=3, metavar=('R', 'G', 'B'), help='colour to make transparent')
    ap.add_argument('-alpha', type=int, nargs='?', const=128, metavar='T',
                    help='alpha below T (default 128) is transparent')
    opts = ap.parse_args()

    if opts.like:
        bpp, org, plt = settings_from_tim(opts.like)
        opts.bpp = opts.bpp or bpp
        opts.org = opts.org or org
        opts.plt = opts.plt or plt
    if opts.org is None:
        ap.error('-org X Y is required (or --like)')
    if opts.output is None:
        opts.output = opts.png.rsplit('.', 1)[0] + '.tim'
    convert(opts)


if __name__ == '__main__':
    main()
