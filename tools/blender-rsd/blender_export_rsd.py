"""
This script exports PlayStation SDK compatible RSD,PLY,MAT files from Blender.
Supports normals, colors and texture mapped triangles.
Only one mesh can be exported at a time.

Port to Blender 4.x of Lameguy64's Blender-RSD-Plugin 2.0.0
(https://github.com/Lameguy64/Blender-RSD-Plugin, GPL-3.0, see LICENSE).
The files written are the same, byte for byte, as the original wrote in
Blender 2.76 for the same mesh. Blender 2.8+ works differently underneath, so:
- Faces are split into triangles/quads ("tessfaces") and normals are
  calculated here the way Blender 2.7x did, including its float rounding.
- Blender 2.8+ has no per-face images. A face's texture is the image of the
  first Image Texture node in the face's material (the face still needs UVs).
- Vertex colours are the active colour attribute (face corner or vertex).
- The Scale Factor option does nothing, as in the original.
"""

import math
import struct

import bpy
import mathutils

from bpy.props import (StringProperty,
                       BoolProperty,
                       FloatProperty,
                       )

from bpy_extras.io_utils import ExportHelper

bl_info = {
    "name":         "Export: Playstation RSD,PLY,MAT Model Format",
    "author":       "Lameguy64 plugin ported to Blender 4",
    "blender":      (4, 2, 0),
    "version":      (2, 1, 0),
    "location":     "File > Export",
    "description":  "Export mesh to PlayStation SDK compatible RSD,PLY,MAT format",
    "support":      "COMMUNITY",
    "category":     "Import-Export"
}


# ---------------------------------------------------------------------------
# Blender 2.7x geometry, reproduced exactly
#
# The normals and the splitting of faces with more than 4 sides follow
# Blender 2.76's C code, with its float rounding (32-bit build: each
# expression is evaluated at higher precision and rounded to a 32-bit float
# when stored), so that the numbers in the files are the same as the
# original plug-in wrote.
# ---------------------------------------------------------------------------

def _f32(x):
    """Rounds a Python float to 32-bit float precision."""
    return struct.unpack('<f', struct.pack('<f', x))[0]


_SHORT_TO_FLOAT = _f32(1.0 / 32767.0)


def _normalize(n):
    """normalize_v3(): returns (normal, length)."""
    d = _f32(n[0] * n[0] + n[1] * n[1] + n[2] * n[2])
    if d > 1e-35:
        d = _f32(math.sqrt(d))
        inv = _f32(1.0 / d)
        return [_f32(c * inv) for c in n], d
    return [0.0, 0.0, 0.0], 0.0


def _tri_quad_normal(cos):
    """normal_tri_v3() / normal_quad_v3(): a tessface's normal."""
    if len(cos) == 3:
        v1, v2, v3 = cos
        n1 = [_f32(v1[i] - v2[i]) for i in range(3)]
        n2 = [_f32(v2[i] - v3[i]) for i in range(3)]
    else:
        v1, v2, v3, v4 = cos
        n1 = [_f32(v1[i] - v3[i]) for i in range(3)]
        n2 = [_f32(v2[i] - v4[i]) for i in range(3)]
    n = [_f32(n1[1] * n2[2] - n1[2] * n2[1]),
         _f32(n1[2] * n2[0] - n1[0] * n2[2]),
         _f32(n1[0] * n2[1] - n1[1] * n2[0])]
    return _normalize(n)[0]


def _newell_normal(cos):
    """Polygon normal by Newell's method (add_newell_cross_v3_v3v3)."""
    n = [0.0, 0.0, 0.0]
    vp = cos[-1]
    for vc in cos:
        n[0] = _f32(n[0] + (vp[1] - vc[1]) * (vp[2] + vc[2]))
        n[1] = _f32(n[1] + (vp[2] - vc[2]) * (vp[0] + vc[0]))
        n[2] = _f32(n[2] + (vp[0] - vc[0]) * (vp[1] + vc[1]))
        vp = vc
    return _normalize(n)


def _saacos(fac):
    return math.pi if fac <= -1.0 else 0.0 if fac >= 1.0 else math.acos(fac)


