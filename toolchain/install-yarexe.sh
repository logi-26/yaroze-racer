#!/bin/sh
# Builds gwald's Yarexe (siocons 'auto' script -> psx.exe packager) for Linux
# and installs it with a wrapper that accepts DOS-style scripts.
#
# usage: sudo ./toolchain/install-yarexe.sh [/opt/yaroze]
# needs: git, gcc (with static libc: libc6-dev)
#
# Yarexe's licence forbids distributing modified source, so it is fetched
# unmodified; the two Linux build errors are fixed with a forced-include header.
set -e
PREFIX=${1:-/opt/yaroze}
REPO=https://github.com/gwald/Yarexe
COMMIT=cc79e98

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

git clone -q "$REPO" "$tmp/Yarexe"
git -C "$tmp/Yarexe" checkout -q "$COMMIT"

# yarexe.c uses errno without <errno.h>, and calls the static casepath()
# before defining it.
cat > "$tmp/prelude.h" <<'EOF'
#include <errno.h>
static int casepath(char const *path, char *r);
EOF
gcc -static -O3 -w -include "$tmp/prelude.h" "$tmp/Yarexe/yarexe.c" -o "$tmp/yarexe"

mkdir -p "$PREFIX/libexec" "$PREFIX/bin"
install -m 755 "$tmp/yarexe" "$PREFIX/libexec/yarexe"

# The Linux build doesn't translate the script's '\' paths, so the wrapper
# hands it a copy with '/' separators and no CRs. Paths stay relative to the
# current directory, as with siocons.
cat > "$PREFIX/bin/yarexe" <<EOF
#!/bin/sh
# yarexe wrapper: yarexe <siocons script> [-v]
[ -f "\$1" ] || exec "$PREFIX/libexec/yarexe" "\$@"
script=\$(mktemp ./.yarexe-XXXXXX)
trap 'rm -f "\$script"' EXIT
tr -d '\\015' < "\$1" | tr '\\134' '/' > "\$script"
shift
"$PREFIX/libexec/yarexe" "\$script" "\$@"
EOF
chmod 755 "$PREFIX/bin/yarexe"

echo "yarexe installed into $PREFIX/bin"
