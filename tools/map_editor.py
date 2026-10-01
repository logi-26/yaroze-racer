#!/usr/bin/env python3
"""
Yaroze Racer - Ground Map Editor
Saves map data as a .h file for inclusion in game/ground.c

Controls:
  Left-click palette        select tile
  Q / Tab                   toggle Paint / Select mode
  W                         toggle Waypoint mode (the AI racing line)
  Paint mode:
    Left-click/drag grid    paint selected tile
    Right-click grid        erase to grass ('3')
  Select mode:
    Left-click/drag grid    draw selection rectangle
    Right-click             clear selection
    Ctrl+A                  select all tiles
    Delete                  fill selection with grass
    Ctrl+C                  copy selection to clipboard
    Ctrl+V                  enter Paste mode (hover for preview, click to place, Esc to cancel)
  Waypoint mode:
    Left-click              add a waypoint after the selected one (or at the end)
    Left-drag a waypoint    move it
    Shift+click a line      insert a waypoint on that segment
    Right-click a waypoint  delete it (or Delete for the selected one)
    Home                    make the selected waypoint the first one (start line)
    R                       reverse the race direction
    I                       import waypoints from a file (replaces them): any
                            { x, z } list, e.g. the L1 recorder's printf output
  Mouse wheel               zoom in / out (around the cursor)
  Middle-drag / arrow keys  pan the map
  F                         fit the whole map in the window
  S                         save (file dialog)
  Ctrl+S                    save to current file
  O                         open map file (smaller maps are padded with grass)
  ESC                       cancel paste / clear selection / quit
"""

import math
import re
import sys
import tkinter as tk
from pathlib import Path
from tkinter import filedialog

import pygame

# Paths
REPO_ROOT = Path(__file__).resolve().parent.parent
TRACK_DIR = REPO_ROOT / "data" / "game" / "track"
MAPS_DIR  = REPO_ROOT / "game"


def read_ground_h() -> tuple:
    """Map dimensions (cols, rows) and tile size in world units, from game/ground.h
    (GROUND_MAX_X, GROUND_MAX_Z and SEPERATION)."""
    cols, rows, tile_world = 64, 64, 1200
    try:
        text = (MAPS_DIR / 'ground.h').read_text(encoding='utf-8', errors='replace')
        mx = re.search(r'#define\s+GROUND_MAX_X\s+\(?\s*(\d+)', text)
        mz = re.search(r'#define\s+GROUND_MAX_Z\s+\(?\s*(\d+)', text)
        ms = re.search(r'#define\s+SEPERATION\s+\(?\s*(\d+)', text)
        if mx and mz:
            cols, rows = int(mx.group(1)), int(mz.group(1))
        if ms:
            tile_world = int(ms.group(1))
    except Exception as exc:
        print(f"Warning: could not read ground.h ({exc}), using {cols}x{rows} tiles of {tile_world}")
    return cols, rows, tile_world


# Map dimensions, and the size of a tile in world units
# Map tile [row][col] covers world X from row * TILE_WORLD and Z from col * TILE_WORLD
COLS, ROWS, TILE_WORLD = read_ground_h()

# Tiles the game counts as off the track (GetTerrainType)
OFF_TRACK = {'3', 'v'}

# Layout
PAL_W    = 240
PAL_COLS = 5
PAL_IMG  = 44
PAL_CW   = 48
PAL_CH   = 58
STATUS_H = 30
MARGIN   = 8

# Grid zoom (pixels per tile)
TILE_MIN = 6
TILE_MAX = 48
TILE_FIT_MAX = 24

# The palette needs this much height
MIN_WIN_H = MARGIN + 7 * PAL_CH + 40 + STATUS_H

# Colours
BG        = ( 28,  28,  40)
PANEL     = ( 42,  42,  58)
GRID_LN   = ( 55,  55,  75)
SEL_CLR   = (255, 215,  40)
STAT_BG   = ( 18,  18,  28)
TEXT_CLR  = (215, 215, 215)
SAND_CLR  = (200, 175,  95)
ERR_CLR   = (180,  60, 180)
HOVER_CLR = (255, 255, 255)
BOX_SEL   = ( 80, 160, 255)
PASTE_CLR = ( 80, 200,  80)
WP_LINE   = (  0, 220, 255)
WP_START  = ( 80, 255,  80)
WP_SHARP  = (255, 150,   0)
WP_OFF    = (255,  60,  60)
WP_TEXT   = (255, 255, 255)

# Mode constants
MODE_PAINT  = 'paint'
MODE_SELECT = 'select'
MODE_WAYPOINT = 'waypoint'