def _vertex_normals(co, polys):
    """BKE_mesh_calc_normals_poly(): angle weighted vertex normals, stored as
    shorts like Blender 2.7x's MVert.no."""
    tn = [[0.0, 0.0, 0.0] for _ in co]
    for verts in polys:
        n = len(verts)
        pno, d = _newell_normal([co[v] for v in verts])
        if d == 0.0:
            pno = [0.0, 0.0, 1.0]
        edges = [None] * n
        for i in range(n):
            a, b = co[verts[i - 1]], co[verts[i]]
            edges[i - 1] = _normalize([_f32(a[k] - b[k]) for k in range(3)])[0]
        prev = edges[n - 1]
        for i in range(n):
            cur = edges[i]
            fac = _saacos(-(cur[0] * prev[0] + cur[1] * prev[1] + cur[2] * prev[2]))
            t = tn[verts[i]]
            for k in range(3):
                t[k] = _f32(t[k] + pno[k] * fac)
            prev = cur
    out = []
    for i, t in enumerate(tn):
        no, d = _normalize(t)
        if d == 0.0:
            no = _normalize(list(co[i]))[0]
        out.append(tuple(_f32(int(_f32(c * 32767.0)) * _SHORT_TO_FLOAT) for c in no))
    return out


def _project_2d(cos, normal):
    """axis_dominant_v3_to_m3_negate() + mul_v2_m3v3(): polygon to 2D."""
    n = [-c for c in normal]
    f = _f32(n[0] * n[0] + n[1] * n[1])
    if f > 1.1920928955078125e-07:  # FLT_EPSILON
        d = _f32(1.0 / _f32(math.sqrt(f)))
        r1 = [_f32(n[1] * d), _f32(-n[0] * d), 0.0]
        r2 = [_f32(-n[2] * r1[1]), _f32(n[2] * r1[0]), _f32(n[0] * r1[1] - n[1] * r1[0])]
    else:
        r1 = [-1.0 if n[2] < 0.0 else 1.0, 0.0, 0.0]
        r2 = [0.0, 1.0, 0.0]
    return [(_f32(r1[0] * c[0] + r1[1] * c[1] + r1[2] * c[2]),
             _f32(r2[0] * c[0] + r2[1] * c[1] + r2[2] * c[2])) for c in cos]


def _polyfill(coords):
    """BLI_polyfill_calc() (ear clipping, USE_CLIP_EVEN, USE_CLIP_SWEEP,
    USE_CONVEX_SKIP): triangles as index triples, in Blender 2.7x's order."""
    CONCAVE, CONVEX = -1, 1
    count = len(coords)
    nxt = [(i + 1) % count for i in range(count)]
    prv = [(i - 1) % count for i in range(count)]

    def span_sign(v1, v2, v3):
        # area_tri_signed_v2_alt_2x(v3, v2, v1)
        a = _f32(v3[0] * (v2[1] - v1[1]) + v2[0] * (v1[1] - v3[1]) + v1[0] * (v3[1] - v2[1]))
        return 0 if a == 0.0 else 1 if a > 0.0 else -1

    def sign_calc(i):
        return span_sign(coords[prv[i]], coords[i], coords[nxt[i]])

    sign = [sign_calc(i) for i in range(count)]
    concave = sum(1 for s in sign if s != CONVEX)
    # Points the ear test looks at (non-convex, not yet removed): the kd-tree
    candidates = set(i for i in range(count) if sign[i] != CONVEX) if concave else set()
    head = 0
    remaining = count
    tris = []

    def ear_tip_check(tip):
        if concave == 0:
            return True
        if sign[tip] == CONCAVE:
            return False
        tri = (tip, nxt[tip], prv[tip])
        vs = [coords[i] for i in tri]
        xmin, xmax = min(v[0] for v in vs), max(v[0] for v in vs)
        ymin, ymax = min(v[1] for v in vs), max(v[1] for v in vs)
        for i in candidates:
            co = coords[i]
            if (xmin <= co[0] <= xmax and ymin <= co[1] <= ymax
                    and span_sign(vs[0], vs[1], co) != CONCAVE
                    and span_sign(vs[1], vs[2], co) != CONCAVE
                    and span_sign(vs[2], vs[0], co) != CONCAVE
                    and i not in tri):
                return False
        return True

    def ear_tip_find(start, reverse):
        ear = start
        for _ in range(remaining):
            if ear_tip_check(ear):
                return ear
            ear = prv[ear] if reverse else nxt[ear]
        ear = start
        for _ in range(remaining):
            if sign[ear] != CONCAVE:
                return ear
            ear = nxt[ear]
        return ear

    ear_init = head
    reverse = False
    while remaining > 3:
        ear = ear_tip_find(ear_init, reverse)
        if ear != ear_init:
            reverse = not reverse
        if sign[ear] != CONVEX:
            concave -= 1
        p, n = prv[ear], nxt[ear]
        tris.append((p, ear, n))
        candidates.discard(ear)
        nxt[p], prv[n] = n, p
        if head == ear:
            head = n
        remaining -= 1
        for i in (p, n):
            if sign[i] != CONVEX:
                sign[i] = sign_calc(i)
                if sign[i] == CONVEX:
                    concave -= 1
                    candidates.discard(i)
        ear_init = prv[p] if reverse else nxt[n]

    tris.append((head, nxt[head], nxt[nxt[head]]))
    return tris


