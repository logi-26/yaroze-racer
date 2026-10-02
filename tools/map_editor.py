#!/usr/bin/env python3
"""
Yaroze Racer - Ground Map Editor
Saves map data as a .h file for inclusion in game/ground.c

Controls:
  Left-click palette        select tile
  Q / Tab                   toggle Paint / Select mode
  W                         toggle Waypoint mode (the AI racing line)
  B                         toggle Objects mode (barriers and buildings inside the arena)
  Paint mode:
    Left-click/drag grid    paint selected tile
    Right-click grid        erase to grass ('3')
  Select mode:
    Left-click/drag grid    draw selection rectangle
    Right-click             clear selection
    Ctrl+A                  select all tiles
    Delete                  fill selection with grass (and remove its barriers and buildings)
    Ctrl+C                  copy selection to clipboard (tiles, barriers and buildings)
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
  Objects mode:
    1 / 2 / 3               barrier / building / start position tool
    Left-click              place one (positions snap to 1/8 of a tile)
    Left-drag an object     move it (a back-to-back barrier pair moves together)
    Shift+drag (barriers)   lay a run of barriers along the barrier's direction
    Right-click an object   delete it (or Delete for the selected one)
    R                       rotate the barrier 90 degrees (buildings: 180)
    X                       flip which way a barrier faces (barriers are one-sided)
    M                       switch model (BARRIER_1/2, BUILDING_1/2, STAND)
    D                       double-sided barriers on/off (back-to-back pairs, 5 apart)
    E / Shift+E             line the infield / outside edges of the road with barriers
    [ / ]                   make the arena (the walled-in area, white outline) a tile smaller / bigger
    Ctrl+arrow keys         move the arena a tile (its walls move with it)
    G                       rebuild the arena walls to fit the arena (buildings are left as they are)
    Start positions (3):    drag a car to move it, R turns it 90 degrees, X turns it round,
                            P places all of them on the start grid tiles (h / g)
    Ctrl+Z                  undo (objects)
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


def read_define(name: str, default: int) -> int:
    """A number #defined in game/ground.h."""
    try:
        text = (MAPS_DIR / 'ground.h').read_text(encoding='utf-8', errors='replace')
        m = re.search(r'#define\s+' + name + r'\s+\(?\s*(\d+)', text)
        if m:
            return int(m.group(1))
    except OSError:
        pass
    return default


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

# Corner tiles that are only partly tarmac: the road is a disc (radius in tiles) around one
# corner of the tile (row, col fraction). Inner corners (t_01) and outer corners (t_1)
CURVED_TILES = {
    'n': ((0, 1), 0.6), 'p': ((1, 1), 0.6), 'q': ((1, 0), 0.6), 'r': ((0, 0), 0.6),
    '8': ((1, 0), 1.0), 'a': ((0, 0), 1.0), 'c': ((0, 1), 1.0), 'd': ((1, 1), 1.0),
}

# Objects inside the arena (world.c): barriers are 1950 long with a capsule of radius 100
# for collision, and only seen from the side they face; buildings collide as circles
MAX_BARRIERS  = read_define('MAX_TRACK_BARRIERS', 150)
MAX_BUILDINGS = read_define('MAX_TRACK_BUILDINGS', 40)
BARRIER_HALF  = 975
BARRIER_R     = 100
PAIR_GAP      = 5
OBJ_SNAP      = 150
BUILDING_R    = 400
STAND_R       = 350
VIEW_PAD      = 6         # tiles of margin shown around the map (the buildings stand outside it)
BUILDING_Y    = -450
EDGE_SETBACK  = 450       # auto-edged barriers: distance from the road edge into the grass

# Inner corner tiles are mostly grass (for finding the infield and the road edges)
FILLETS = {'n', 'p', 'q', 'r'}

# Barrier rot: 0/1 run along Z (across the grid) facing +X / -X (down / up),
#              2/3 run along X (down the grid) facing +Z / -Z (right / left)
FACING   = {0: (1, 0), 1: (-1, 0), 2: (0, 1), 3: (0, -1)}
OPPOSITE = {0: 1, 1: 0, 2: 3, 3: 2}
ROT_CW   = {0: 3, 3: 1, 1: 2, 2: 0}
FACING_NAME = {0: 'faces down', 1: 'faces up', 2: 'faces right', 3: 'faces left'}

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
BAR_CLR   = ((215, 215, 215), (235, 130, 60))     # BARRIER_1, BARRIER_2
BLD_CLR   = ((150, 110, 80), (110, 120, 170), (170, 60, 150))     # BUILDING_1, BUILDING_2, STAND
BLD_NAME  = ('building 1', 'building 2', 'stand')
ARENA_CLR = (255, 255, 255)
OBJ_BAD   = (255, 40, 40)

# Mode constants
MODE_PAINT  = 'paint'
MODE_SELECT = 'select'
MODE_WAYPOINT = 'waypoint'
MODE_OBJECTS  = 'objects'

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


# The walls around the arena: 200 inside the near edges and 700 inside the far edges,
# a barrier every 1950 from 1200 (alternating the two models)
WALL_NEAR, WALL_FAR_GAP, WALL_FIRST, WALL_SPACING = 200, 700, 1200, 1950


def arena_walls(size: int, x0: int = 0, z0: int = 0) -> list:
    """The barriers of the walls around an arena of this size, with its corner at world (x0, z0)."""
    far, out, k, pos = size - WALL_FAR_GAP, [], 0, WALL_FIRST
    while pos <= far - 900:
        out += [[x0 + WALL_NEAR, z0 + pos, 0, k & 1], [x0 + far, z0 + pos, 1, k & 1],
                [x0 + pos, z0 + WALL_NEAR, 2, k & 1], [x0 + pos, z0 + far, 3, k & 1]]
        k, pos = k + 1, pos + WALL_SPACING
    return out


# The start positions (game: StartDef): player 1, player 2 (link race) and the AI racers,
# as [x, z, heading]: heading 0-4095, 0 = facing +Z (right on the grid), 1024 = facing +X (down)
NUM_STARTS = read_define('NUM_STARTS', 7)
STANDARD_STARTS = [[3605, 9273, 0], [3000, 8800, 0], [3000, 8800, 0], [4200, 7800, 0],
                   [3600, 5800, 0], [4800, 4800, 0], [4200, 2800, 0]]
STANDARD_GRID_CENTRE = (3600, 9600)       # the middle of map_1's grid tiles (rows 2-3, cols 7-8)
START_NAMES = ['P1', 'P2'] + [f'A{i}' for i in range(1, NUM_STARTS - 1)]
START_CLR = ((80, 255, 80), (80, 200, 255), (255, 220, 60))   # player 1, player 2, AI


