#!/usr/bin/env python3
"""
Yaroze Racer - Ground Map Editor
Saves map data as a .h file for inclusion in game/ground.c

Controls:
  Left-click palette        select tile
  Q / Tab                   toggle Paint / Select mode
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
  S                         save (file dialog)
  Ctrl+S                    save to current file
  O                         open map file
  ESC                       cancel paste / clear selection / quit
"""

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

# Map dimensions
COLS, ROWS = 30, 30

# Layout
PAL_W    = 240
TILE_PX  = 24
PAL_COLS = 5
PAL_IMG  = 44
PAL_CW   = 48
PAL_CH   = 58
STATUS_H = 30
MARGIN   = 8

GRID_PX_W = COLS * TILE_PX
GRID_PX_H = ROWS * TILE_PX
WIN_W     = PAL_W + MARGIN + GRID_PX_W + MARGIN
WIN_H     = GRID_PX_H + STATUS_H + MARGIN * 2

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

# Mode constants
MODE_PAINT  = 'paint'
MODE_SELECT = 'select'

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
        src = pygame.transform.rotate(src, -(deg + 90))
        out[ch] = pygame.transform.smoothscale(src, (px, px))
    return out


def parse_map(src_path: Path) -> list:
    """Load any groundData* array from a .h or .c file."""
    grid = [['3'] * COLS for _ in range(ROWS)]
    try:
        text = src_path.read_text(encoding='utf-8', errors='replace')
        m = re.search(
            r'groundData\w+\s*\[.*?\]\s*\[.*?\]\s*=\s*\{(.*?)\};',
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
    stem  = dest.stem
    guard = re.sub(r'\W', '_', stem).upper() + '_H'
    # Derive array name from filename: map_1 -> groundDataMap1, map_2 -> groundDataMap2
    parts      = re.split(r'[_\-]', stem)
    array_name = 'groundData' + ''.join(p.capitalize() for p in parts)

    lines = [
        f'// {dest.name} — generated by tools/map_editor.py',
        '//',
        '// Include in game/ground.c with:',
        f'//   #include "{dest.name}"',
        '',
        f'#ifndef {guard}',
        f'#define {guard}',
        '',
        f'char {array_name}[30][30] = {{',
    ]
    for z, row in enumerate(grid):
        cells = ','.join(f"'{c}'" for c in row)
        comma = ',' if z < ROWS - 1 else ''
        lines.append(f'\t{{{cells}}}{comma}')
    lines += ['};', '', f'#endif // {guard}', '']
    dest.write_text('\n'.join(lines), encoding='utf-8')


class Editor:
    def __init__(self):
        self._tk = tk.Tk()
        self._tk.withdraw()

        pygame.init()
        self.screen = pygame.display.set_mode((WIN_W, WIN_H))
        self.clock  = pygame.time.Clock()
        self.font_s = pygame.font.SysFont('monospace',  9)
        self.font_m = pygame.font.SysFont('monospace', 13)

        self.current_file: Path | None = None
        self.grid     = [['3'] * COLS for _ in range(ROWS)]
        self.selected = '3'
        self.status   = "O=open  S=save  Q=select mode  Ctrl+C/V=copy/paste"
        self._update_title()

        self.pal_imgs  = load_images(PAL_IMG)
        self.grid_imgs = load_images(TILE_PX)

        self.pal_rects = []
        for i, ch in enumerate(PAL_ORDER):
            col = i % PAL_COLS
            row = i // PAL_COLS
            rx  = MARGIN + col * PAL_CW
            ry  = MARGIN + row * PAL_CH
            self.pal_rects.append((ch, pygame.Rect(rx, ry, PAL_CW - 2, PAL_CH - 2)))

        self.grid_ox = PAL_W + MARGIN
        self.grid_oy = MARGIN

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
        x = (mx - self.grid_ox) // TILE_PX
        z = (my - self.grid_oy) // TILE_PX
        if 0 <= x < COLS and 0 <= z < ROWS:
            return z, x
        return None


    def draw_palette(self):
        pygame.draw.rect(self.screen, PANEL, (0, 0, PAL_W, WIN_H))

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
        else:
            mode_label = "[ PAINT ]"
            mode_color = (200, 200, 200)

        badge = self.font_m.render(mode_label, True, mode_color)
        bx = (PAL_W - badge.get_width()) // 2
        self.screen.blit(badge, (bx, WIN_H - STATUS_H - 22))


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

        # Selection overlay (shown in select mode)
        if self.mode == MODE_SELECT:
            sel = self._active_sel_rect()
            if sel is not None:
                r0, c0, r1, c1 = sel
                px = ox + c0 * TILE_PX
                py = oy + r0 * TILE_PX
                w  = (c1 - c0 + 1) * TILE_PX
                h  = (r1 - r0 + 1) * TILE_PX
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
                        px2 = ox + tc * TILE_PX
                        py2 = oy + tr * TILE_PX
                        if img:
                            ghost = img.copy()
                            ghost.set_alpha(170)
                            self.screen.blit(ghost, (px2, py2))
                        tint = pygame.Surface((TILE_PX, TILE_PX), pygame.SRCALPHA)
                        tint.fill((*PASTE_CLR, 70))
                        self.screen.blit(tint, (px2, py2))
            ph = len(self.clipboard)
            pw = len(self.clipboard[0]) if ph else 0
            clip_h = min(ph, ROWS - hr)
            clip_w = min(pw, COLS - hc)
            if clip_h > 0 and clip_w > 0:
                pygame.draw.rect(self.screen, PASTE_CLR,
                                 (ox + hc * TILE_PX, oy + hr * TILE_PX,
                                  clip_w * TILE_PX, clip_h * TILE_PX), 2)
        elif not self.pasting and hover:
            hz, hx = hover
            pygame.draw.rect(
                self.screen, HOVER_CLR,
                (ox + hx * TILE_PX, oy + hz * TILE_PX, TILE_PX, TILE_PX), 2
            )

        # Grid lines
        for x in range(COLS + 1):
            lx = ox + x * TILE_PX
            pygame.draw.line(self.screen, GRID_LN, (lx, oy), (lx, oy + GRID_PX_H))
        for z in range(ROWS + 1):
            ly = oy + z * TILE_PX
            pygame.draw.line(self.screen, GRID_LN, (ox, ly), (ox + GRID_PX_W, ly))


    def draw_status(self, hover):
        sy = WIN_H - STATUS_H
        pygame.draw.rect(self.screen, STAT_BG, (0, sy, WIN_W, STATUS_H))

        if self.pasting:
            mode_tag = "PASTE"
        elif self.mode == MODE_SELECT:
            mode_tag = "SELECT"
        else:
            mode_tag = "PAINT"

        sel_name  = TILES[self.selected][3]
        hover_str = ''
        if hover:
            hz, hx = hover
            ch = self.grid[hz][hx]
            tile_name = TILES.get(ch, ('', '', '', '?'))[3]
            hover_str = f"   [{hz:02d},{hx:02d}] '{ch}' {tile_name}"

        file_str = self.current_file.name if self.current_file else 'Untitled'
        msg = (f"  [{mode_tag}]  [{file_str}]  "
               f"'{self.selected}' {sel_name}{hover_str}   |   {self.status}")
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
                        if self.pasting:
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
                    elif ev.key == pygame.K_c and ctrl:
                        self._copy_selection()
                    elif ev.key == pygame.K_v and ctrl:
                        self._start_paste()
                    elif ev.key == pygame.K_a and ctrl:
                        if self.mode == MODE_SELECT:
                            self.selection = (0, 0, ROWS - 1, COLS - 1)
                            self.status = f"Selected all ({ROWS}×{COLS}) — Ctrl+C to copy"
                    elif ev.key == pygame.K_DELETE:
                        if self.mode == MODE_SELECT and self.selection:
                            r0, c0, r1, c1 = self.selection
                            for r in range(r0, r1 + 1):
                                for c in range(c0, c1 + 1):
                                    self.grid[r][c] = '3'
                            self.status = "Selection filled with grass"
                    elif ev.key in (pygame.K_q, pygame.K_TAB):
                        self._toggle_mode()

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
                            if self.pasting and hover:
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
                        if self.mode == MODE_SELECT:
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

                elif ev.type == pygame.MOUSEMOTION:
                    if self.sel_drag and hover:
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