class _TessFace:
    """What Blender 2.7x called a tessface: a triangle or quad of a polygon."""
    __slots__ = ('vertices', 'loops', 'use_smooth', 'normal', 'polygon')

    def __init__(self, mesh, co, polygon, loops):
        self.loops = loops
        self.vertices = [mesh.loops[l].vertex_index for l in loops]
        self.use_smooth = polygon.use_smooth
        self.normal = mathutils.Vector(_tri_quad_normal([co[v] for v in self.vertices]))
        self.polygon = polygon


def _tessfaces(mesh, co):
    """BKE_mesh_recalc_tessellation(): triangles and quads stay as they are
    (quads with vertex 0 as their 3rd or 4th corner are rotated by two
    corners, see test_index_face()), bigger faces are split into triangles."""
    faces = []
    for p in mesh.polygons:
        loops = list(p.loop_indices)
        if len(loops) == 3:
            faces.append(_TessFace(mesh, co, p, loops))
        elif len(loops) == 4:
            if mesh.loops[loops[2]].vertex_index == 0 or mesh.loops[loops[3]].vertex_index == 0:
                loops = loops[2:] + loops[:2]
            faces.append(_TessFace(mesh, co, p, loops))
        elif len(loops) > 4:
            cos = [co[mesh.loops[l].vertex_index] for l in loops]
            normal, d = _newell_normal(cos)
            if d == 0.0:
                normal[2] = 1.0
            for tri in _polyfill(_project_2d(cos, normal)):
                faces.append(_TessFace(mesh, co, p, [loops[i] for i in tri]))
    return faces


def _material_image(material):
    """The image of the first Image Texture node of a material, or None."""
    if material is None or not material.use_nodes or material.node_tree is None:
        return None
    for node in material.node_tree.nodes:
        if node.type == 'TEX_IMAGE' and node.image is not None:
            return node.image
    return None


class _FaceUV:
    """Stands in for 2.7x MeshTextureFace: image and uv1..uv4 of a tessface."""
    __slots__ = ('image', 'uv1', 'uv2', 'uv3', 'uv4')

    def __init__(self, image, uvs):
        self.image = image
        uvs = uvs + [uvs[0]] * (4 - len(uvs))
        self.uv1, self.uv2, self.uv3, self.uv4 = uvs


class _FaceColours:
    """Stands in for 2.7x MeshColor: color1..color4 of a tessface."""
    __slots__ = ('color1', 'color2', 'color3', 'color4')

    def __init__(self, cols):
        cols = cols + [(0.0, 0.0, 0.0)] * (4 - len(cols))
        self.color1, self.color2, self.color3, self.color4 = cols


def _active_colours(mesh):
    attrs = mesh.color_attributes
    if len(attrs) == 0:
        return None
    attr = attrs.active_color or attrs[0]
    if attr.domain not in ('CORNER', 'POINT'):
        return None
    return attr