def starts_for_grid(grid: list):
    """The start positions placed on a map's start grid tiles: map_1's layout moved to the
    first 'h' tile (and turned round if the start line is behind it). None if there's no grid."""
    at = next(((r, c) for r in range(ROWS) for c in range(COLS) if grid[r][c] == 'h'), None)
    if at is None:
        return None
    r, c = at
    cx, cz = (r + 1) * TILE_WORLD, (c + 1) * TILE_WORLD
    lines = [cc for rr in (r, r + 1) if rr < ROWS for cc in range(COLS) if grid[rr][cc] in '45']
    backwards = bool(lines) and min(lines, key=lambda cc: abs(cc - c)) < c
    out = []
    for x, z, h in STANDARD_STARTS[:NUM_STARTS]:
        dx, dz = x - STANDARD_GRID_CENTRE[0], z - STANDARD_GRID_CENTRE[1]
        if backwards:
            out.append([cx - dx, cz - dz, (h + 2048) % 4096])
        else:
            out.append([cx + dx, cz + dz, h])
    return out


# The two crowd stands by the start straight
ARENA_STANDS = [[-1700, BUILDING_Y, 15000, 0, 2, STAND_R], [7500, BUILDING_Y, 15000, 1, 2, STAND_R]]


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


def parse_struct_array(text: str, type_name: str, count_prefix: str, fields: int) -> list:
    """The entries of a `const <type_name> name[...] = { ... };` array, as lists of ints
    (only as many as its NUM_ count says: an empty array is saved with one placeholder)."""
    m = re.search(type_name + r'\s+\w+\s*\[[^\]]*\]\s*=\s*\{(.*?)\};', text, re.DOTALL)
    if not m:
        return []
    num = r'\s*(-?\d+)\s*'
    entry = re.compile(r'\{' + ','.join([num] * fields) + r'\}')
    items = [[int(v) for v in e] for e in entry.findall(m.group(1))]
    c = re.search(r'#define\s+' + count_prefix + r'\w*\s+\(\s*(\d+)\s*\)', text)
    return items[:int(c.group(1))] if c else items


def parse_map(src_path: Path) -> tuple:
    """Load any groundData* array (and its waypoints, barriers and buildings) from a .h or .c file.
    Returns (grid, (rows, cols) of the file, waypoints, barriers, buildings, arena (x, z, size), starts).
    A smaller map is padded with grass."""
    grid = [['3'] * COLS for _ in range(ROWS)]
    rows_in = cols_in = 0
    waypoints, barriers, buildings, starts = [], [], [], []
    arena = (0, 0, COLS * TILE_WORLD)
    try:
        text = src_path.read_text(encoding='utf-8', errors='replace')

        def arena_define(name, default):
            m = re.search(r'#define\s+ARENA_' + name + r'_\w*\s+\(\s*(-?\d+)\s*\)', text)
            return int(m.group(1)) if m else default
        arena = (arena_define('X', 0), arena_define('Z', 0), arena_define('SIZE', COLS * TILE_WORLD))
        if re.search(r'Waypoint\s+\w+\s*\[', text):
            waypoints = parse_waypoints(text)
        barriers = parse_struct_array(text, 'BarrierDef', 'NUM_BARRIERS_', 4)
        buildings = parse_struct_array(text, 'BuildingDef', 'NUM_BUILDINGS_', 6)
        starts = parse_struct_array(text, 'StartDef', 'NUM_STARTS_', 3)
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
    # A map without start positions gets map_1's
    starts = (starts + [list(s) for s in STANDARD_STARTS])[:NUM_STARTS]
    return grid, (rows_in, cols_in), waypoints, barriers, buildings, arena, starts


def save_map(grid: list, waypoints: list, dest: Path, barriers=(), buildings=(), arena=None, starts=None) -> None:
    """Write the map array, its AI waypoints, barriers, buildings and arena (x, z, size) to dest as a C header file."""
    arena_x, arena_z, arena_size = arena or (0, 0, COLS * TILE_WORLD)
    stem  = dest.stem
    guard = re.sub(r'\W', '_', stem).upper() + '_H'
    # Derive the names from the filename: map_1 -> groundDataMap1, waypointsMap1, NUM_WAYPOINTS_MAP1
    parts      = re.split(r'[_\-]', stem)
    suffix     = ''.join(p.capitalize() for p in parts)
    array_name = 'groundData' + suffix
    wp_name    = 'waypoints' + suffix
    wp_count   = 'NUM_WAYPOINTS_' + suffix.upper()
    bar_name   = 'barriers' + suffix
    bar_count  = 'NUM_BARRIERS_' + suffix.upper()
    bld_name   = 'buildings' + suffix
    bld_count  = 'NUM_BUILDINGS_' + suffix.upper()
    arena_name = 'ARENA_SIZE_' + suffix.upper()
    arena_x_name = 'ARENA_X_' + suffix.upper()
    starts_name  = 'starts' + suffix
    starts_count = 'NUM_STARTS_' + suffix.upper()
    arena_z_name = 'ARENA_Z_' + suffix.upper()

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
    lines += ['};', '']

    # The arena: the walled-in square of the world (the walls are barriers below)
    lines += [
        '// The arena: its corner (world X, Z) and size (the walls around it are barriers,',
        '// and its corners have collision posts)',
        f'#define {arena_x_name} ({arena_x})',
        f'#define {arena_z_name} ({arena_z})',
        f'#define {arena_name} ({arena_size})',
        '',
    ]

    # Barriers and buildings (an empty array keeps one placeholder entry)
    bars = list(barriers) or [[0, 0, 0, 0]]
    lines += [
        '// Barriers: world (X, Z), rot, model (the arena walls and everything inside it)',
        '// rot: 0 / 1 = along Z facing +X / -X, 2 / 3 = along X facing +Z / -Z (they are one-sided)',
        '// model: 0 = BARRIER_1, 1 = BARRIER_2',
        f'#define {bar_count} ({len(barriers)})',
        '',
        f'const BarrierDef {bar_name}[{len(bars)}] = {{',
    ]
    for i, (x, z, rot, model) in enumerate(bars):
        comma = ',' if i < len(bars) - 1 else ''
        lines.append(f'\t{{ {x:6d}, {z:6d}, {rot}, {model} }}{comma}')
    lines += ['};', '']

    blds = list(buildings) or [[0, 0, 0, 0, 0, 0]]
    lines += [
        '// Buildings: world (X, Y, Z), rot (0, or 1 = 180 degrees), model, collision radius',
        '// model: 0 = BUILDING_1, 1 = BUILDING_2, 2 = STAND',
        f'#define {bld_count} ({len(buildings)})',
        '',
        f'const BuildingDef {bld_name}[{len(blds)}] = {{',
    ]
    for i, (x, y, z, rot, model, radius) in enumerate(blds):
        comma = ',' if i < len(blds) - 1 else ''
        lines.append(f'\t{{ {x:6d}, {y:4d}, {z:6d}, {rot}, {model}, {radius} }}{comma}')
    lines += ['};', '']

    # Start positions
    sts = [list(s) for s in (starts or STANDARD_STARTS)][:NUM_STARTS]
    lines += [
        '// Start positions: world (X, Z) and heading (0-4095: 0 = facing +Z, 1024 = facing +X)',
        '// 0 = player 1, 1 = player 2 (link race), 2 onwards = the AI racers',
        f'#define {starts_count} ({len(sts)})',
        '',
        f'const StartDef {starts_name}[{starts_count}] = {{',
    ]
    for i, (x, z, h) in enumerate(sts):
        comma = ',' if i < len(sts) - 1 else ''
        lines.append(f'\t{{ {x:6d}, {z:6d}, {h:4d} }}{comma}    // {START_NAMES[i]}')
    lines += ['};', '', f'#endif // {guard}', '']
    dest.write_text('\n'.join(lines), encoding='utf-8')


