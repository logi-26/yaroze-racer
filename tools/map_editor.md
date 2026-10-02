# Map editor

Track editor. Makes the `game/map_N.h` files (tiles, AI waypoints, barriers,
buildings, the arena and the start positions)

    pip install pygame
    python tools/map_editor.py

Reads the map size and limits from `game/ground.h` and the tile images from
`data/game/track/`.

## Things to remember

- Tile `[row][col]` covers world X from `row * 1200` and Z from `col * 1200`
  (X down the screen, Z across). The game draws the tiles the same way, so what's
  in the editor is where it ends up in the game.
- Barriers are one-sided. The tick shows the side they face. Leave double-sided
  (D) on for walls seen from both sides (back-to-back pairs, 5 apart).
- Orange waypoints = turns over 45°, where the AI brakes. Splitting a corner
  into lots of small turns stops it braking.
- Waypoint 0 doesn't have to be on the start line. The game finds the line
  (`4`/`5` tiles) and counts laps there.
- Rebuild (`make`) after saving.
- Old 30 x 30 maps get padded with grass when opened.

## Keys

Anywhere:

| Key | |
|---|---|
| Q / Tab | Paint / Select |
| W | Waypoint mode |
| B | Objects mode |
| Wheel, middle-drag, arrows | Zoom / pan |
| F | Fit the map in the window |
| O / Ctrl+S / S | Open / save / save as |
| Esc | Cancel, deselect, quit |

Paint: pick from the palette, click/drag to paint, right-click for grass.

Select (drag a box):

| Key | |
|---|---|
| Ctrl+C / Ctrl+V | Copy / paste tiles with their barriers and buildings (paste replaces what's there, waypoints aren't copied) |
| Delete | Grass it and remove its objects |
| Ctrl+A | Select all |
| Right-click | Clear the selection |

Waypoints:

| Key | |
|---|---|
| Click | Add after the selected one (or at the end) |
| Drag | Move |
| Shift+click line | Insert |
| Right-click / Delete | Remove |
| Home | Make the selected one waypoint 0 |
| R | Reverse the lap |
| I | Import `{ x, z }` pairs (e.g. the L1 recorder's output) |

Objects, tools 1 barriers / 2 buildings / 3 start positions:

| Key | |
|---|---|
| Click / drag | Place / move (snaps to 150) |
| Shift+drag | Run of barriers |
| Right-click / Delete | Remove |
| R / X | Rotate / flip (start positions: 90° / turn round) |
| M | Model: barrier 1/2, building 1 / building 2 / stand |
| D | Double-sided barriers |
| E / Shift+E | Barriers along the infield / outside road edges (450 back from the road, gaps left at corners) |
| P | Start positions onto the painted grid |
| [ / ] | Arena a tile smaller / bigger |
| Ctrl+arrows | Move the arena (walls go with it) |
| G | Rebuild the arena walls to fit (doesn't touch buildings, or walls moved by hand) |
| Ctrl+Z | Undo |

Red = object on the road, or a start position on grass / outside the arena.

Start positions: P1 me, P2 link player, A1-A5 AI. P2 and A1 share a spot
(no AI in link races).

## Adding a track to the game

1. `#include "map_N.h"` in `game/ground.c`
2. Add it to `tracks[]` in `game/ground.c`
3. Bump `NUM_TRACKS` in `game/ground.h`
4. Name and y position in `trackNames[]` / `trackY[]` in `states/menu_track_select.c`

## Limits

64 x 64 tiles, 600 barriers (perimeter walls included), 120 buildings,
7 start positions (2 player and 5 AI). 
All from `game/ground.h`.
