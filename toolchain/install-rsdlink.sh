#!/bin/sh
# Installs an 'rsdlink' command that runs Sony's DOS RSDLINK.EXE (RSD -> TMD)
# in headless DOSBox, in the current directory. Output is byte-identical to
# running it in DOS/Windows.
#
# usage: sudo apt install dosbox
#        sudo ./toolchain/install-rsdlink.sh /mnt/c/yaroze_vm_share/psx-bin/RSDLINK.EXE [/opt/yaroze]
#
# RSDLINK.EXE is Sony's (Net Yaroze SDK, PSX\BIN) and is not redistributed here.
set -e
EXE=${1:?path to RSDLINK.EXE}
PREFIX=${2:-/opt/yaroze}
DOS=$PREFIX/libexec/dos

command -v dosbox >/dev/null || { echo "dosbox not found: sudo apt install dosbox" >&2; exit 1; }
[ -f "$EXE" ] || { echo "no such file: $EXE" >&2; exit 1; }

mkdir -p "$DOS" "$PREFIX/bin"
cp "$EXE" "$DOS/RSDLINK.EXE"

# No window, no sound, run as fast as possible.
cat > "$DOS/dosbox.conf" <<'EOF'
[sdl]
output=surface
[cpu]
core=dynamic
cycles=max
[mixer]
nosound=true
[speaker]
pcspeaker=false
EOF

# SCRDUMP.COM: saves the 80x25 text screen (B800:0000, 4000 bytes) to
# _RSDSCR.BIN. RSDLINK prints its errors and usage to the screen (stderr),
# which DOS can't redirect, so the wrapper reads them back from this dump.
#   mov ah,3Ch / xor cx,cx / mov dx,name / int 21h / jc done   ; create file
#   mov bx,ax / push ds / mov ax,0B800h / mov ds,ax / xor dx,dx
#   mov cx,4000 / mov ah,40h / int 21h / pop ds                 ; write screen
#   mov ah,3Eh / int 21h                                        ; close
#   done: mov ax,4C00h / int 21h                                ; exit
#   name: db "_RSDSCR.BIN",0
printf '\264\074\061\311\272\046\001\315\041\162\026\211\303\036\270\000\270\216\330\061\322\271\240\017\264\100\315\041\037\264\076\315\041\270\000\114\315\041_RSDSCR.BIN\000' \
	> "$DOS/SCRDUMP.COM"

sed "s|@DOS@|$DOS|" > "$PREFIX/bin/rsdlink" <<'EOF'
#!/bin/sh
# rsdlink wrapper: runs RSDLINK.EXE in headless DOSBox with the current
# directory as C:. File names must be DOS 8.3 names.
#
# RSDLINK always exits with errorlevel 255, so success means the output file
# (-o, default a.tmd) was (re)created. Its normal output goes to the terminal;
# its messages (errors, usage) are read back from a dump of the DOS screen.
DOS=@DOS@

out=a.tmd
prev=
for a; do [ "$prev" = -o ] && out=$a; prev=$a; done
find . -maxdepth 1 -iname "$out" -delete

log=_RSDLNK.LOG
scr=_RSDSCR.BIN
rm -f "$log" "$scr"
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy dosbox -conf "$DOS/dosbox.conf" \
	-c "mount c \"$PWD\"" -c "mount d \"$DOS\"" -c "c:" -c "cls" \
	-c "d:\\rsdlink.exe $* > $log" -c "d:\\scrdump.com" -c "exit" > /dev/null 2>&1

# Screen text: every other byte of the dump (the rest are colours), 80 per
# line, without the DOS prompt, blank lines, the copyright banner and the
# "(Reading ...)" progress lines.
screen() {
	[ -f "$scr" ] || return 0
	od -An -v -tu1 "$scr" | awk '{
		for (i = 1; i <= NF; i++) {
			n++
			if (n % 2) { c = $i; if (c < 32 || c > 126) c = 32; line = line sprintf("%c", c) }
			if (n % 160 == 0) { sub(/ +$/, "", line); print line; line = "" }
		}
	}' | grep -v -e '^[A-Z]:\\>' -e '^$' -e '^(C) 19' -e '^rsdlink Version' -e '^(Reading' -e '^(Writing'
}
msgs=$(screen)
rm -f "$scr"

# RSDLINK still writes a TMD after some errors (e.g. a missing texture, which
# leaves wrong texture positions in it), so its error messages count as failure.
errors='Cannot|ERROR|Error:|Unexpected EOF|Fail to|Bad argument|is not a |bad version|Illegal option|out of the PS-X'

if [ ! -f "$log" ]; then
	echo "rsdlink: DOSBox failed to run RSDLINK.EXE" >&2
	exit 1
fi
tr -d '\r' < "$log"
rm -f "$log"
[ -n "$msgs" ] && printf '%s\n' "$msgs" >&2

if printf '%s\n' "$msgs" | grep -qE "$errors"; then
	find . -maxdepth 1 -iname "$out" -delete
	echo "rsdlink: failed, $out not created" >&2
	exit 1
fi
if [ -z "$(find . -maxdepth 1 -iname "$out")" ]; then
	echo "rsdlink: $out was not created" >&2
	exit 1
fi
EOF
chmod 755 "$PREFIX/bin/rsdlink"

echo "rsdlink installed into $PREFIX/bin"