# Tile definitions
TILES = {
    '3': ('grass_1', 'grass_1.png',    0, 'Grass'),
    'v': (None,       None,            0, 'Sand'),
    '9': ('blank',   'blank.png',      0, 'Tarmac'),
    '1': ('st_1',    'st_1.png',       0, 'Straight 0'),
    '2': ('st_1',    'st_1.png',     180, 'Straight 180'),
    '6': ('st_1',    'st_1.png',      90, 'Straight 90'),
    '7': ('st_1',    'st_1.png',     270, 'Straight 270'),
    'i': ('st_01',   'st_01.png',     0,  'Straight NC 0'),
    'j': ('st_01',   'st_01.png',    90,  'Straight NC 90'),
    'k': ('st_01',   'st_01.png',   180,  'Straight NC 180'),
    'l': ('st_01',   'st_01.png',   270,  'Straight NC 270'),
    '4': ('line',    'line.png',      0,  'Start Line 0'),
    '5': ('line',    'line.png',    180,  'Start Line 180'),
    'h': ('grid',    'grid.png',      0,  'Grid 0'),
    'g': ('grid',    'grid.png',    180,  'Grid 180'),
    '8': ('t_1',     't_1.png',       0,  'Outer Turn 0'),
    'a': ('t_1',     't_1.png',      90,  'Outer Turn 90'),
    'c': ('t_1',     't_1.png',     180,  'Outer Turn 180'),
    'd': ('t_1',     't_1.png',     270,  'Outer Turn 270'),
    'b': ('t_00',    't_00.png',     0,   'Inner-A 0'),
    'e': ('t_00',    't_00.png',    90,   'Inner-A 90'),
    'f': ('t_00',    't_00.png',   180,   'Inner-A 180'),
    'm': ('t_00',    't_00.png',   270,   'Inner-A 270'),
    'n': ('t_01',    't_01.png',     0,   'Inner-B 0'),
    'p': ('t_01',    't_01.png',    90,   'Inner-B 90'),
    'q': ('t_01',    't_01.png',   180,   'Inner-B 180'),
    'r': ('t_01',    't_01.png',   270,   'Inner-B 270'),
    'o': ('t_02',    't_02.png',     0,   'Inner-C 0'),
    's': ('t_02',    't_02.png',    90,   'Inner-C 90'),
    't': ('t_02',    't_02.png',   180,   'Inner-C 180'),
    'u': ('t_02',    't_02.png',   270,   'Inner-C 270'),
}

PAL_ORDER = [
    '3', 'v', '9', '1', '2',
    '6', '7', 'i', 'j', 'k',
    'l', '4', '5', 'h', 'g',
    '8', 'a', 'c', 'd', 'b',
    'e', 'f', 'm', 'n', 'p',
    'q', 'r', 'o', 's', 't',
    'u',
]


_raw_images: dict = {}


def load_images(px: int) -> dict:
    """Load, rotate and scale every tile image to px×px. Returns {char: Surface}"""
    out: dict = {}
    for ch, (sub, fn, deg, _) in TILES.items():
        if sub is None:
            surf = pygame.Surface((px, px))
            surf.fill(SAND_CLR)
            out[ch] = surf
            continue
        key = (sub, fn, deg)
        if key not in _raw_images:
            path = TRACK_DIR / sub / fn
            try:
                src = pygame.image.load(str(path)).convert_alpha()
            except Exception as exc:
                print(f"Warning: could not load {path}: {exc}")
                src = pygame.Surface((64, 64))
                src.fill(ERR_CLR)
            _raw_images[key] = pygame.transform.rotate(src, -(deg + 90))
        out[ch] = pygame.transform.smoothscale(_raw_images[key], (px, px))
    return out


WAYPOINT_RE = re.compile(r'\{\s*(-?\d+)\s*,\s*(-?\d+)\s*\}')


def parse_waypoints(text: str) -> list:
    """The waypoints of a map file (its Waypoint array), or failing that every
    { x, z } pair in the text (e.g. the L1 recorder's printf output)."""
    m = re.search(r'Waypoint\s+\w+\s*\[[^\]]*\]\s*=\s*\{(.*?)\};', text, re.DOTALL)
    body = m.group(1) if m else text
    return [[int(x), int(z)] for x, z in WAYPOINT_RE.findall(body)]


def parse_map(src_path: Path) -> tuple:
    """Load any groundData* array (and its waypoints) from a .h or .c file.
    Returns (grid, (rows, cols) of the file, waypoints). A smaller map is padded with grass."""
    grid = [['3'] * COLS for _ in range(ROWS)]
    rows_in = cols_in = 0
    waypoints = []
    try:
        text = src_path.read_text(encoding='utf-8', errors='replace')
        if re.search(r'Waypoint\s+\w+\s*\[', text):
            waypoints = parse_waypoints(text)
        m = re.search(
            r'groundData\w+\s*\[.*?\]\s*\[.*?\]\s*=\s*\{(.*?)\};',
            text, re.DOTALL
        )
        if m:
            row_strs = re.findall(r'\{([^}]+)\}', m.group(1))
            rows_in = len(row_strs)
            for z, rs in enumerate(row_strs):
                cells = re.findall(r"'(.)'", rs)
                cols_in = max(cols_in, len(cells))
                if z >= ROWS:
                    continue
                for x, ch in enumerate(cells[:COLS]):
                    grid[z][x] = ch
    except Exception as exc:
        print(f"parse_map ({src_path.name}): {exc}")
    return grid, (rows_in, cols_in), waypoints