class Editor:
    def __init__(self):
        self._tk = tk.Tk()
        self._tk.withdraw()

        pygame.init()

        # Window size (the whole map at the largest zoom that fits the desktop)
        desk_w, desk_h = pygame.display.get_desktop_sizes()[0]
        fit = min((desk_w - 80 - PAL_W - MARGIN * 2) // (COLS + 2 * VIEW_PAD),
                  (desk_h - 120 - STATUS_H - MARGIN * 2) // (ROWS + 2 * VIEW_PAD),
                  TILE_FIT_MAX)
        self.tile = max(TILE_MIN, fit)
        win_w = PAL_W + MARGIN + min((COLS + 2 * VIEW_PAD) * self.tile, desk_w - 80 - PAL_W - MARGIN * 2) + MARGIN
        win_h = max(MIN_WIN_H, min((ROWS + 2 * VIEW_PAD) * self.tile,
                                   desk_h - 120 - STATUS_H - MARGIN * 2) + STATUS_H + MARGIN * 2)
        self.screen = pygame.display.set_mode((win_w, win_h), pygame.RESIZABLE)
        self.clock  = pygame.time.Clock()
        self.font_s = pygame.font.SysFont('monospace',  9)
        self.font_m = pygame.font.SysFont('monospace', 13)

        self.current_file: Path | None = None
        self.grid     = [['3'] * COLS for _ in range(ROWS)]
        self.selected = '3'
        self.status   = "O=open  S=save  Q=select mode  W=waypoints  B=objects  Ctrl+C/V=copy/paste  Wheel=zoom  Middle-drag=pan  F=fit"
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
        self.clip_barriers  = []  # barriers / buildings copied with it (positions from its corner)
        self.clip_buildings = []
        self.pasting   = False   # True while waiting to place clipboard

        # AI waypoints: [world x, world z] in race order
        self.waypoints = []
        self.wp_sel    = None    # Index of the selected waypoint, or None
        self.wp_drag   = False   # True while dragging the selected waypoint

        # Objects inside the arena: barriers [x, z, rot, model], buildings [x, y, z, rot, model, radius]
        self.barriers  = []
        self.buildings = []
        self.obj_sel   = None    # ('bar', i) or ('bld', i)
        self.obj_drag  = None    # [(kind, i, dx, dz)] moving with the mouse
        self.obj_run   = None    # world [x, z] where a Shift+drag barrier run started
        self.obj_tool  = 'bar'
        self.bar_rot   = 0
        self.bar_model = 0
        self.bld_rot   = 0
        self.bld_model = 0
        self.double_sided = True
        self.obj_undo  = []      # earlier (barriers, buildings, arena), for Ctrl+Z
        self.arena     = COLS * TILE_WORLD   # the arena: its size, and its corner (world X, Z)
        self.arena_x   = 0
        self.arena_z   = 0
        self.wall_arena = (0, 0, self.arena)   # the arena (x, z, size) the current walls were made for
        self.starts    = [list(s) for s in STANDARD_STARTS][:NUM_STARTS]


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
        self.scroll_x = max(0, min(self.scroll_x, (COLS + 2 * VIEW_PAD) * self.tile - self.view.w))
        self.scroll_y = max(0, min(self.scroll_y, (ROWS + 2 * VIEW_PAD) * self.tile - self.view.h))


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
        self.tile = max(TILE_MIN, min(TILE_MAX, self.view.w // (COLS + 2 * VIEW_PAD),
                                      self.view.h // (ROWS + 2 * VIEW_PAD)))
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
            self.grid, (rows_in, cols_in), self.waypoints, self.barriers, self.buildings, arena, self.starts = parse_map(p)
            self.arena_x, self.arena_z, self.arena = arena
            self.wall_arena = arena
            self.wp_sel = None
            self.obj_sel = None
            self.current_file = p
            self._update_title()
            wp_str = f", {len(self.waypoints)} waypoints, {len(self.barriers)} barriers, {len(self.buildings)} buildings"
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
            save_map(self.grid, self.waypoints, p, self.barriers, self.buildings,
                     (self.arena_x, self.arena_z, self.arena), self.starts)
            self.current_file = p
            self._update_title()
            self.status = f"Saved → {p.name}" + self._waypoint_warnings() + self._object_warnings()


    def _save(self):
        if self.current_file:
            save_map(self.grid, self.waypoints, self.current_file, self.barriers, self.buildings,
                     (self.arena_x, self.arena_z, self.arena), self.starts)
            self.status = f"Saved → {self.current_file.name}" + self._waypoint_warnings() + self._object_warnings()
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


    def _object_warnings(self):
        """Problems with the barriers and buildings (for the status after a save)."""
        out = ""
        on_road = (sum(1 for b in self.barriers if self._barrier_on_road(b)) +
                   sum(1 for b in self.buildings if self._building_on_road(b)))
        if on_road:
            out += f"   WARNING: {on_road} object(s) on the road (red)"
        bad = [START_NAMES[i] for i in range(len(self.starts)) if self._start_bad(i)]
        if bad:
            out += f"   WARNING: start position(s) {', '.join(bad)} off the road or outside the arena"
        if len(self.barriers) > MAX_BARRIERS:
            out += f"   WARNING: {len(self.barriers)} barriers, the game takes {MAX_BARRIERS}"
        if len(self.buildings) > MAX_BUILDINGS:
            out += f"   WARNING: {len(self.buildings)} buildings, the game takes {MAX_BUILDINGS}"
        return out


    def _toggle_objects_mode(self):
        if self.mode == MODE_OBJECTS:
            self.mode     = MODE_PAINT
            self.obj_drag = None
            self.obj_run  = None
            self.status   = "PAINT mode"
        else:
            self.mode      = MODE_OBJECTS
            self.painting  = False
            self.pasting   = False
            self.sel_drag  = False
            self.selection = None
            self.wp_drag   = False
            self.status    = ("OBJECTS — 1 barrier  2 building  click place  drag move  Shift+drag barrier run  "
                              "right-click delete  R rotate  X flip  M model  D double-sided  B=back")


    def _road_in_rect(self, x0, x1, z0, z1):
        """Whether any road tile overlaps the world rectangle [x0, x1] x [z0, z1]."""
        r0, r1 = max(0, int(x0 // TILE_WORLD)), min(ROWS - 1, int(x1 // TILE_WORLD))
        c0, c1 = max(0, int(z0 // TILE_WORLD)), min(COLS - 1, int(z1 // TILE_WORLD))
        for r in range(r0, r1 + 1):
            for c in range(c0, c1 + 1):
                ch = self.grid[r][c]
                if ch in OFF_TRACK:
                    continue
                if ch in CURVED_TILES:
                    # Only the tarmac disc counts: the nearest point of the rectangle to its centre
                    (fr, fc), radius = CURVED_TILES[ch]
                    px, pz = (r + fr) * TILE_WORLD, (c + fc) * TILE_WORLD
                    nx, nz = max(x0, min(px, x1)), max(z0, min(pz, z1))
                    if math.hypot(nx - px, nz - pz) >= radius * TILE_WORLD:
                        continue
                return True
        return False


    def _barrier_on_road(self, b):
        x, z, rot, _ = b
        if rot in (0, 1):
            return self._road_in_rect(x - BARRIER_R, x + BARRIER_R, z - BARRIER_HALF - BARRIER_R, z + BARRIER_HALF + BARRIER_R)
        return self._road_in_rect(x - BARRIER_HALF - BARRIER_R, x + BARRIER_HALF + BARRIER_R, z - BARRIER_R, z + BARRIER_R)


    def _building_on_road(self, b):
        x, _, z, _, _, r = b
        return self._road_in_rect(x - r, x + r, z - r, z + r)


    def _barrier_ends(self, b):
        x, z, rot, _ = b
        if rot in (0, 1):
            return (x, z - BARRIER_HALF), (x, z + BARRIER_HALF)
        return (x - BARRIER_HALF, z), (x + BARRIER_HALF, z)


    def _mouse_world(self, mx, my):
        """The world position under the mouse (unsnapped)."""
        pad = VIEW_PAD * self.tile
        return ((my - self.view.y + self.scroll_y - pad) * TILE_WORLD / self.tile,
                (mx - self.view.x + self.scroll_x - pad) * TILE_WORLD / self.tile)


    @staticmethod
    def _snap(v):
        return int(round(v / OBJ_SNAP) * OBJ_SNAP)


    def _start_bad(self, i):
        """Whether start position i is off the road or outside the arena."""
        x, z, _ = self.starts[i]
        r, c = int(x // TILE_WORLD), int(z // TILE_WORLD)
        if not (0 <= r < ROWS and 0 <= c < COLS) or self.grid[r][c] in OFF_TRACK:
            return True
        return not (self.arena_x <= x < self.arena_x + self.arena and self.arena_z <= z < self.arena_z + self.arena)


    def _obj_hit(self, mx, my):
        """The object under the mouse: ('bld', i), ('bar', i) or with the start tool ('start', i), or None."""
        wx, wz = self._mouse_world(mx, my)
        tol = 5 * TILE_WORLD / self.tile          # 5 pixels
        if self.obj_tool == 'start':
            best, best_d = None, max(350, tol)
            for i, (x, z, _) in enumerate(self.starts):
                d = math.hypot(wx - x, wz - z)
                if d <= best_d:
                    best, best_d = ('start', i), d
            return best
        for i, b in enumerate(self.buildings):
            if math.hypot(wx - b[0], wz - b[2]) <= max(b[5], tol):
                return ('bld', i)
        best, best_d = None, max(BARRIER_R, tol)
        for i, b in enumerate(self.barriers):
            (ax, az), (bx, bz) = self._barrier_ends(b)
            u = max(0.0, min(1.0, ((wx - ax) * (bx - ax) + (wz - az) * (bz - az)) / float(2 * BARRIER_HALF) ** 2))
            d = math.hypot(ax + u * (bx - ax) - wx, az + u * (bz - az) - wz)
            if d <= best_d:
                best, best_d = ('bar', i), d
        return best


    def _partner(self, i):
        """The barrier back to back with barrier i (a double-sided pair), or None."""
        x, z, rot, _ = self.barriers[i]
        for j, (x2, z2, rot2, _) in enumerate(self.barriers):
            if j != i and rot2 == OPPOSITE[rot]:
                if rot in (0, 1) and abs(z2 - z) <= 2 and abs(x2 - x) <= 10:
                    return j
                if rot in (2, 3) and abs(x2 - x) <= 2 and abs(z2 - z) <= 10:
                    return j
        return None


    def _add_barrier(self, x, z, rot, model):
        """Add a barrier (and its back-to-back partner when double-sided). Returns its index."""
        self.barriers.append([x, z, rot, model])
        i = len(self.barriers) - 1
        if self.double_sided:
            fx, fz = FACING[rot]
            self.barriers.append([x - PAIR_GAP * fx, z - PAIR_GAP * fz, OPPOSITE[rot], model])
        return i


    def _run_barriers(self, start, end):
        """The barriers of a run from start to end, along the barrier tool's direction."""
        (sx, sz), (ex, ez) = start, end
        along_z = self.bar_rot in (0, 1)
        length = (ez - sz) if along_z else (ex - sx)
        n = max(1, int(round(abs(length) / (2 * BARRIER_HALF))))
        d = 1 if length >= 0 else -1
        out = []
        for k in range(n):
            c = (sz if along_z else sx) + d * (BARRIER_HALF + k * 2 * BARRIER_HALF)
            x, z = (self._snap(sx), c) if along_z else (c, self._snap(sz))
            out.append([x, z, self.bar_rot, (self.bar_model + k) & 1])
        return out


    def _obj_push_undo(self):
        self.obj_undo.append(([list(b) for b in self.barriers], [list(b) for b in self.buildings],
                              (self.arena_x, self.arena_z, self.arena), self.wall_arena,
                              [list(s) for s in self.starts]))
        del self.obj_undo[:-50]


    def _obj_undo(self):
        if not self.obj_undo:
            self.status = "Nothing to undo"
            return
        self.barriers, self.buildings, arena, self.wall_arena, self.starts = self.obj_undo.pop()
        self.arena_x, self.arena_z, self.arena = arena
        self.obj_sel = None
        self.obj_drag = None
        self.status = f"Undone ({len(self.barriers)} barriers, {len(self.buildings)} buildings)"


    def _obj_mouse_down(self, mx, my, shift):
        if not self.view.collidepoint(mx, my):
            return
        self._obj_push_undo()
        hit = self._obj_hit(mx, my)
        wx, wz = self._mouse_world(mx, my)
        if hit is not None:
            self.obj_sel = hit
            self._start_obj_drag(wx, wz)
            return
        if self.obj_tool == 'start':
            return                          # the start positions are only moved, not added
        if shift and self.obj_tool == 'bar':
            self.obj_run = [self._snap(wx), self._snap(wz)]
            return
        x, z = self._snap(wx), self._snap(wz)
        if self.obj_tool == 'bar':
            self.obj_sel = ('bar', self._add_barrier(x, z, self.bar_rot, self.bar_model))
            self.bar_model ^= 1            # alternate the models, like the walls
        else:
            self.buildings.append([x, BUILDING_Y, z, self.bld_rot, self.bld_model,
                                   STAND_R if self.bld_model == 2 else BUILDING_R])
            self.obj_sel = ('bld', len(self.buildings) - 1)
        self._start_obj_drag(wx, wz)


    def _start_obj_drag(self, wx, wz):
        kind, i = self.obj_sel
        if kind == 'start':
            s = self.starts[i]
            self.obj_drag = [('start', i, s[0] - wx, s[1] - wz)]
            return
        if kind == 'bld':
            b = self.buildings[i]
            self.obj_drag = [('bld', i, b[0] - wx, b[2] - wz)]
            return
        b = self.barriers[i]
        self.obj_drag = [('bar', i, b[0] - wx, b[1] - wz)]
        j = self._partner(i)
        if j is not None:
            self.obj_drag.append(('bar', j, self.barriers[j][0] - wx, self.barriers[j][1] - wz))


    def _obj_drag_to(self, mx, my):
        wx, wz = self._mouse_world(mx, my)
        # Snap the selected object, and keep the partner at the same offset from it
        kind, i, dx, dz = self.obj_drag[0]
        x, z = self._snap(wx + dx), self._snap(wz + dz)
        def lookup(kind, j):
            if kind == 'bld':
                return self.buildings[j], 2
            return (self.starts[j] if kind == 'start' else self.barriers[j]), 1
        obj, zi = lookup(kind, i)
        ox, oz = obj[0], obj[zi]
        for kind, j, _, _ in self.obj_drag:
            obj, zi = lookup(kind, j)
            obj[0] += x - ox
            obj[zi] += z - oz


    def _obj_mouse_up(self, mx, my):
        if self.obj_run is not None:
            wx, wz = self._mouse_world(mx, my)
            run = self._run_barriers(self.obj_run, (wx, wz))
            for x, z, rot, model in run:
                self._add_barrier(x, z, rot, model)
            self.obj_run = None
            self.status = f"Added a run of {len(run)} barrier(s){' (double-sided)' if self.double_sided else ''}"
        self.obj_drag = None


    def _obj_selected_pair(self):
        """Indices of the selected barrier and its partner (if any)."""
        kind, i = self.obj_sel
        j = self._partner(i)
        return [i] if j is None else [i, j]


    def _obj_delete(self, hit):
        if hit[0] == 'start':
            self.status = "Start positions can't be deleted (drag them where you want them)"
            return
        self._obj_push_undo()
        kind, i = hit
        if kind == 'bld':
            del self.buildings[i]
            self.status = f"Deleted a building ({len(self.buildings)} left)"
        else:
            idx = sorted([i] + ([] if self._partner(i) is None else [self._partner(i)]), reverse=True)
            for k in idx:
                del self.barriers[k]
            self.status = f"Deleted {len(idx)} barrier(s) ({len(self.barriers)} left)"
        self.obj_sel = None


    def _obj_rotate(self):
        self._obj_push_undo()
        if self.obj_sel and self.obj_sel[0] == 'start':
            s = self.starts[self.obj_sel[1]]
            s[2] = (s[2] + 1024) % 4096
        elif self.obj_sel and self.obj_sel[0] == 'bld':
            b = self.buildings[self.obj_sel[1]]
            b[3] ^= 1
            self.bld_rot = b[3]
        elif self.obj_sel and self.obj_sel[0] == 'bar':
            i = self.obj_sel[1]
            pair = self._obj_selected_pair()
            b = self.barriers[i]
            b[2] = ROT_CW[b[2]]
            if len(pair) == 2:
                fx, fz = FACING[b[2]]
                self.barriers[pair[1]][:3] = [b[0] - PAIR_GAP * fx, b[1] - PAIR_GAP * fz, OPPOSITE[b[2]]]
            self.bar_rot = b[2]
        elif self.obj_tool == 'bld':
            self.bld_rot ^= 1
        else:
            self.bar_rot = ROT_CW[self.bar_rot]


    def _obj_flip(self):
        self._obj_push_undo()
        if self.obj_sel and self.obj_sel[0] == 'start':
            s = self.starts[self.obj_sel[1]]
            s[2] = (s[2] + 2048) % 4096
        elif self.obj_sel and self.obj_sel[0] == 'bar':
            for k in self._obj_selected_pair():
                self.barriers[k][2] = OPPOSITE[self.barriers[k][2]]
            self.bar_rot = self.barriers[self.obj_sel[1]][2]
        elif self.obj_tool == 'bar':
            self.bar_rot = OPPOSITE[self.bar_rot]


    def _obj_model(self):
        if self.obj_tool == 'start':
            return
        self._obj_push_undo()
        if self.obj_sel and self.obj_sel[0] == 'bld':
            b = self.buildings[self.obj_sel[1]]
            b[4] = (b[4] + 1) % 3
            b[5] = STAND_R if b[4] == 2 else BUILDING_R
        elif self.obj_sel and self.obj_sel[0] == 'bar':
            for k in self._obj_selected_pair():
                self.barriers[k][3] ^= 1
        elif self.obj_tool == 'bld':
            self.bld_model = (self.bld_model + 1) % 3
        else:
            self.bar_model ^= 1


    def _grass_like(self, r, c):
        return self.grid[r][c] in OFF_TRACK or self.grid[r][c] in FILLETS


    def _outside_grass(self):
        """The grass tiles reachable from the edge of the map (the rest of the grass is infield)."""
        seen = set()
        stack = [(r, c) for r in range(ROWS) for c in range(COLS)
                 if (r in (0, ROWS - 1) or c in (0, COLS - 1)) and self._grass_like(r, c)]
        seen.update(stack)
        while stack:
            r, c = stack.pop()
            for nr, nc in ((r + 1, c), (r - 1, c), (r, c + 1), (r, c - 1)):
                if 0 <= nr < ROWS and 0 <= nc < COLS and (nr, nc) not in seen and self._grass_like(nr, nc):
                    seen.add((nr, nc))
                    stack.append((nr, nc))
        return seen


    def _auto_edge(self, outside):
        """Line the road edges that border the infield (or the outside) with barriers facing the road."""
        out_grass = self._outside_grass()

        def wanted(r, c):
            if not (0 <= r < ROWS and 0 <= c < COLS) or not self._grass_like(r, c):
                return False
            return ((r, c) in out_grass) == outside

        # Tile edges between the road and the wanted grass, grouped by the line they're on:
        # (direction of the grass, line coordinate) -> tile indices along the line
        lines = {}
        for r in range(ROWS):
            for c in range(COLS):
                if self._grass_like(r, c):
                    continue
                for dr, dc in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                    if wanted(r + dr, c + dc):
                        if dr:
                            lines.setdefault((dr, 0, r + (1 if dr > 0 else 0)), []).append(c)
                        else:
                            lines.setdefault((0, dc, c + (1 if dc > 0 else 0)), []).append(r)

        new = []
        for (dr, dc, line), idx in lines.items():
            idx.sort()
            runs, start = [], idx[0]
            for a, b in zip(idx, idx[1:] + [None]):
                if b != a + 1:
                    runs.append((start, a + 1))
                    start = b
            for t0, t1 in runs:
                length = (t1 - t0) * TILE_WORLD
                k = max(1, int(round(length / (2 * BARRIER_HALF))))
                first = t0 * TILE_WORLD + (length - k * 2 * BARRIER_HALF) / 2 + BARRIER_HALF
                for n in range(k):
                    along = int(round(first + n * 2 * BARRIER_HALF))
                    if dr:      # the grass is above (-X) or below (+X): the barrier runs along Z
                        x = line * TILE_WORLD + dr * EDGE_SETBACK
                        new.append([x, along, 0 if dr < 0 else 1, n & 1])
                    else:       # the grass is left (-Z) or right (+Z): the barrier runs along X
                        z = line * TILE_WORLD + dc * EDGE_SETBACK
                        new.append([along, z, 2 if dc < 0 else 3, n & 1])

        # Leave out any touching the road (at the corners) or already there
        def exists(b):
            return any(o[2] == b[2] and abs(o[0] - b[0]) < 300 and abs(o[1] - b[1]) < 300 for o in self.barriers)
        added = [b for b in new if not self._barrier_on_road(b) and not exists(b)]
        if not added:
            self.status = "No edges to add barriers to"
            return
        self._obj_push_undo()
        self.barriers += added
        self.obj_sel = None
        where = 'outside' if outside else 'infield'
        self.status = (f"Added {len(added)} barriers along the {where} edges ({len(self.barriers)}/{MAX_BARRIERS})"
                       f" — Ctrl+Z to undo" + self._object_warnings())


    def _starts_on_grid(self):
        """Place all the start positions on the start grid tiles."""
        starts = starts_for_grid(self.grid)
        if starts is None:
            self.status = "No start grid on the map (paint the grid tiles 'h' / 'g' first)"
            return
        self._obj_push_undo()
        self.starts = starts
        self.obj_sel = None
        self.status = f"Placed the start positions on the grid (P1 at {starts[0][0]}, {starts[0][1]})"


    def _arena_resize(self, tiles):
        room = min(ROWS * TILE_WORLD - self.arena_x, COLS * TILE_WORLD - self.arena_z)
        size = max(12 * TILE_WORLD, min(room, self.arena + tiles * TILE_WORLD))
        if size == self.arena:
            if tiles < 0:
                limit = "already the smallest (12 tiles)"
            elif self.arena_x or self.arena_z:
                limit = "it reaches the edge of the map (Ctrl+arrow keys move it)"
            else:
                limit = f"already the whole map ({COLS} tiles)"
            self.status = f"Arena {self.arena}: {limit}"
            return
        self._obj_push_undo()
        self.arena = size
        self.status = f"Arena {self.arena} ({self.arena // TILE_WORLD} tiles) — G rebuilds its walls to fit"


    def _arena_move(self, rows, cols):
        """Move the arena by whole tiles (staying on the map); its walls move with it."""
        x = max(0, min(ROWS * TILE_WORLD - self.arena, self.arena_x + rows * TILE_WORLD))
        z = max(0, min(COLS * TILE_WORLD - self.arena, self.arena_z + cols * TILE_WORLD))
        dx, dz = x - self.arena_x, z - self.arena_z
        if not dx and not dz:
            self.status = "The arena is at the edge of the map"
            return
        self._obj_push_undo()
        # The walls (the ones made for the arena as it is) move with it
        if self.wall_arena == (self.arena_x, self.arena_z, self.arena):
            walls = set(map(tuple, arena_walls(self.arena, self.arena_x, self.arena_z)))
            for b in self.barriers:
                if tuple(b) in walls:
                    b[0] += dx
                    b[1] += dz
            self.wall_arena = (x, z, self.arena)
            moved = "with its walls"
        else:
            moved = "(G rebuilds its walls to fit)"
        self.arena_x, self.arena_z = x, z
        self.obj_sel = None
        self.status = f"Arena moved to ({x}, {z}) {moved}"


    def _arena_rebuild(self):
        """Replace the walls made for the old arena size with ones for the current size
        (the buildings are left as they are)."""
        self._obj_push_undo()
        wx, wz, ws = self.wall_arena
        old = set(map(tuple, arena_walls(ws, wx, wz)))
        self.barriers = (arena_walls(self.arena, self.arena_x, self.arena_z)
                         + [b for b in self.barriers if tuple(b) not in old])
        self.wall_arena = (self.arena_x, self.arena_z, self.arena)
        self.obj_sel = None
        self.status = f"Rebuilt the walls for an arena of {self.arena} ({len(self.barriers)} barriers)"


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
        return (self.view.x - self.scroll_x + VIEW_PAD * T + wz * T / TILE_WORLD,
                self.view.y - self.scroll_y + VIEW_PAD * T + wx * T / TILE_WORLD)


    def _screen_to_world(self, mx, my):
        T = self.tile
        wz = (mx - self.view.x + self.scroll_x - VIEW_PAD * T) * TILE_WORLD / T
        wx = (my - self.view.y + self.scroll_y - VIEW_PAD * T) * TILE_WORLD / T
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


    @staticmethod
    def _tiles_box(r0, c0, r1, c1):
        """The world area (x0, z0, x1, z1) of a block of tiles (r1, c1 included)."""
        return r0 * TILE_WORLD, c0 * TILE_WORLD, (r1 + 1) * TILE_WORLD, (c1 + 1) * TILE_WORLD


    def _objects_in(self, box):
        """Indices of the barriers and the buildings whose centre is in the world area box."""
        x0, z0, x1, z1 = box
        bars = [i for i, b in enumerate(self.barriers) if x0 <= b[0] < x1 and z0 <= b[1] < z1]
        blds = [i for i, b in enumerate(self.buildings) if x0 <= b[0] < x1 and z0 <= b[2] < z1]
        return bars, blds


    def _remove_objects_in(self, box):
        bars, blds = self._objects_in(box)
        self.barriers = [b for i, b in enumerate(self.barriers) if i not in set(bars)]
        self.buildings = [b for i, b in enumerate(self.buildings) if i not in set(blds)]
        self.obj_sel = None
        return len(bars), len(blds)


    def _copy_selection(self):
        if self.selection is None:
            self.status = "Nothing selected — drag a region first"
            return
        r0, c0, r1, c1 = self.selection
        self.clipboard = [
            [self.grid[r][c] for c in range(c0, c1 + 1)]
            for r in range(r0, r1 + 1)
        ]
        # The barriers and buildings in it, relative to its top-left corner
        ox, oz = r0 * TILE_WORLD, c0 * TILE_WORLD
        bars, blds = self._objects_in(self._tiles_box(r0, c0, r1, c1))
        self.clip_barriers = [[self.barriers[i][0] - ox, self.barriers[i][1] - oz] + self.barriers[i][2:] for i in bars]
        self.clip_buildings = [[self.buildings[i][0] - ox, self.buildings[i][1], self.buildings[i][2] - oz]
                               + self.buildings[i][3:] for i in blds]
        h = r1 - r0 + 1
        w = c1 - c0 + 1
        objs = f", {len(bars)} barriers, {len(blds)} buildings" if bars or blds else ""
        self.status = f"Copied {h}×{w} region{objs} — Ctrl+V to paste"


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

        # The objects: the ones in the pasted area (on the map) are replaced by the copied ones
        r1, c1 = min(ROWS, anchor_r + h) - 1, min(COLS, anchor_c + w) - 1
        box = self._tiles_box(anchor_r, anchor_c, r1, c1)
        self._obj_push_undo()
        self._remove_objects_in(box)
        ox, oz = anchor_r * TILE_WORLD, anchor_c * TILE_WORLD
        x0, z0, x1, z1 = box
        added = 0
        for x, z, rot, model in self.clip_barriers:
            if x0 <= x + ox < x1 and z0 <= z + oz < z1:
                self.barriers.append([x + ox, z + oz, rot, model])
                added += 1
        for x, y, z, rot, model, r in self.clip_buildings:
            if x0 <= x + ox < x1 and z0 <= z + oz < z1:
                self.buildings.append([x + ox, y, z + oz, rot, model, r])
                added += 1

        objs = f" with {added} barriers/buildings" if added else ""
        wps = sum(1 for wx, wz in self.waypoints if x0 <= wx < x1 and z0 <= wz < z1)
        wp_note = f"   ({wps} waypoints in the pasted area: check the racing line there)" if wps else ""
        self.status = f"Pasted {h}×{w}{objs} — click again to place another, Esc to finish{wp_note}"


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
        x = (mx - self.view.x + self.scroll_x) // self.tile - VIEW_PAD
        z = (my - self.view.y + self.scroll_y) // self.tile - VIEW_PAD
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
        elif self.mode == MODE_OBJECTS:
            mode_label = "[ OBJECTS ]"
            mode_color = BAR_CLR[1]
        else:
            mode_label = "[ PAINT ]"
            mode_color = (200, 200, 200)

        badge = self.font_m.render(mode_label, True, mode_color)
        bx = (PAL_W - badge.get_width()) // 2
        self.screen.blit(badge, (bx, win_h - STATUS_H - 22))


    def draw_grid(self, hover):
        T = self.tile
        ox = self.view.x - self.scroll_x + VIEW_PAD * T
        oy = self.view.y - self.scroll_y + VIEW_PAD * T
        self.screen.set_clip(self.view)

        # Tiles (only the ones in the viewport)
        x0 = max(0, self.scroll_x // T - VIEW_PAD)
        z0 = max(0, self.scroll_y // T - VIEW_PAD)
        x1 = min(COLS, (self.scroll_x + self.view.w) // T + 1 - VIEW_PAD)
        z1 = min(ROWS, (self.scroll_y + self.view.h) // T + 1 - VIEW_PAD)
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

        self.draw_objects()
        self.draw_starts()
        self.draw_paste_objects()
        self.draw_waypoints()
        self.screen.set_clip(None)


    def draw_objects(self):
        """Barriers (as lines with a tick on the side they face) and buildings (as their collision circle)."""
        T = self.tile
        scale = T / TILE_WORLD
        width = max(2, int(2 * BARRIER_R * scale))
        sel = self.obj_sel if self.mode == MODE_OBJECTS else None

        # The arena (the walls are 200 / 700 inside its edges)
        (ax, ay) = self._world_to_screen(self.arena_x, self.arena_z)
        (bx, by) = self._world_to_screen(self.arena_x + self.arena, self.arena_z + self.arena)
        pygame.draw.rect(self.screen, ARENA_CLR, (ax, ay, bx - ax, by - ay), 1)

        bars = [(i, b, False) for i, b in enumerate(self.barriers)]
        if self.obj_run is not None:
            mx, my = pygame.mouse.get_pos()
            bars += [(None, b, True) for b in self._run_barriers(self.obj_run, self._mouse_world(mx, my))]
        sel_pair = self._obj_selected_pair() if sel and sel[0] == 'bar' else []

        for i, b, preview in bars:
            a, c = (self._world_to_screen(*p) for p in self._barrier_ends(b))
            clr = BAR_CLR[b[3] & 1]
            if preview:
                clr = PASTE_CLR
            elif self._barrier_on_road(b):
                clr = OBJ_BAD
            if i is not None and i in sel_pair:
                pygame.draw.line(self.screen, SEL_CLR, a, c, width + 4)
            pygame.draw.line(self.screen, clr, a, c, width)
            # Tick on the side it faces
            fx, fz = FACING[b[2]]
            cx, cy = self._world_to_screen(b[0], b[1])
            tick = max(3, int(350 * scale))
            pygame.draw.line(self.screen, clr, (cx, cy), (cx + fz * tick, cy + fx * tick), max(1, width // 2))

        for i, b in enumerate(self.buildings):
            cx, cy = self._world_to_screen(b[0], b[2])
            r = max(3, int(b[5] * scale))
            pygame.draw.circle(self.screen, BLD_CLR[b[4] % 3], (int(cx), int(cy)), r)
            if self._building_on_road(b):
                pygame.draw.circle(self.screen, OBJ_BAD, (int(cx), int(cy)), r, 2)
            if sel == ('bld', i):
                pygame.draw.circle(self.screen, SEL_CLR, (int(cx), int(cy)), r + 3, 2)


    def draw_starts(self):
        """The start positions, as cars pointing the way they face."""
        scale = self.tile / TILE_WORLD
        length, width = max(8, int(700 * scale)), max(5, int(400 * scale))
        for i, (x, z, h) in enumerate(self.starts):
            cx, cy = self._world_to_screen(x, z)
            a = h * 2 * math.pi / 4096
            fx, fy = math.cos(a), math.sin(a)          # forward on the screen (heading 0 = right)
            sx, sy = -fy, fx
            nose = (cx + fx * length / 2, cy + fy * length / 2)
            back_l = (cx - fx * length / 2 + sx * width / 2, cy - fy * length / 2 + sy * width / 2)
            back_r = (cx - fx * length / 2 - sx * width / 2, cy - fy * length / 2 - sy * width / 2)
            clr = START_CLR[min(i, 2)]
            pygame.draw.polygon(self.screen, clr, [nose, back_l, back_r])
            if self._start_bad(i):
                pygame.draw.polygon(self.screen, OBJ_BAD, [nose, back_l, back_r], 2)
            if self.mode == MODE_OBJECTS and self.obj_sel == ('start', i):
                pygame.draw.circle(self.screen, SEL_CLR, (int(cx), int(cy)), int(length * 0.7), 2)
            if self.mode == MODE_OBJECTS and self.tile >= 9:
                lbl = self.font_s.render(START_NAMES[i], True, WP_TEXT)
                self.screen.blit(lbl, (cx + width / 2 + 2, cy - lbl.get_height()))


    def draw_paste_objects(self):
        """The copied barriers and buildings where the paste would put them."""
        if not (self.pasting and (self.clip_barriers or self.clip_buildings)):
            return
        hover = self.grid_cell(*pygame.mouse.get_pos())
        if not hover:
            return
        ox, oz = hover[0] * TILE_WORLD, hover[1] * TILE_WORLD
        scale = self.tile / TILE_WORLD
        width = max(2, int(2 * BARRIER_R * scale))
        for x, z, rot, model in self.clip_barriers:
            a, c = (self._world_to_screen(*p) for p in self._barrier_ends([x + ox, z + oz, rot, model]))
            pygame.draw.line(self.screen, PASTE_CLR, a, c, width)
        for x, y, z, rot, model, r in self.clip_buildings:
            cx, cy = self._world_to_screen(x + ox, z + oz)
            pygame.draw.circle(self.screen, PASTE_CLR, (int(cx), int(cy)), max(3, int(r * scale)), 2)


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
        elif self.mode == MODE_OBJECTS:
            mode_tag = "OBJECTS"
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
        elif self.mode == MODE_OBJECTS:
            mx, my = pygame.mouse.get_pos()
            if self.obj_tool == 'start':
                tool = "start positions"
            elif self.obj_tool == 'bar':
                tool = (f"barrier {self.bar_model + 1} {FACING_NAME[self.bar_rot]}"
                        f" {'double' if self.double_sided else 'single'}")
            else:
                tool = f"{BLD_NAME[self.bld_model % 3]}{' rot 180' if self.bld_rot else ''}"
            hover_str = (f"   {tool}   arena {self.arena} at ({self.arena_x}, {self.arena_z})"
                         f"   barriers {len(self.barriers)}/{MAX_BARRIERS}"
                         f"  buildings {len(self.buildings)}/{MAX_BUILDINGS}")
            hit = self._obj_hit(mx, my) if self.view.collidepoint(mx, my) else None
            if hit is None:
                hit = self.obj_sel
            if hit and hit[0] == 'start':
                x, z, h = self.starts[hit[1]]
                hover_str += f"   {START_NAMES[hit[1]]} ({x}, {z}) heading {h}"
            elif hit and hit[0] == 'bar' and hit[1] < len(self.barriers):
                x, z, rot, model = self.barriers[hit[1]]
                pair = ' (pair)' if self._partner(hit[1]) is not None else ''
                hover_str += f"   barrier ({x}, {z}) {FACING_NAME[rot]} model {model + 1}{pair}"
            elif hit and hit[0] == 'bld' and hit[1] < len(self.buildings):
                x, y, z, rot, model, r = self.buildings[hit[1]]
                hover_str += f"   {BLD_NAME[model % 3]} ({x}, {z}) radius {r}{' rot 180' if rot else ''}"
            elif self.view.collidepoint(mx, my):
                wx, wz = self._mouse_world(mx, my)
                hover_str += f"   world ({self._snap(wx)}, {self._snap(wz)})"
            sel_name = ''
        elif hover:
            hz, hx = hover
            ch = self.grid[hz][hx]
            tile_name = TILES.get(ch, ('', '', '', '?'))[3]
            hover_str = f"   [{hz:02d},{hx:02d}] '{ch}' {tile_name}"

        file_str = self.current_file.name if self.current_file else 'Untitled'
        tile_str = '' if self.mode in (MODE_WAYPOINT, MODE_OBJECTS) else f"'{self.selected}' {sel_name}"
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
                        if self.mode == MODE_OBJECTS and (self.obj_sel is not None or self.obj_run is not None):
                            self.obj_sel = None
                            self.obj_run = None
                            self.status = "Deselected"
                        elif self.mode == MODE_WAYPOINT and self.wp_sel is not None:
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
                    elif ev.key == pygame.K_c and ctrl and self.mode not in (MODE_WAYPOINT, MODE_OBJECTS):
                        self._copy_selection()
                    elif ev.key == pygame.K_v and ctrl and self.mode not in (MODE_WAYPOINT, MODE_OBJECTS):
                        self._start_paste()
                    elif ev.key == pygame.K_a and ctrl:
                        if self.mode == MODE_SELECT:
                            self.selection = (0, 0, ROWS - 1, COLS - 1)
                            self.status = f"Selected all ({ROWS}×{COLS}) — Ctrl+C to copy"
                    elif ev.key == pygame.K_w and not ctrl:
                        self._toggle_waypoint_mode()
                    elif ev.key == pygame.K_b and not ctrl:
                        self._toggle_objects_mode()
                    elif self.mode == MODE_OBJECTS and ev.key in (pygame.K_1, pygame.K_KP1):
                        self.obj_tool = 'bar'
                        self.obj_sel = None
                    elif self.mode == MODE_OBJECTS and ev.key in (pygame.K_2, pygame.K_KP2):
                        self.obj_tool = 'bld'
                        self.obj_sel = None
                    elif self.mode == MODE_OBJECTS and ev.key in (pygame.K_3, pygame.K_KP3):
                        self.obj_tool = 'start'
                        self.obj_sel = None
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_p and not ctrl:
                        self._starts_on_grid()
                    elif self.mode == MODE_OBJECTS and ev.key in (pygame.K_DELETE, pygame.K_BACKSPACE):
                        if self.obj_sel is not None:
                            self._obj_delete(self.obj_sel)
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_r and not ctrl:
                        self._obj_rotate()
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_x and not ctrl:
                        self._obj_flip()
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_m and not ctrl:
                        self._obj_model()
                    elif self.mode == MODE_OBJECTS and ctrl and ev.key in (pygame.K_LEFT, pygame.K_RIGHT,
                                                                            pygame.K_UP, pygame.K_DOWN):
                        self._arena_move(-1 if ev.key == pygame.K_UP else 1 if ev.key == pygame.K_DOWN else 0,
                                         -1 if ev.key == pygame.K_LEFT else 1 if ev.key == pygame.K_RIGHT else 0)
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_LEFTBRACKET:
                        self._arena_resize(-1)
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_RIGHTBRACKET:
                        self._arena_resize(1)
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_g and not ctrl:
                        self._arena_rebuild()
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_z and ctrl:
                        self._obj_undo()
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_e and not ctrl:
                        self._auto_edge(bool(ev.mod & pygame.KMOD_SHIFT))
                    elif self.mode == MODE_OBJECTS and ev.key == pygame.K_d and not ctrl:
                        self.double_sided = not self.double_sided
                        self.status = f"Double-sided barriers {'on' if self.double_sided else 'off'}"
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
                            self._obj_push_undo()
                            nb, nh = self._remove_objects_in(self._tiles_box(r0, c0, r1, c1))
                            objs = f" ({nb} barriers and {nh} buildings removed)" if nb or nh else ""
                            self.status = f"Selection filled with grass{objs}"
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
                            elif self.mode == MODE_OBJECTS:
                                shift = pygame.key.get_mods() & pygame.KMOD_SHIFT
                                self._obj_mouse_down(ev.pos[0], ev.pos[1], shift)
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
                        if self.mode == MODE_OBJECTS:
                            hit = self._obj_hit(ev.pos[0], ev.pos[1])
                            if hit is not None:
                                self._obj_delete(hit)
                        elif self.mode == MODE_WAYPOINT:
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
                        if self.mode == MODE_OBJECTS:
                            self._obj_mouse_up(ev.pos[0], ev.pos[1])

                elif ev.type == pygame.MOUSEMOTION:
                    if self.panning:
                        self.scroll_x -= ev.rel[0]
                        self.scroll_y -= ev.rel[1]
                        self._clamp_scroll()
                    elif self.wp_drag and self.wp_sel is not None:
                        self.waypoints[self.wp_sel] = self._screen_to_world(*ev.pos)
                    elif self.obj_drag:
                        self._obj_drag_to(*ev.pos)
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
