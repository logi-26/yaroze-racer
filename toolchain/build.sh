#!/bin/sh
# Builds the Net Yaroze GCC toolchain in Docker and writes yaroze-toolchain.tar.gz.
# Run from WSL:  ./toolchain/build.sh
# Then install:  sudo tar xzf toolchain/yaroze-toolchain.tar.gz -C /
#                sudo ./toolchain/install-sdk.sh /path/to/psx3
set -e
cd "$(dirname "$0")"

mkdir -p src
[ -f src/binutils-2.16.1.tar.bz2 ] || wget -P src https://ftp.gnu.org/gnu/binutils/binutils-2.16.1.tar.bz2
[ -f src/gcc-2.8.1.tar.gz ]         || wget -P src https://ftp.gnu.org/gnu/gcc/gcc-2.8.1.tar.gz

# Without Docker Desktop's WSL integration, fall back to the Windows CLI.
docker=docker
docker version >/dev/null 2>&1 || docker=docker.exe

$docker build --platform linux/386 -t yaroze-toolchain .
id=$($docker create yaroze-toolchain)
$docker cp "$id:/yaroze-toolchain.tar.gz" .
$docker rm "$id" >/dev/null
echo "Built toolchain/yaroze-toolchain.tar.gz"