def save_map(grid: list, waypoints: list, dest: Path) -> None:
    """Write the map array and its AI waypoints to dest as a C header file."""
    stem  = dest.stem
    guard = re.sub(r'\W', '_', stem).upper() + '_H'
    # Derive the names from the filename: map_1 -> groundDataMap1, waypointsMap1, NUM_WAYPOINTS_MAP1
    parts      = re.split(r'[_\-]', stem)
    suffix     = ''.join(p.capitalize() for p in parts)
    array_name = 'groundData' + suffix
    wp_name    = 'waypoints' + suffix
    wp_count   = 'NUM_WAYPOINTS_' + suffix.upper()

    lines = [
        f'// {dest.name} — generated by tools/map_editor.py',
        '//',
        '// Include in game/ground.c with:',
        f'//   #include "{dest.name}"',
        '',
        f'#ifndef {guard}',
        f'#define {guard}',
        '',
        f'char {array_name}[{ROWS}][{COLS}] = {{',
    ]
    for z, row in enumerate(grid):
        cells = ','.join(f"'{c}'" for c in row)
        comma = ',' if z < ROWS - 1 else ''
        lines.append(f'\t{{{cells}}}{comma}')
    lines += ['};', '']

    # The AI needs at least one waypoint to drive to
    wps = waypoints if waypoints else [[0, 0]]
    lines += [
        '// AI waypoints: world (X, Z) in race order, the first one at the start line',
        f'// (map tile [row][col] covers world X from row * {TILE_WORLD} and Z from col * {TILE_WORLD})',
        f'#define {wp_count} ({len(wps)})',
        '',
        f'const Waypoint {wp_name}[{wp_count}] = {{',
    ]
    for i, (x, z) in enumerate(wps):
        comma = ',' if i < len(wps) - 1 else ''
        lines.append(f'\t{{ {x:6d}, {z:6d} }}{comma}')
    lines += ['};', '', f'#endif // {guard}', '']
    dest.write_text('\n'.join(lines), encoding='utf-8')


