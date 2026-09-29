# rsdlink

Converts 3D models from RSD format (`.rsd` with its `.ply`, `.mat` and
textures) into the PlayStation TMD format that the game loads.

`rsdlink` is Sony's DOS program `RSDLINK.EXE` (version 3.72, from the Net
Yaroze SDK). In WSL it runs inside DOSBox without a window, through a small
wrapper command, and produces exactly the same TMD files as running it on
Windows/DOS.

## Setup (once)

    sudo apt install dosbox
    sudo sh ./toolchain/install-rsdlink.sh /mnt/c/yaroze_vm_share/psx-bin/RSDLINK.EXE

This installs the `rsdlink` command into `/opt/yaroze/bin`. Run the second
command again after `install-rsdlink.sh` changes, to update the installed
command. The makefile
adds that folder to the PATH; for your own shell, add
`export PATH=/opt/yaroze/bin:$PATH` to `~/.bashrc`.

`RSDLINK.EXE` itself is not in this repo (it is Sony's). The copy in
`C:\yaroze_vm_share\psx-bin` came from the SDK in the Net Yaroze VM.

## Common tasks

Rebuild all the game's models (the ones listed in `data/game/CONV_ALL.BAT`):

    make models

Convert one model: go to its folder and run the command from its
`CONV2.BAT`, e.g.

    cd data/game/barrier1
    rsdlink -v -o barrier1.tmd barrier1.rsd

Convert with a scale factor:

    rsdlink -s 2 -o stand.tmd stand.rsd

Each model takes about a second (DOSBox has to start each time).

## Options

    rsdlink [options] rsd-files...

| Option | Meaning |
|---|---|
| `-o FILE` | Output TMD file name. Default: `a.tmd`. |
| `-s FACTOR` | Scale the model by FACTOR (decimal, e.g. `0.5`). Default: 1.0. |
| `-t X Y Z` | Move the model by X, Y, Z. Default: 0 0 0. |
| `-v` | Verbose: print a summary (polygon, vertex and normal counts, primitive types, file size). |
| `-info` | Print details. |
| `-id PRJ-FILE` | Read a PRJ file and write a C list of object IDs. |

Several `.rsd` files can be given at once to put several objects into one TMD.

## The RSD files

A `.rsd` file is a short text file listing the other files of the model:

    @RSD940102
    PLY=barrier1.ply        vertices, normals and polygons
    MAT=barrier1.mat        material of each polygon (colour, shading, texture)
    NTEX=1                  number of textures
    TEX[0]=barrier1.tim     texture files

All of these are read from the current folder.

**Textures:** rsdlink reads each `.tim` listed in the `.rsd` to find where the
texture sits in VRAM (its image and CLUT positions), and stores that in the
TMD. So after moving a texture (a different `-org`/`-plt` in `png2tim`),
run rsdlink again for every model that uses it. If a listed `.tim` is
missing, RSDLINK would still write a TMD with wrong texture positions; the
wrapper treats this as a failure and removes it (see Errors).

## Things to know

- **DOS file names.** RSDLINK is a DOS program, so every file it reads or
  writes must have a DOS 8.3 name (at most 8 characters, a dot, at most 3),
  and must be in the current folder or a folder below it.
- **Upper case output.** New files are created with upper case names, e.g.
  `BARRIER1.TMD`. On `/mnt/c` this makes no difference, because file names
  there are not case sensitive.
- **Messages.** RSDLINK's normal output (e.g. the `-v` summary) goes to the
  terminal as usual. Its error messages and usage text are written straight
  to the DOS screen, which DOS can't redirect, so after each run the wrapper
  saves the DOSBox screen (with a tiny helper, `SCRDUMP.COM`) and prints the
  messages from it. Only the last 25 lines of the screen are kept, which is
  more than RSDLINK's messages need. Its copyright banner and progress lines
  are left out.
- **Success or failure.** RSDLINK itself always reports failure (DOS
  errorlevel 255), even when it works, so the wrapper decides: it fails
  (exit status 1) if RSDLINK printed an error message, or if the output file
  wasn't created. Warnings, such as an old file version, are shown but don't
  fail. `make models` stops at the first model that fails.
- **The tunnel model** (`data/game/tunnel`) is not in `CONV_ALL.BAT`, so
  `make models` leaves it alone. Its `tunnel.tmd` in the repo differs from
  what its `CONV2.BAT` produces (different scale and normals), so running
  that `CONV2.BAT` would change the model.

## Errors

RSDLINK's own message comes first, then the wrapper's summary, e.g.

    Cannot Open: "barrier1.tim" for barrier1.mat
    rsdlink: failed, barrier1.tmd not created

| Message | Fix |
|---|---|
| `Cannot open: "FILE"` | The `.rsd` (or a file it lists) isn't there, or its name isn't a DOS 8.3 name. |
| `Cannot Open: "FILE.tim" for FILE.mat` | A texture listed in the `.rsd` is missing. |
| `-x : Illegal option.` | Unknown option; the usage text follows it. |
| `"FILE" has a bad version` / `is not a ... file` | The file isn't a valid RSD, PLY or MAT file. |
| `rsdlink: FILE was not created` | RSDLINK produced no output and no known error message; check its messages above. Running `rsdlink` with no arguments shows the usage text and this line. |
| `rsdlink: DOSBox failed to run RSDLINK.EXE` | DOSBox is missing or failed to start: `sudo apt install dosbox`, and rerun `install-rsdlink.sh`. |
| `rsdlink: command not found` | Add `/opt/yaroze/bin` to your PATH, or run the setup above. |
