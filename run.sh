#!/bin/sh
# Builds the game (main.exe + psx.exe) and runs it in PCSX-Redux.
# Run from WSL in the project folder:  ./run.sh
#
# Needs the toolchain from toolchain/ installed in /opt/yaroze.
# Set PCSX_REDUX to use a different emulator path.
cd "$(dirname "$0")" && exec make run
