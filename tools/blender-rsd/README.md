# Blender RSD exporter (Blender 4.x)

Exports a Blender mesh as PlayStation SDK `.rsd`, `.ply` and `.mat` files,
which `rsdlink` turns into a TMD model (see `toolchain/rsdlink.md`).

This is a port to Blender 4.x of
[Lameguy64's Blender-RSD-Plugin](https://github.com/Lameguy64/Blender-RSD-Plugin)
2.0.0, the plug-in used in Blender 2.76 in the Net Yaroze VM. It writes the
same files as the original did: byte for byte for the same mesh.

Licence: GPL-3.0, like the original (see `LICENSE`).

## Installing

1. In Blender: *Edit > Preferences > Add-ons*, then the drop-down menu at the
   top right, *Install from Disk...*
2. Choose `tools/blender-rsd/blender_export_rsd.py`.
3. Tick **Export: Playstation RSD,PLY,MAT Model Format** to enable it.

Tested with Blender 4.5.3 LTS.

## Exporting

1. Select the mesh to export (the active object is exported; one mesh at a
   time).
2. *File > Export > PlayStation RSD (.rsd,.ply,.mat)*.
3. Choose the file name. The `.ply` and `.mat` are written next to the
   `.rsd` with the same name. Keep names to DOS 8.3 (e.g. `car2B00.rsd`),
   because `rsdlink` is a DOS program.

Options (in the side panel of the file browser):

| Option | Meaning |
|---|---|
| Apply Modifiers | Export the mesh with its modifiers applied (default: on). |
| Colored Textured Polygons | Export textured faces with their vertex colours too (no lighting on those faces in libgs). |
| Scale Factor | Does nothing, as in the original plug-in. Scale the model in Blender, or afterwards (e.g. `scale_rsd.py`). |

## Setting up the model

- **Shading:** smooth faces (*Shade Smooth*) are exported as gouraud shaded,
  flat faces as flat.
- **Textures:** a face is textured if the mesh has a UV map and the face's
  material has an **Image Texture** node with an image; the first such node
  in the material is used. The RSD lists the texture as the image's file name
  with `.tim` (e.g. `car2.png` becomes `car2.tim`), so give the TIM the same
  name (`png2tim` does this). The texture coordinates use the image's size,
  so the image file must be found by Blender; if it isn't, the coordinates
  come out as 0.
- **Vertex colours:** the active colour attribute (*Object Data > Color
  Attributes*), painted per face corner or per vertex. A face whose corners
  all have the same colour is exported flat coloured, otherwise gouraud.
  Faces without a texture and without vertex colours are white.
- Triangles and quads are exported as they are; faces with more sides are
  split into triangles.

## Opening Blender 2.7x files

Blender 2.8 and later drop the per-face images of Blender 2.7x files (the
UVs are kept). After opening an old `.blend`, give the textured faces a
material with an Image Texture node using the texture, then export.

## Differences from Blender 2.76

The port was checked by exporting the same meshes with the original plug-in
in Blender 2.76b and with this one in Blender 4.5.3: test meshes with
triangles, quads, 5- and 6-sided faces, smooth and flat shading, flat and
gouraud vertex colours, textures of different sizes and a modifier (with all
option combinations), and the models in `data/game`. All files were
identical, apart from one case that can't be reproduced:

- Blender 2.7x stored vertex normals in the `.blend` file. If the mesh was
  scaled or rotated with *Apply* in 2.7x, those stored normals were
  transformed rather than recalculated, and differ slightly (by 1/32767) from
  recalculated ones. Blender 4.x doesn't keep them, so the export uses
  recalculated normals. In this project that affects 4 vertices of
  `stand.blend`.
