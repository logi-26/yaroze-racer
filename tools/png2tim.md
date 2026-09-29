# png2tim

Converts a PNG image into a PlayStation TIM texture
You choose where the image and its CLUT (colour palette) go in
VRAM, and how transparency is handled.

It only needs Python 3, so it runs in WSL (or anywhere with Python).

## Running it

In WSL the tool is available as the `png2tim` command. It is a link to
`tools/png2tim.py` in this repo:

    ln -sfn /mnt/c/yaroze_vm_share/yaroze-racer/tools/png2tim.py ~/.local/bin/png2tim

Without the link, run the script directly:

    python3 tools/png2tim.py <options>

`png2tim --help` lists every option.

## Common tasks

Convert an image, giving the image and CLUT positions (like the plugin):

    png2tim grass_1.png -org 384 0 -plt 320 400

Re-export an edited image with the same settings as its existing TIM
(bit depth, image position and CLUT position are read from the TIM):

    png2tim grass_1.png --like grass_1.tim

Re-export and also keep the existing TIM's exact palette, e.g. for the cars,
whose body colours are changed in the game by CLUT index (vehicle_colour.c):

    png2tim green.png --like green.tim -pal green.tim

Convert a full colour image to 16-bit (no CLUT needed):

    png2tim sky2.png -org 512 0

The output is written next to the PNG with a `.tim` extension. Use
`-o file.tim` to choose another name. Existing files are overwritten.

## Options

| Option | Meaning |
|---|---|
| `-org X Y` | Position of the image in VRAM. Required (unless `--like` is used). |
| `-plt X Y` | Position of the CLUT in VRAM. Required for 4-bit and 8-bit images. |
| `-bpp 4\|8\|16` | Colour depth. Default: 4 or 8 for indexed PNGs (depending on the highest palette index used), 16 for full colour PNGs. |
| `--like TIM` | Take the colour depth and both positions from an existing TIM. Options given on the command line still win. |
| `-pal FILE` | Use a fixed palette from a TIM, or from a Photoshop colour table (`.ACT`). Each pixel is mapped to the palette entry with the same colour. |
| `-nearest` | With `-pal`: colours not in the palette are mapped to the closest entry instead of stopping with an error. |
| `-o FILE` | Output file name. |

VRAM positions: the image X is given in 16-bit VRAM units, so a 4-bit image
takes 1/4 of its pixel width in VRAM and an 8-bit image 1/2. CLUTs are 16x1
entries (4-bit) or 256x1 (8-bit). No overlap checking is done; the existing
TIMs in `data/` show the layout already in use.

## Transparency

On the PlayStation, pure black (0,0,0) is drawn as transparent unless its STP
bit is set. Colours with the STP bit set are drawn semi-transparent when the
game draws them with semi-transparency turned on.

| Option | Effect |
|---|---|
| (default) | Pure black is made **opaque** (STP bit set), which is what the Photoshop plugin exported for this project. |
| `-black-transp` | Leave pure black **transparent**. |
| `-semi` | Set the STP bit on all non-black colours (semi-transparent). |
| `-tindex N` | Make palette index N transparent. |
| `-tcol R G B` | Make the colour R,G,B transparent. |
| `-alpha [T]` | Make pixels with alpha below T transparent (default 128). For PNGs with an alpha channel. |

Transparent pixels are stored as pure black without the STP bit.

## Palettes and CLUT order

With an **indexed** (palette) PNG, the palette is used in its existing order,
so palette index N in the image is CLUT entry N in the TIM. This matters when
code changes CLUT entries by index, as the car colours do.

With a **full colour** PNG and `-bpp 4` or `-bpp 8`, a palette is built from
the colours in the order they first appear. If the image has more than 16 or
256 colours, reduce it first (save it as an indexed PNG) or use
`-pal` with `-nearest`.

With `-pal`, the palette file sets the CLUT. A TIM's CLUT is copied exactly,
STP bits included. If a colour appears in the palette more than once, the
last copy is used, apart from the black padding that fills the end of the
palette (this matches what the Photoshop plugin produced).

## Making textures in GIMP

1. Paint the texture.
2. *Image > Mode > Indexed...*: choose "Generate optimum palette" with a
   maximum of 16 colours (4-bit) or 256 (8-bit). To keep a fixed palette,
   choose "Use custom palette" instead.
3. *File > Export As...* and save as `.png`.
4. Convert: `png2tim texture.png -org X Y -plt X Y`, or
   `png2tim texture.png --like texture.tim` to replace an existing texture.

## Errors

| Message | Fix |
|---|---|
| `-org X Y is required` | Give the image position, or use `--like`. |
| `-plt X Y (CLUT position) is required` | 4-bit and 8-bit images need a CLUT position. |
| `N colours, too many for 8bpp` | Save the image as an indexed PNG, or use `-pal` with `-nearest`. |
| `uses palette index N, too high for 4bpp` | The image uses more than 16 palette entries; use `-bpp 8` or reduce the colours. |
| `width W must be a multiple of 4` | 4-bit images need a width that is a multiple of 4 (8-bit: 2). |
| `colour RRGGBB is not in the -pal palette` | The image has a colour the palette doesn't; add `-nearest`. |
| `interlaced PNGs are not supported` | Re-save the PNG without interlacing. |
