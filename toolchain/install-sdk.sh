#!/bin/sh
# Installs the Net Yaroze SDK (headers, libps, libsngcc, specs, linker script)
# from an original PSX3 install into the toolchain prefix.
#
# usage: sudo ./toolchain/install-sdk.sh /mnt/c/yaroze_vm_share/psx3 [/opt/yaroze]
#
# The SDK is Sony's and is not redistributed with this repo.
set -e
SDK=${1:?path to psx3 folder}
PREFIX=${2:-/opt/yaroze}
TARGET=mipsel-unknown-ecoff
TOOLDIR=$PREFIX/$TARGET
GCCLIB=$PREFIX/lib/gcc-lib/$TARGET/2.8.1

[ -f "$SDK/mips-unknown-ecoff/lib/LIBPS.A" ] || { echo "no LIBPS.A under $SDK" >&2; exit 1; }
[ -d "$GCCLIB" ] || { echo "toolchain not installed at $PREFIX" >&2; exit 1; }

# Headers: the SDK was made on DOS/Windows, so file names are upper case and
# lines end in CRLF (which breaks backslash-continued macros on Linux).
mkdir -p "$TOOLDIR/include"
(cd "$SDK/mips-unknown-ecoff/include" && find . -type f) | while read -r f; do
	dst="$TOOLDIR/include/$(echo "$f" | tr 'A-Z' 'a-z')"
	mkdir -p "$(dirname "$dst")"
	tr -d '\r' < "$SDK/mips-unknown-ecoff/include/$f" > "$dst"
done

# PSX3 had no gcc-lib include dir: stddef.h, stdarg.h etc. come from the SDK.
# Keep GCC's own headers aside rather than letting them shadow the SDK's.
if [ -d "$GCCLIB/include" ]; then
	rm -rf "$GCCLIB/include.gcc"
	mv "$GCCLIB/include" "$GCCLIB/include.gcc"
fi

mkdir -p "$TOOLDIR/lib"
cp "$SDK/mips-unknown-ecoff/lib/LIBPS.A" "$TOOLDIR/lib/libps.a"
cp "$SDK/mips-unknown-ecoff/lib/Libsngcc.a" "$TOOLDIR/lib/libsngcc.a"

# Original linker script: entry is _start (from libps) rather than binutils' 'start'.
tr -d '\r' < "$SDK/mips-unknown-ecoff/lib/ldscripts/mipsidt.x" |
	sed "s|SEARCH_DIR(.*);|SEARCH_DIR($TOOLDIR/lib);|" > "$TOOLDIR/lib/yaroze.x"

# Original driver specs (-lps instead of libc, -lsngcc instead of libgcc, no crt0),
# plus our linker script.
tr -d '\r' < "$SDK/lib/gcc-lib/mips-unknown-ecoff/2.8.1/specs" |
	sed "/^\*link:/{n;s|^|-T $TOOLDIR/lib/yaroze.x |}" > "$GCCLIB/specs"

echo "Net Yaroze SDK installed into $PREFIX"
