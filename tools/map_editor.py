#!/usr/bin/env python3
"""
Yaroze Racer - Ground Map Editor
Saves map data as a .h file for inclusion in game/ground.c

Controls:
  Left-click palette    select tile
  Left-click/drag grid  paint selected tile
  Right-click grid      erase to grass ('3')
  S                     save (file dialog)
  Ctrl+S                save to current file without dialog
  O                     open map file (file dialog)
  ESC                   quit
"""

import pygame
import sys
import re
import tkinter as tk
from tkinter import filedialog
from pathlib import Path

# Paths
REPO_ROOT = Path(__file__).resolve().parent.parent
TRACK_DIR = REPO_ROOT / "data" / "game" / "track"
GROUND_C  = REPO_ROOT / "game" / "ground.c"
MAPS_DIR  = REPO_ROOT / "game"   # default directory for open/save dialogs


# Map dimensions
COLS, ROWS = 30, 30


# Layout
PAL_W      = 240        # palette panel width
TILE_PX    = 24         # grid cell size in pixels
PAL_COLS   = 5          # palette columns
PAL_IMG    = 44         # palette tile image size
PAL_CW     = 48         # palette cell width  (5 * 48 = 240 = PAL_W)
PAL_CH     = 58         # palette cell height
STATUS_H   = 30
MARGIN     = 8

GRID_PX_W  = COLS * TILE_PX                            # 720
GRID_PX_H  = ROWS * TILE_PX                            # 720
WIN_W      = PAL_W + MARGIN + GRID_PX_W + MARGIN       # 976
WIN_H      = GRID_PX_H + STATUS_H + MARGIN * 2         # 776


# Colours
BG       = ( 28,  28,  40)
PANEL    = ( 42,  42,  58)
GRID_LN  = ( 55,  55,  75)
SEL_CLR  = (255, 215,  40)
STAT_BG  = ( 18,  18,  28)
TEXT_CLR = (215, 215, 215)
SAND_CLR = (200, 175,  95)
ERR_CLR  = (180,  60, 180)
HOVER_CLR= (255, 255, 255)


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


# Left-to-right, top-to-bottom order in the palette
PAL_ORDER = [
    '3', 'v', '9', '1', '2',
    '6', '7', 'i', 'j', 'k',
    'l', '4', '5', 'h', 'g',
    '8', 'a', 'c', 'd', 'b',
    'e', 'f', 'm', 'n', 'p',
    'q', 'r', 'o', 's', 't',
    'u',
]


# Image loading
def load_images(px: int) -> dict:
    """Load, rotate and scale every tile image to px×px. Returns {char: Surface}"""
    raw_cache: dict = {}
    out: dict = {}
    
    for ch, (sub, fn, deg, _) in TILES.items():
        
        if sub is None:
            surf = pygame.Surface((px, px))
            surf.fill(SAND_CLR)
            out[ch] = surf
            continue
        key = (sub, fn)
        
        if key not in raw_cache:
            path = TRACK_DIR / sub / fn
            try:
                raw_cache[key] = pygame.image.load(str(path)).convert_alpha()
            except Exception as exc:
                print(f"Warning: could not load {path}: {exc}")
                placeholder = pygame.Surface((64, 64))
                placeholder.fill(ERR_CLR)
                raw_cache[key] = placeholder

        src = raw_cache[key]
        # PNGs are portrait (road going up-down) but the game renders 0-degree
        # tiles horizontally, so we add 90° to every rotation to compensate.
        src = pygame.transform.rotate(src, -(deg + 90))
        out[ch] = pygame.transform.smoothscale(src, (px, px))
    return out


# Map I/O
def parse_map(src_path: Path) -> list:
    """
    Load groundDataTrack1 from the given .h or .c file.
    Returns a ROWS×COLS list-of-lists of char strings, defaulting to grass.
    """
    grid = [['3'] * COLS for _ in range(ROWS)]
    try:
        text = src_path.read_text(encoding='utf-8', errors='replace')
        m = re.search(
            r'groundDataTrack1\s*\[.*?\]\s*\[.*?\]\s*=\s*\{(.*?)\};',
            text, re.DOTALL
        )
        
        if m:
            for z, rs in enumerate(re.findall(r'\{([^}]+)\}', m.group(1))[:ROWS]):
                for x, ch in enumerate(re.findall(r"'(.)'", rs)[:COLS]):
                    grid[z][x] = ch
    
    except Exception as exc:
        print(f"parse_map ({src_path.name}): {exc}")
    return grid