class ExportRSD(bpy.types.Operator, ExportHelper):

    bl_idname       = "export_mesh.rsd"
    bl_label        = "Export RSD,PLY,MAT"

    filename_ext    = ".rsd"
    filter_glob: StringProperty(default="*.rsd;*.ply;*.mat", options={'HIDDEN'})

    # Export options
    exp_applyModifiers: BoolProperty(
        name="Apply Modifiers",
        description="Apply modifiers to the exported mesh.",
        default=True,
        )

    exp_coloredTexPolys: BoolProperty(
        name="Colored Textured Polygons",
        description="Export all textured faces as vertex colored, "
                    "light source calculation on such faces will be "
                    "disabled however due to libgs limitations.",
        default=False,
        )

    exp_scaleFactor: FloatProperty(
        name="Scale Factor",
        description="Scale factor of exported mesh.",
        min=0.01, max=1000.0,
        default=1.0,
        )

    @classmethod
    def poll(cls, context):
        return context.object is not None and context.object.type == 'MESH'

    def execute(self, context):

        filepath = self.filepath
        filepath = filepath.replace(self.filename_ext, "")
        rsd_filepath = bpy.path.ensure_ext(filepath, self.filename_ext)
        ply_filepath = bpy.path.ensure_ext(filepath, '.ply')
        mat_filepath = bpy.path.ensure_ext(filepath, '.mat')

        # Get object context
        obj = context.object

        # Get mesh
        if self.exp_applyModifiers:
            src = obj.evaluated_get(context.evaluated_depsgraph_get())
        else:
            src = obj
        mesh = src.to_mesh()
        try:
            self.write(mesh, rsd_filepath, ply_filepath, mat_filepath)
        finally:
            src.to_mesh_clear()

        return {'FINISHED'}

    def write(self, mesh, rsd_filepath, ply_filepath, mat_filepath):

        co = [tuple(v.co) for v in mesh.vertices]
        normals = _vertex_normals(co, [[mesh.loops[l].vertex_index for l in p.loop_indices]
                                       for p in mesh.polygons])
        tessfaces = _tessfaces(mesh, co)

        # Write PLY file
        with open(ply_filepath, "w") as f:

            f.write("@PLY940102\n")
            f.write("%d %d %d\n" % (len(mesh.vertices), (len(mesh.vertices)+len(mesh.polygons)), len(tessfaces)))

            # Write vertices
            f.write("# Vertices\n")
            for v in mesh.vertices:
                f.write("%E %E %E\n" % (v.co.x, -v.co.z, v.co.y))

            # Write normals
            f.write("# Normals\n")
            f.write("# Smooth normals begin here\n")
            for n in normals:
                f.write("%E %E %E\n" % (n[0], -n[2], n[1]))

            f.write("# Flat normals begin here\n")
            flatnorms_start = len(mesh.vertices)
            for p in tessfaces:
                f.write("%E %E %E\n" % (p.normal.x, -p.normal.z, p.normal.y))

            # Write polygons
            f.write("# Polygon\n")
            for i,p in enumerate(tessfaces):

                # Write vertex indices
                if (len(p.vertices) == 3):
                    f.write("0 %d %d %d 0 " % (p.vertices[0], p.vertices[2], p.vertices[1]))
                elif (len(p.vertices) == 4):
                    f.write("1 %d %d %d %d " % (p.vertices[3], p.vertices[2], p.vertices[0], p.vertices[1]))

                # Write normal indices and shading mode
                if p.use_smooth:
                    if (len(p.vertices) == 3):
                        f.write("%d %d %d 0" % (p.vertices[0], p.vertices[2], p.vertices[1]))
                    elif (len(p.vertices) == 4):
                        f.write("%d %d %d %d" % (p.vertices[3], p.vertices[2], p.vertices[0], p.vertices[1]))
                else:
                    n = flatnorms_start+i
                    if (len(p.vertices) == 3):
                        f.write("%d %d %d 0" % (n, n, n))
                    elif (len(p.vertices) == 4):
                        f.write("%d %d %d %d" % (n, n, n, n))

                f.write("\n")

        # Write MAT file
        with open(mat_filepath, "w") as f:

            f.write("@MAT940801\n")
            f.write("%d\n" % len(tessfaces))

            # Get textures (each face's image comes from its material)
            uv_layer = mesh.uv_layers.active
            if uv_layer is not None:
                images = [_material_image(m) for m in mesh.materials]
                mesh_uvs = []
                for p in tessfaces:
                    mi = p.polygon.material_index
                    image = images[mi] if mi < len(images) else None
                    mesh_uvs.append(_FaceUV(image, [uv_layer.data[l].uv for l in p.loops]))
            else:
                mesh_uvs = None

            # Scan through all faces for assigned textures
            if mesh_uvs is not None:
                tex_table = []
                tex_files = []
                for uv in mesh_uvs:
                    if uv.image is not None:
                        addTex = True
                        texFileName = bpy.path.display_name_from_filepath(uv.image.filepath)
                        if len(tex_files)>0:
                            for c,t in enumerate(tex_files):
                                if t == texFileName:
                                    tex_table.append(c+1)
                                    addTex = False
                                    break
                        if addTex:
                            tex_files.append(texFileName)
                            tex_table.append(len(tex_files))
                    else:
                        tex_table.append(0)
            else:
                tex_table = None
                tex_files = None


            colours = _active_colours(mesh)
            if colours is not None:
                if colours.domain == 'CORNER':
                    mesh_cols = [_FaceColours([tuple(colours.data[l].color_srgb[:3]) for l in p.loops])
                                 for p in tessfaces]
                else:
                    mesh_cols = [_FaceColours([tuple(colours.data[v].color_srgb[:3]) for v in p.vertices])
                                 for p in tessfaces]
            else:
                mesh_cols = None

            for i,p in enumerate(tessfaces):

                f.write("%d\t 0 " % i)

                # Set flat or gouraud
                if p.use_smooth:
                    f.write("G ")
                else:
                    f.write("F ")

                # So that vertex colors will be correct for textured polys
                if tex_table is not None:
                    if (tex_table[i] > 0):
                        color_mul = 128.0
                        pol_textured = True
                    else:
                        color_mul = 255.0
                        pol_textured = False
                else:
                    color_mul = 255.0
                    pol_textured = False

                # Check if polygon is flat or gouraud shaded
                if mesh_cols is not None:
                    col = mesh_cols[i]
                    col = col.color1[:], col.color2[:], col.color3[:], col.color4[:]
                    # Check if polygon is flat shaded
                    if (col[0] == col[1]) and (col[1] == col[2]) and (col[2] == col[0]):
                        # is flat...
                        pol_gouraud = False
                    else:
                        # is gouraud...
                        pol_gouraud = True
                else:
                    pol_gouraud = False

                # Write texture coordinates
                if pol_textured:
                    if self.exp_coloredTexPolys:
                        if pol_gouraud:
                            f.write("H ")
                        else:
                            f.write("D ")
                    else:
                        f.write("T ")
                    f.write("%d " % (tex_table[i]-1))
                    if (len(p.vertices) == 3):
                        uv = (mesh_uvs[i].uv1,
                              mesh_uvs[i].uv3,
                              mesh_uvs[i].uv2,
                              )
                    elif (len(p.vertices) == 4):
                        uv = (mesh_uvs[i].uv4,
                              mesh_uvs[i].uv3,
                              mesh_uvs[i].uv1,
                              mesh_uvs[i].uv2
                              )
                    tex_w = mesh_uvs[i].image.size[0]-0.85
                    tex_h = mesh_uvs[i].image.size[1]-0.85
                    for j,c in enumerate(p.vertices):
                        f.write("%d %d " % (round(tex_w*uv[j].x), round(tex_h-(tex_h*uv[j].y))))
                    if (len(p.vertices) == 3):
                        f.write("0 0 ")
                else:
                    if pol_gouraud:
                        f.write("G ")
                    else:
                        f.write("C ")

                # Write vertex colors
                if mesh_cols is not None:
                    if (self.exp_coloredTexPolys) or (pol_textured == False):
                        if (pol_gouraud):
                            if (len(p.vertices) == 4):
                                index_tab = [ 3, 2, 0, 1 ]
                            else:
                                index_tab = [ 0, 2, 1 ]
                            for j,c in enumerate(p.vertices):
                                color = col[index_tab[j]]
                                color = (int(color[0]*color_mul), int(color[1]*color_mul), int(color[2]*color_mul))
                                f.write("%d %d %d " % (color[0], color[1], color[2]))
                        else:
                            color = col[0]
                            color = (int(color[0]*color_mul),
                                     int(color[1]*color_mul),
                                     int(color[2]*color_mul),
                                     )
                            f.write("%d %d %d " % color[:])
                else:
                    f.write("%d %d %d " % (color_mul, color_mul, color_mul))

                f.write("\n")

        # Write RSD file
        with open(rsd_filepath, "w") as f:
            f.write("@RSD940102\n")
            f.write("PLY=%s\n" % bpy.path.basename(ply_filepath))
            f.write("MAT=%s\n" % bpy.path.basename(mat_filepath))
            # Write texture files
            if tex_files is not None:
                f.write("NTEX=%d\n" % len(tex_files))
                for i,n in enumerate(tex_files):
                    f.write("TEX[%d]=%s\n" % (i, bpy.path.ensure_ext(n, '.tim')))
            else:
                f.write("NTEX=0\n")


# For registering to Blender menus
def menu_func(self, context):
    self.layout.operator(ExportRSD.bl_idname, text="PlayStation RSD (.rsd,.ply,.mat)")

def register():
    bpy.utils.register_class(ExportRSD)
    bpy.types.TOPBAR_MT_file_export.append(menu_func)

def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func)
    bpy.utils.unregister_class(ExportRSD)

if __name__ == "__main__":
    register()