class Editor:
    def __init__(self):
        self._tk = tk.Tk()
        self._tk.withdraw()

        pygame.init()

        # Window size (the whole map at the largest zoom that fits the desktop)
        desk_w, desk_h = pygame.display.get_desktop_sizes()[0]
        fit = min((desk_w - 80 - PAL_W - MARGIN * 2) // COLS,
                  (desk_h - 120 - STATUS_H - MARGIN * 2) // ROWS,
                  TILE_FIT_MAX)
        self.tile = max(TILE_MIN, fit)
        win_w = PAL_W + MARGIN + min(COLS * self.tile, desk_w - 80 - PAL_W - MARGIN * 2) + MARGIN
        win_h = max(MIN_WIN_H, min(ROWS * self.tile, desk_h - 120 - STATUS_H - MARGIN * 2) + STATUS_H + MARGIN * 2)
        self.screen = pygame.display.set_mode((win_w, win_h), pygame.RESIZABLE)
        self.clock  = pygame.time.Clock()
        self.font_s = pygame.font.SysFont('monospace',  9)
        self.font_m = pygame.font.SysFont('monospace', 13)

        self.current_file: Path | None = None
        self.grid     = [['3'] * COLS for _ in range(ROWS)]
        self.selected = '3'
        self.status   = "O=open  S=save  Q=select mode  W=waypoints  Ctrl+C/V=copy/paste  Wheel=zoom  Middle-drag=pan  F=fit"
        self._update_title()

        self.pal_imgs  = load_images(PAL_IMG)
        self.grid_imgs_cache: dict = {}
        self.grid_imgs = self._grid_images()

        self.pal_rects = []
        for i, ch in enumerate(PAL_ORDER):
            col = i % PAL_COLS
            row = i // PAL_COLS
            rx  = MARGIN + col * PAL_CW
            ry  = MARGIN + row * PAL_CH
            self.pal_rects.append((ch, pygame.Rect(rx, ry, PAL_CW - 2, PAL_CH - 2)))

        # Grid viewport (screen rect) and scroll (pixels)
        self.view = pygame.Rect(0, 0, 0, 0)
        self.scroll_x = 0
        self.scroll_y = 0
        self.panning = False
        self._layout()

        # Paint state
        self.painting = False

        # Mode and selection state
        self.mode      = MODE_PAINT
        self.sel_drag  = False   # True while drag-selecting
        self.sel_start = None    # (row, col) drag origin
        self.sel_end   = None    # (row, col) current drag endpoint
        self.selection = None    # finalized (r0, c0, r1, c1) or None

        # Clipboard / paste state
        self.clipboard = None    # 2-D list of chars, or None
        self.pasting   = False   # True while waiting to place clipboard

        # AI waypoints: [world x, world z] in race order
        self.waypoints = []
        self.wp_sel    = None    # Index of the selected waypoint, or None
        self.wp_drag   = False   # True while dragging the selected waypoint


    def _update_title(self):
        name = self.current_file.name if self.current_file else 'Untitled'
        pygame.display.set_caption(f'Yaroze Racer — Map Editor  [{name}]  {COLS}×{ROWS}')


    def _grid_images(self):
        """Tile images at the current zoom (cached per zoom level)."""
        if self.tile not in self.grid_imgs_cache:
            self.grid_imgs_cache[self.tile] = load_images(self.tile)
        return self.grid_imgs_cache[self.tile]


    def _layout(self):
        """Place the grid viewport in the window (after a resize)."""
        win_w, win_h = self.screen.get_size()
        self.view = pygame.Rect(PAL_W + MARGIN, MARGIN,
                                max(1, win_w - PAL_W - MARGIN * 2),
                                max(1, win_h - STATUS_H - MARGIN * 2))
        self._clamp_scroll()


    def _clamp_scroll(self):
        self.scroll_x = max(0, min(self.scroll_x, COLS * self.tile - self.view.w))
        self.scroll_y = max(0, min(self.scroll_y, ROWS * self.tile - self.view.h))


    def _zoom(self, steps, mx, my):
        """Zoom in/out keeping the point under the mouse in place."""
        new_tile = self.tile
        for _ in range(abs(steps)):
            new_tile = new_tile + max(1, new_tile // 4) if steps > 0 else new_tile - max(1, new_tile // 5)
        new_tile = max(TILE_MIN, min(TILE_MAX, new_tile))
        if new_tile == self.tile:
            return
        if not self.view.collidepoint(mx, my):
            mx, my = self.view.center
        fx = (mx - self.view.x + self.scroll_x) / self.tile
        fy = (my - self.view.y + self.scroll_y) / self.tile
        self.tile = new_tile
        self.scroll_x = int(fx * self.tile) - (mx - self.view.x)
        self.scroll_y = int(fy * self.tile) - (my - self.view.y)
        self._clamp_scroll()
        self.grid_imgs = self._grid_images()


    def _fit(self):
        """Zoom so the whole map fits the viewport."""
        self.tile = max(TILE_MIN, min(TILE_MAX, self.view.w // COLS, self.view.h // ROWS))
        self.scroll_x = self.scroll_y = 0
        self.grid_imgs = self._grid_images()


    def _open_dialog(self):
        path = filedialog.askopenfilename(
            title='Open map file',
            initialdir=str(MAPS_DIR),
            filetypes=[('C Header / Source', '*.h *.c'), ('All files', '*.*')],
        )
        if path:
            p = Path(path)
            self.grid, (rows_in, cols_in), self.waypoints = parse_map(p)
            self.wp_sel = None
            self.current_file = p
            self._update_title()
            wp_str = f", {len(self.waypoints)} waypoints"
            if (rows_in, cols_in) == (ROWS, COLS):
                self.status = f"Opened {p.name}{wp_str}"
            elif rows_in <= ROWS and cols_in <= COLS:
                self.status = f"Opened {p.name} ({cols_in}×{rows_in}, padded with grass to {COLS}×{ROWS}){wp_str}"
            else:
                self.status = f"Opened {p.name} ({cols_in}×{rows_in}, CROPPED to {COLS}×{ROWS}){wp_str}"


    def _save_as_dialog(self):
        initial = self.current_file.name if self.current_file else 'map.h'
        path = filedialog.asksaveasfilename(
            title='Save map as',
            initialdir=str(MAPS_DIR),
            initialfile=initial,
            defaultextension='.h',
            filetypes=[('C Header', '*.h'), ('All files', '*.*')],
        )
        if path:
            p = Path(path)
            save_map(self.grid, self.waypoints, p)
            self.current_file = p
            self._update_title()
            self.status = f"Saved → {p.name}" + self._waypoint_warnings()


    def _save(self):
        if self.current_file:
            save_map(self.grid, self.waypoints, self.current_file)
            self.status = f"Saved → {self.current_file.name}" + self._waypoint_warnings()
        else:
            self._save_as_dialog()


    def _waypoint_warnings(self):
        """Problems with the waypoints the AI would run into (for the status after a save)."""
        if len(self.waypoints) < 2:
            return "   WARNING: the AI needs waypoints (W = waypoint mode)"
        off = sum(1 for i in range(len(self.waypoints)) if self._wp_off_track(i))
        if off:
            return f"   WARNING: {off} waypoint(s) off the track (red)"
        return ""


    def _toggle_waypoint_mode(self):
        if self.mode == MODE_WAYPOINT:
            self.mode    = MODE_PAINT
            self.wp_drag = False
            self.status  = "PAINT mode"
        else:
            self.mode      = MODE_WAYPOINT
            self.painting  = False
            self.pasting   = False
            self.sel_drag  = False
            self.selection = None
            self.status    = ("WAYPOINTS — click add  drag move  Shift+click line insert  "
                              "right-click delete  Home=start  R=reverse  I=import  W=back")


    # Map tile [row][col] covers world X from row * TILE_WORLD and Z
    # from col * TILE_WORLD, so world X runs down the grid and world Z across it

    def _world_to_screen(self, wx, wz):
        T = self.tile
        return (self.view.x - self.scroll_x + wz * T / TILE_WORLD,
                self.view.y - self.scroll_y + wx * T / TILE_WORLD)


    def _screen_to_world(self, mx, my):
        T = self.tile
        wz = (mx - self.view.x + self.scroll_x) * TILE_WORLD / T
        wx = (my - self.view.y + self.scroll_y) * TILE_WORLD / T
        wx = max(0, min(ROWS * TILE_WORLD - 1, int(round(wx))))
        wz = max(0, min(COLS * TILE_WORLD - 1, int(round(wz))))
        return [wx, wz]


    def _wp_radius(self):
        return max(5, self.tile // 4)


    def _wp_hit(self, mx, my):
        """Index of the waypoint under the mouse, or None."""
        best, best_d = None, self._wp_radius() ** 2
        for i, (wx, wz) in enumerate(self.waypoints):
            sx, sy = self._world_to_screen(wx, wz)
            d = (sx - mx) ** 2 + (sy - my) ** 2
            if d <= best_d:
                best, best_d = i, d
        return best


    def _segment_hit(self, mx, my):
        """Index of the waypoint that starts the segment under the mouse, or None."""
        n = len(self.waypoints)
        best, best_d = None, self._wp_radius() ** 2
        for i in range(n if n > 2 else n - 1):
            ax, ay = self._world_to_screen(*self.waypoints[i])
            bx, by = self._world_to_screen(*self.waypoints[(i + 1) % n])
            vx, vy = bx - ax, by - ay
            len2 = vx * vx + vy * vy
            if len2 == 0:
                continue
            u = max(0.0, min(1.0, ((mx - ax) * vx + (my - ay) * vy) / len2))
            d = (ax + u * vx - mx) ** 2 + (ay + u * vy - my) ** 2
            if d <= best_d:
                best, best_d = i, d
        return best


    def _wp_tile(self, i):
        wx, wz = self.waypoints[i]
        row, col = wx // TILE_WORLD, wz // TILE_WORLD
        if 0 <= row < ROWS and 0 <= col < COLS:
            return self.grid[row][col]
        return None


    def _wp_off_track(self, i):
        tile = self._wp_tile(i)
        return tile is None or tile in OFF_TRACK


    def _wp_turn(self, i):
        """How far the line turns at waypoint i, in degrees (0 = straight on)."""
        n = len(self.waypoints)
        if n < 3:
            return 0.0
        px, pz = self.waypoints[(i - 1) % n]
        cx, cz = self.waypoints[i]
        nx, nz = self.waypoints[(i + 1) % n]
        a = math.atan2(cz - pz, cx - px)
        b = math.atan2(nz - cz, nx - cx)
        d = abs(math.degrees(b - a)) % 360
        return 360 - d if d > 180 else d


    def _wp_seg_len(self, i):
        """Length of the segment from waypoint i to the next one (world units)."""
        n = len(self.waypoints)
        if n < 2:
            return 0
        (ax, az), (bx, bz) = self.waypoints[i], self.waypoints[(i + 1) % n]
        return int(math.hypot(bx - ax, bz - az))


    def _wp_mouse_down(self, mx, my, shift):
        hit = self._wp_hit(mx, my)
        if hit is not None:
            self.wp_sel  = hit
            self.wp_drag = True
            return
        if not self.view.collidepoint(mx, my):
            return
        pos = self._screen_to_world(mx, my)
        seg = self._segment_hit(mx, my) if shift else None
        if seg is not None:
            at = seg + 1
        elif self.wp_sel is not None:
            at = self.wp_sel + 1
        else:
            at = len(self.waypoints)
        self.waypoints.insert(at, pos)
        self.wp_sel  = at
        self.wp_drag = True
        self.status  = f"Added waypoint {at} ({len(self.waypoints)} in all)"


    def _wp_delete(self, i):
        del self.waypoints[i]
        if not self.waypoints:
            self.wp_sel = None
        elif self.wp_sel is not None and self.wp_sel >= i:
            self.wp_sel = max(0, self.wp_sel - 1)
        self.status = f"Deleted waypoint {i} ({len(self.waypoints)} left)"


    def _wp_make_start(self):
        if self.wp_sel is None:
            self.status = "Select the waypoint at the start line first"
            return
        s = self.wp_sel
        self.waypoints = self.waypoints[s:] + self.waypoints[:s]
        self.wp_sel = 0
        self.status = "That waypoint is now the first one (the start line)"


    def _wp_reverse(self):
        if len(self.waypoints) < 2:
            return
        self.waypoints = self.waypoints[:1] + self.waypoints[:0:-1]
        if self.wp_sel:
            self.wp_sel = len(self.waypoints) - self.wp_sel
        self.status = "Race direction reversed"


    def _wp_import_dialog(self):
        path = filedialog.askopenfilename(
            title='Import waypoints ({ x, z } pairs)',
            initialdir=str(MAPS_DIR),
            filetypes=[('Text / C', '*.txt *.log *.h *.c'), ('All files', '*.*')],
        )
        if path:
            text = Path(path).read_text(encoding='utf-8', errors='replace')
            wps = parse_waypoints(text)
            if not wps:
                self.status = f"No {{ x, z }} waypoints found in {Path(path).name}"
                return
            self.waypoints = wps
            self.wp_sel = None
            self.status = f"Imported {len(wps)} waypoints from {Path(path).name}" + self._waypoint_warnings()


    def _toggle_mode(self):
        if self.mode == MODE_PAINT:
            self.mode     = MODE_SELECT
            self.painting = False
            self.pasting  = False
            self.status   = "SELECT — drag to select  Ctrl+C copy  Ctrl+V paste  Q=back to paint"
        else:
            self.mode      = MODE_PAINT
            self.sel_drag  = False
            self.sel_start = None
            self.sel_end   = None
            self.selection = None
            self.pasting   = False
            self.status    = "PAINT mode"


    def _copy_selection(self):
        if self.selection is None:
            self.status = "Nothing selected — drag a region first"
            return
        r0, c0, r1, c1 = self.selection
        self.clipboard = [
            [self.grid[r][c] for c in range(c0, c1 + 1)]
            for r in range(r0, r1 + 1)
        ]
        h = r1 - r0 + 1
        w = c1 - c0 + 1
        self.status = f"Copied {h}×{w} region — Ctrl+V to paste"


    def _start_paste(self):
        if self.clipboard is None:
            self.status = "Nothing to paste — Ctrl+C to copy a selection first"
            return
        self.pasting = True
        h = len(self.clipboard)
        w = len(self.clipboard[0]) if h else 0
        self.status = f"PASTE {h}×{w} — click to place, Esc to cancel"


    def _do_paste(self, anchor_r, anchor_c):
        for dr, row in enumerate(self.clipboard):
            for dc, ch in enumerate(row):
                tr, tc = anchor_r + dr, anchor_c + dc
                if 0 <= tr < ROWS and 0 <= tc < COLS:
                    self.grid[tr][tc] = ch
        h = len(self.clipboard)
        w = len(self.clipboard[0]) if h else 0
        self.status = f"Pasted {h}×{w} — click again to place another, Esc to finish"


    def _active_sel_rect(self):
        """Return the live drag rect or the finalized selection, whichever applies."""
        if self.sel_drag and self.sel_start and self.sel_end:
            r0 = min(self.sel_start[0], self.sel_end[0])
            c0 = min(self.sel_start[1], self.sel_end[1])
            r1 = max(self.sel_start[0], self.sel_end[0])
            c1 = max(self.sel_start[1], self.sel_end[1])
            return (r0, c0, r1, c1)
        return self.selection


    def grid_cell(self, mx, my):
        """Return (row, col) for mouse position, or None if outside the grid."""
        if not self.view.collidepoint(mx, my):
            return None
        x = (mx - self.view.x + self.scroll_x) // self.tile
        z = (my - self.view.y + self.scroll_y) // self.tile
        if 0 <= x < COLS and 0 <= z < ROWS:
            return z, x
        return None


    def draw_palette(self):
        win_h = self.screen.get_height()
        pygame.draw.rect(self.screen, PANEL, (0, 0, PAL_W, win_h))

        for ch, rect in self.pal_rects:
            if ch == self.selected:
                pygame.draw.rect(self.screen, SEL_CLR, rect.inflate(4, 4), 3)
            img = self.pal_imgs.get(ch)
            if img:
                ix = rect.x + (PAL_CW - 2 - PAL_IMG) // 2
                iy = rect.y + 2
                self.screen.blit(img, (ix, iy))
            lbl = self.font_s.render(f"'{ch}'", True, TEXT_CLR)
            lx  = rect.x + (PAL_CW - 2 - lbl.get_width()) // 2
            self.screen.blit(lbl, (lx, rect.bottom - 13))

        # Mode badge just above the status bar
        if self.pasting:
            mode_label = "[ PASTE ]"
            mode_color = (120, 220, 120)
        elif self.mode == MODE_SELECT:
            mode_label = "[ SELECT ]"
            mode_color = (120, 180, 255)
        elif self.mode == MODE_WAYPOINT:
            mode_label = "[ WAYPOINTS ]"
            mode_color = WP_LINE
        else:
            mode_label = "[ PAINT ]"
            mode_color = (200, 200, 200)

        badge = self.font_m.render(mode_label, True, mode_color)
        bx = (PAL_W - badge.get_width()) // 2
        self.screen.blit(badge, (bx, win_h - STATUS_H - 22))


    def draw_grid(self, hover):
        T = self.tile
        ox, oy = self.view.x - self.scroll_x, self.view.y - self.scroll_y
        self.screen.set_clip(self.view)

        # Tiles (only the ones in the viewport)
        x0 = self.scroll_x // T
        z0 = self.scroll_y // T
        x1 = min(COLS, (self.scroll_x + self.view.w) // T + 1)
        z1 = min(ROWS, (self.scroll_y + self.view.h) // T + 1)
        for z in range(z0, z1):
            for x in range(x0, x1):
                ch  = self.grid[z][x]
                img = self.grid_imgs.get(ch)
                px, py = ox + x * T, oy + z * T
                if img:
                    self.screen.blit(img, (px, py))
                else:
                    pygame.draw.rect(self.screen, (70, 70, 70),
                                     (px, py, T, T))

        # Selection overlay (shown in select mode)
        if self.mode == MODE_SELECT:
            sel = self._active_sel_rect()
            if sel is not None:
                r0, c0, r1, c1 = sel
                px = ox + c0 * T
                py = oy + r0 * T
                w  = (c1 - c0 + 1) * T
                h  = (r1 - r0 + 1) * T
                surf = pygame.Surface((w, h), pygame.SRCALPHA)
                surf.fill((*BOX_SEL, 55))
                self.screen.blit(surf, (px, py))
                pygame.draw.rect(self.screen, BOX_SEL, (px, py, w, h), 2)

        # Paste preview
        if self.pasting and self.clipboard and hover:
            hr, hc = hover
            for dr, row in enumerate(self.clipboard):
                for dc, ch in enumerate(row):
                    tr, tc = hr + dr, hc + dc
                    if 0 <= tr < ROWS and 0 <= tc < COLS:
                        img = self.grid_imgs.get(ch)
                        px2 = ox + tc * T
                        py2 = oy + tr * T
                        if img:
                            ghost = img.copy()
                            ghost.set_alpha(170)
                            self.screen.blit(ghost, (px2, py2))
                        tint = pygame.Surface((T, T), pygame.SRCALPHA)
                        tint.fill((*PASTE_CLR, 70))
                        self.screen.blit(tint, (px2, py2))
            ph = len(self.clipboard)
            pw = len(self.clipboard[0]) if ph else 0
            clip_h = min(ph, ROWS - hr)
            clip_w = min(pw, COLS - hc)
            if clip_h > 0 and clip_w > 0:
                pygame.draw.rect(self.screen, PASTE_CLR,
                                 (ox + hc * T, oy + hr * T,
                                  clip_w * T, clip_h * T), 2)
        elif not self.pasting and hover and self.mode != MODE_WAYPOINT:
            hz, hx = hover
            pygame.draw.rect(
                self.screen, HOVER_CLR,
                (ox + hx * T, oy + hz * T, T, T), 2
            )

        # Grid lines (hidden when zoomed far out)
        if T >= 8:
            for x in range(x0, min(COLS, x1) + 1):
                lx = ox + x * T
                pygame.draw.line(self.screen, GRID_LN, (lx, oy), (lx, oy + ROWS * T))
            for z in range(z0, min(ROWS, z1) + 1):
                ly = oy + z * T
                pygame.draw.line(self.screen, GRID_LN, (ox, ly), (ox + COLS * T, ly))

        self.draw_waypoints()
        self.screen.set_clip(None)


    def draw_waypoints(self):
        """The AI racing line: always shown, in full (numbers, warnings) in waypoint mode."""
        n = len(self.waypoints)
        if n == 0:
            return
        active = self.mode == MODE_WAYPOINT
        pts = [self._world_to_screen(wx, wz) for wx, wz in self.waypoints]

        # Lines (the one closing the lap is thinner)
        width = 2 if active else 1
        for i in range(n if n > 2 else n - 1):
            a, b = pts[i], pts[(i + 1) % n]
            pygame.draw.line(self.screen, WP_LINE, a, b, width if i < n - 1 else 1)

        # Direction arrow halfway along the first segment
        if n >= 2:
            (ax, ay), (bx, by) = pts[0], pts[1]
            ang = math.atan2(by - ay, bx - ax)
            mx, my = (ax + bx) / 2, (ay + by) / 2
            size = max(6, self.tile // 2)
            tip = (mx + math.cos(ang) * size, my + math.sin(ang) * size)
            left = (mx + math.cos(ang + 2.5) * size, my + math.sin(ang + 2.5) * size)
            right = (mx + math.cos(ang - 2.5) * size, my + math.sin(ang - 2.5) * size)
            pygame.draw.polygon(self.screen, WP_START, [tip, left, right])

        # Points: green = start, red = off the track, orange = sharp turn (> 45 degrees, AI brakes)
        r = self._wp_radius() if active else max(2, self._wp_radius() // 2)
        label_every = 1 if self.tile >= 14 else (5 if self.tile >= 9 else 10)
        for i, (sx, sy) in enumerate(pts):
            if i == 0:
                clr = WP_START
            elif self._wp_off_track(i):
                clr = WP_OFF
            elif self._wp_turn(i) > 45:
                clr = WP_SHARP
            else:
                clr = WP_LINE
            pygame.draw.circle(self.screen, clr, (int(sx), int(sy)), r if i else r + 2)
            if self._wp_off_track(i):
                pygame.draw.circle(self.screen, WP_OFF, (int(sx), int(sy)), r + 3, 2)
            if active and i == self.wp_sel:
                pygame.draw.circle(self.screen, SEL_CLR, (int(sx), int(sy)), r + 4, 2)
            if active and i % label_every == 0:
                lbl = self.font_s.render(str(i), True, WP_TEXT)
                self.screen.blit(lbl, (sx + r + 1, sy - r - lbl.get_height() + 2))


    def draw_status(self, hover):
        win_w, win_h = self.screen.get_size()
        sy = win_h - STATUS_H
        pygame.draw.rect(self.screen, STAT_BG, (0, sy, win_w, STATUS_H))

        if self.pasting:
            mode_tag = "PASTE"
        elif self.mode == MODE_SELECT:
            mode_tag = "SELECT"
        elif self.mode == MODE_WAYPOINT:
            mode_tag = "WAYPOINTS"
        else:
            mode_tag = "PAINT"

        sel_name  = TILES[self.selected][3]
        hover_str = ''
        if self.mode == MODE_WAYPOINT:
            # The waypoint under the mouse (or the selected one), else the world position
            mx, my = pygame.mouse.get_pos()
            i = self._wp_hit(mx, my)
            if i is None:
                i = self.wp_sel
            if i is not None and i < len(self.waypoints):
                wx, wz = self.waypoints[i]
                hover_str = (f"   WP {i}/{len(self.waypoints)}  ({wx}, {wz})  tile '{self._wp_tile(i)}'"
                             f"  next {self._wp_seg_len(i)}  turn {self._wp_turn(i):.0f}°")
            elif self.view.collidepoint(mx, my):
                wx, wz = self._screen_to_world(mx, my)
                hover_str = f"   {len(self.waypoints)} waypoints   world ({wx}, {wz})"
            sel_name = ''
        elif hover:
            hz, hx = hover
            ch = self.grid[hz][hx]
            tile_name = TILES.get(ch, ('', '', '', '?'))[3]
            hover_str = f"   [{hz:02d},{hx:02d}] '{ch}' {tile_name}"

        file_str = self.current_file.name if self.current_file else 'Untitled'
        tile_str = '' if self.mode == MODE_WAYPOINT else f"'{self.selected}' {sel_name}"
        msg = (f"  [{mode_tag}]  [{file_str}]  "
               f"{tile_str}{hover_str}   |   {self.status}")
        self.screen.blit(self.font_m.render(msg, True, TEXT_CLR), (4, sy + 8))


    def run(self):
        while True:
            mx, my = pygame.mouse.get_pos()
            hover  = self.grid_cell(mx, my)

            for ev in pygame.event.get():
                if ev.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                elif ev.type == pygame.KEYDOWN:
                    ctrl = ev.mod & pygame.KMOD_CTRL

                    if ev.key == pygame.K_ESCAPE:
                        if self.mode == MODE_WAYPOINT and self.wp_sel is not None:
                            self.wp_sel = None
                            self.status = "Waypoint deselected (clicks now add to the end)"
                        elif self.pasting:
                            self.pasting = False
                            self.status  = "Paste cancelled"
                        elif self.selection is not None:
                            self.selection = None
                            self.sel_start = None
                            self.sel_end   = None
                            self.status    = "Selection cleared"
                        else:
                            pygame.quit()
                            sys.exit()

                    elif ev.key == pygame.K_s and ctrl:
                        self._save()
                    elif ev.key == pygame.K_s:
                        self._save_as_dialog()
                    elif ev.key == pygame.K_o and not ctrl:
                        self._open_dialog()
                    elif ev.key == pygame.K_c and ctrl and self.mode != MODE_WAYPOINT:
                        self._copy_selection()
                    elif ev.key == pygame.K_v and ctrl and self.mode != MODE_WAYPOINT:
                        self._start_paste()
                    elif ev.key == pygame.K_a and ctrl:
                        if self.mode == MODE_SELECT:
                            self.selection = (0, 0, ROWS - 1, COLS - 1)
                            self.status = f"Selected all ({ROWS}×{COLS}) — Ctrl+C to copy"
                    elif ev.key == pygame.K_w and not ctrl:
                        self._toggle_waypoint_mode()
                    elif self.mode == MODE_WAYPOINT and ev.key in (pygame.K_DELETE, pygame.K_BACKSPACE):
                        if self.wp_sel is not None:
                            self._wp_delete(self.wp_sel)
                    elif self.mode == MODE_WAYPOINT and ev.key == pygame.K_HOME:
                        self._wp_make_start()
                    elif self.mode == MODE_WAYPOINT and ev.key == pygame.K_r and not ctrl:
                        self._wp_reverse()
                    elif self.mode == MODE_WAYPOINT and ev.key == pygame.K_i and not ctrl:
                        self._wp_import_dialog()
                    elif ev.key == pygame.K_DELETE:
                        if self.mode == MODE_SELECT and self.selection:
                            r0, c0, r1, c1 = self.selection
                            for r in range(r0, r1 + 1):
                                for c in range(c0, c1 + 1):
                                    self.grid[r][c] = '3'
                            self.status = "Selection filled with grass"
                    elif ev.key in (pygame.K_q, pygame.K_TAB):
                        self._toggle_mode()
                    elif ev.key == pygame.K_f:
                        self._fit()
                    elif ev.key in (pygame.K_LEFT, pygame.K_RIGHT, pygame.K_UP, pygame.K_DOWN):
                        step = self.tile * 4
                        if ev.key == pygame.K_LEFT:
                            self.scroll_x -= step
                        elif ev.key == pygame.K_RIGHT:
                            self.scroll_x += step
                        elif ev.key == pygame.K_UP:
                            self.scroll_y -= step
                        else:
                            self.scroll_y += step
                        self._clamp_scroll()

                elif ev.type == pygame.VIDEORESIZE:
                    self._layout()

                elif ev.type == pygame.MOUSEWHEEL:
                    self._zoom(ev.y, mx, my)

                elif ev.type == pygame.MOUSEBUTTONDOWN and ev.button == 2:
                    self.panning = True

                elif ev.type == pygame.MOUSEBUTTONUP and ev.button == 2:
                    self.panning = False

                elif ev.type == pygame.MOUSEBUTTONDOWN:
                    if ev.button == 1:
                        # Palette click — works in any mode
                        hit_pal = False
                        for ch, rect in self.pal_rects:
                            if rect.collidepoint(ev.pos):
                                self.selected = ch
                                hit_pal = True
                                if self.pasting:
                                    self.pasting = False
                                    self.status  = f"Paste cancelled — selected '{ch}'"
                                break

                        if not hit_pal:
                            if self.mode == MODE_WAYPOINT:
                                shift = pygame.key.get_mods() & pygame.KMOD_SHIFT
                                self._wp_mouse_down(ev.pos[0], ev.pos[1], shift)
                            elif self.pasting and hover:
                                self._do_paste(hover[0], hover[1])
                            elif self.mode == MODE_SELECT and hover:
                                self.sel_start = hover
                                self.sel_end   = hover
                                self.sel_drag  = True
                                self.selection = None
                            elif self.mode == MODE_PAINT:
                                self.painting = True
                                if hover:
                                    self.grid[hover[0]][hover[1]] = self.selected

                    elif ev.button == 3:
                        if self.mode == MODE_WAYPOINT:
                            i = self._wp_hit(ev.pos[0], ev.pos[1])
                            if i is not None:
                                self._wp_delete(i)
                        elif self.mode == MODE_SELECT:
                            self.selection = None
                            self.sel_start = None
                            self.sel_end   = None
                            self.status    = "Selection cleared"
                        elif hover:
                            self.grid[hover[0]][hover[1]] = '3'

                elif ev.type == pygame.MOUSEBUTTONUP:
                    if ev.button == 1:
                        if self.sel_drag:
                            self.sel_drag = False
                            if self.sel_start and self.sel_end:
                                r0 = min(self.sel_start[0], self.sel_end[0])
                                c0 = min(self.sel_start[1], self.sel_end[1])
                                r1 = max(self.sel_start[0], self.sel_end[0])
                                c1 = max(self.sel_start[1], self.sel_end[1])
                                self.selection = (r0, c0, r1, c1)
                                h = r1 - r0 + 1
                                w = c1 - c0 + 1
                                self.status = f"Selected {h}×{w} — Ctrl+C to copy"
                        self.painting = False
                        self.wp_drag  = False

                elif ev.type == pygame.MOUSEMOTION:
                    if self.panning:
                        self.scroll_x -= ev.rel[0]
                        self.scroll_y -= ev.rel[1]
                        self._clamp_scroll()
                    elif self.wp_drag and self.wp_sel is not None:
                        self.waypoints[self.wp_sel] = self._screen_to_world(*ev.pos)
                    elif self.sel_drag and hover:
                        self.sel_end = hover
                    elif self.painting and hover:
                        self.grid[hover[0]][hover[1]] = self.selected

            self.screen.fill(BG)
            self.draw_palette()
            self.draw_grid(hover)
            self.draw_status(hover)
            pygame.display.flip()
            self.clock.tick(60)


if __name__ == '__main__':
    Editor().run()