def save_map(grid: list, dest: Path) -> None:
    """Write the map array to dest as a C header file."""
    stem = dest.stem
    guard = re.sub(r'\W', '_', stem).upper() + '_H'
    
    lines = [
        f'// {dest.name} — generated by tools/map_editor.py',
        '//',
        '// Include in game/ground.c with:',
        f'//   #include "{dest.name}"',
        '',
        f'#ifndef {guard}',
        f'#define {guard}',
        '',
        'char groundDataTrack1[30][30] = {',
    ]
    
    for z, row in enumerate(grid):
        cells = ','.join(f"'{c}'" for c in row)
        comma = ',' if z < ROWS - 1 else ''
        lines.append(f'\t{{{cells}}}{comma}')
    
    lines += ['};', '', f'#endif // {guard}', '']
    dest.write_text('\n'.join(lines), encoding='utf-8')


# Editor
class Editor:
    def __init__(self):
        # Hide the tkinter root window used for file dialogs
        self._tk = tk.Tk()
        self._tk.withdraw()

        pygame.init()
        self.screen = pygame.display.set_mode((WIN_W, WIN_H))
        self.clock = pygame.time.Clock()
        self.font_s = pygame.font.SysFont('monospace',  9)
        self.font_m = pygame.font.SysFont('monospace', 13)

        self.current_file: Path | None = None
        self.grid = [['3'] * COLS for _ in range(ROWS)]
        self.selected = '3'
        self.painting = False
        self.status   = "O = open   S = save as   Ctrl+S = save   ESC = quit"
        self._update_title()

        self.pal_imgs = load_images(PAL_IMG)
        self.grid_imgs = load_images(TILE_PX)

        # Palette cell rects
        self.pal_rects = []
        for i, ch in enumerate(PAL_ORDER):
            col = i % PAL_COLS
            row = i // PAL_COLS
            rx = MARGIN + col * PAL_CW
            ry = MARGIN + row * PAL_CH
            self.pal_rects.append((ch, pygame.Rect(rx, ry, PAL_CW - 2, PAL_CH - 2)))

        self.grid_ox = PAL_W + MARGIN
        self.grid_oy = MARGIN


    # File helpers
    def _update_title(self):
        name = self.current_file.name if self.current_file else 'Untitled'
        pygame.display.set_caption(f'Yaroze Racer — Map Editor  [{name}]')


    def _open_dialog(self):
        path = filedialog.askopenfilename(
            title='Open map file',
            initialdir=str(MAPS_DIR),
            filetypes=[('C Header / Source', '*.h *.c'), ('All files', '*.*')],
        )
        
        if path:
            p = Path(path)
            self.grid = parse_map(p)
            self.current_file = p
            self._update_title()
            self.status = f"Opened {p.name}"


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
            save_map(self.grid, p)
            self.current_file = p
            self._update_title()
            self.status = f"Saved → {p.name}"


    def _save(self):
        if self.current_file:
            save_map(self.grid, self.current_file)
            self.status = f"Saved → {self.current_file.name}"
        else:
            self._save_as_dialog()


    # Helpers
    def grid_cell(self, mx, my):
        """Return (row, col) for mouse position, or None if outside the grid."""
        x = (mx - self.grid_ox) // TILE_PX
        z = (my - self.grid_oy) // TILE_PX
        if 0 <= x < COLS and 0 <= z < ROWS:
            return z, x
        return None


    # Drawing
    def draw_palette(self):
        pygame.draw.rect(self.screen, PANEL, (0, 0, PAL_W, WIN_H))

        for ch, rect in self.pal_rects:
            # Selection highlight
            if ch == self.selected:
                pygame.draw.rect(self.screen, SEL_CLR, rect.inflate(4, 4), 3)

            # Tile image
            img = self.pal_imgs.get(ch)
            if img:
                ix = rect.x + (PAL_CW - 2 - PAL_IMG) // 2
                iy = rect.y + 2
                self.screen.blit(img, (ix, iy))

            # Char label centred below the image
            lbl = self.font_s.render(f"'{ch}'", True, TEXT_CLR)
            lx  = rect.x + (PAL_CW - 2 - lbl.get_width()) // 2
            self.screen.blit(lbl, (lx, rect.bottom - 13))


    def draw_grid(self, hover):
        ox, oy = self.grid_ox, self.grid_oy

        # Tiles
        for z in range(ROWS):
            for x in range(COLS):
                ch  = self.grid[z][x]
                img = self.grid_imgs.get(ch)
                px, py = ox + x * TILE_PX, oy + z * TILE_PX
                if img:
                    self.screen.blit(img, (px, py))
                else:
                    pygame.draw.rect(self.screen, (70, 70, 70),
                                     (px, py, TILE_PX, TILE_PX))

        # Hover highlight
        if hover:
            hz, hx = hover
            pygame.draw.rect(
                self.screen, HOVER_CLR,
                (ox + hx * TILE_PX, oy + hz * TILE_PX, TILE_PX, TILE_PX), 2
            )

        # Grid lines
        for x in range(COLS + 1):
            lx = ox + x * TILE_PX
            pygame.draw.line(self.screen, GRID_LN,
                             (lx, oy), (lx, oy + GRID_PX_H))
        for z in range(ROWS + 1):
            ly = oy + z * TILE_PX
            pygame.draw.line(self.screen, GRID_LN,
                             (ox, ly), (ox + GRID_PX_W, ly))

    def draw_status(self, hover):
        sy = WIN_H - STATUS_H
        pygame.draw.rect(self.screen, STAT_BG, (0, sy, WIN_W, STATUS_H))

        sel_name = TILES[self.selected][3]
        hover_str = ''
        if hover:
            hz, hx = hover
            ch = self.grid[hz][hx]
            tile_name = TILES.get(ch, ('', '', '', '?'))[3]
            hover_str = f"   [{hz:02d},{hx:02d}] '{ch}' {tile_name}"

        file_str = self.current_file.name if self.current_file else 'Untitled'
        msg = f"  [{file_str}]  Paint: '{self.selected}' {sel_name}{hover_str}   |   {self.status}"
        self.screen.blit(self.font_m.render(msg, True, TEXT_CLR), (4, sy + 8))


    # Main loop
    def run(self):
        while True:
            mx, my = pygame.mouse.get_pos()
            hover  = self.grid_cell(mx, my)

            for ev in pygame.event.get():
                if ev.type == pygame.QUIT:
                    pygame.quit(); sys.exit()

                elif ev.type == pygame.KEYDOWN:
                    ctrl = ev.mod & pygame.KMOD_CTRL
                    if ev.key == pygame.K_ESCAPE:
                        pygame.quit(); sys.exit()
                    elif ev.key == pygame.K_s and ctrl:
                        self._save()
                    elif ev.key == pygame.K_s:
                        self._save_as_dialog()
                    elif ev.key == pygame.K_o:
                        self._open_dialog()

                elif ev.type == pygame.MOUSEBUTTONDOWN:
                    if ev.button == 1:
                        hit_pal = False
                        for ch, rect in self.pal_rects:
                            if rect.collidepoint(ev.pos):
                                self.selected = ch
                                hit_pal = True
                                break
                        if not hit_pal:
                            self.painting = True
                            if hover:
                                self.grid[hover[0]][hover[1]] = self.selected
                    elif ev.button == 3:
                        if hover:
                            self.grid[hover[0]][hover[1]] = '3'

                elif ev.type == pygame.MOUSEBUTTONUP:
                    if ev.button == 1:
                        self.painting = False

                elif ev.type == pygame.MOUSEMOTION:
                    if self.painting and hover:
                        self.grid[hover[0]][hover[1]] = self.selected

            self.screen.fill(BG)
            self.draw_palette()
            self.draw_grid(hover)
            self.draw_status(hover)
            pygame.display.flip()
            self.clock.tick(60)


if __name__ == '__main__':
    Editor().run()
