# Net Yaroze cross toolchain: binutils 2.16.1 + GCC 2.8.1 for mipsel-unknown-ecoff.
#
# GCC 2.8.1 configured this way generates the same code as the PSX3 (Cygwin)
# Net Yaroze compiler. Built on Debian etch (i386) because old GCC needs a
# pre-4.0 host compiler. The tools are linked statically so they run on any
# x86 Linux (e.g. WSL) without 32-bit runtime libraries.
#
# The Sony SDK (libps, headers, specs) is not included: see install-sdk.sh.

FROM debian/eol:etch

RUN apt-get update >/dev/null 2>&1; \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --force-yes \
        gcc-3.4 libc6-dev make bison flex bzip2 >/dev/null && \
    ln -sf gcc-3.4 /usr/bin/gcc && ln -sf gcc-3.4 /usr/bin/cc

ARG PREFIX=/opt/yaroze
ARG TARGET=mipsel-unknown-ecoff
# The container is i386 but reports the host's x86_64 kernel.
ARG HOST=i686-pc-linux-gnu
# Large-file support: without it stat() fails with EOVERFLOW on WSL's /mnt/c,
# whose 64-bit inode numbers don't fit a 32-bit ino_t.
ENV CC="gcc -static -D_FILE_OFFSET_BITS=64"

COPY src/binutils-2.16.1.tar.bz2 src/gcc-2.8.1.tar.gz /src/
WORKDIR /build

# -all-static is libtool's flag, so it's only passed once the subdirs are configured.
RUN tar xjf /src/binutils-2.16.1.tar.bz2 && mkdir b-binutils && cd b-binutils && \
    ../binutils-2.16.1/configure --build=$HOST --host=$HOST --target=$TARGET \
        --prefix=$PREFIX --disable-nls --disable-werror && \
    make configure-host && make -j"$(nproc)" LDFLAGS=-all-static && make install

ENV PATH=$PREFIX/bin:$PATH

# Configured without --with-gnu-as, like PSX3: GCC then emits MIPS-assembler
# style output (.set nobopt, .loc debug info), which GNU as still accepts.
# libgcc is built without target libc headers and with -mgas (to skip
# mips-tfile); Yaroze programs link Sony's libsngcc instead anyway.
RUN tar xzf /src/gcc-2.8.1.tar.gz && \
    sed -i 's/^TARGET_LIBGCC2_CFLAGS = -G 0$/TARGET_LIBGCC2_CFLAGS = -G 0 -Dinhibit_libc -mgas/' \
        gcc-2.8.1/config/mips/t-ecoff && \
    mkdir b-gcc && cd b-gcc && \
    ../gcc-2.8.1/configure --build=$HOST --host=$HOST --target=$TARGET --prefix=$PREFIX && \
    make LANGUAGES=c LIBGCC1_TEST= && make LANGUAGES=c LIBGCC1_TEST= install

RUN rm -rf $PREFIX/info $PREFIX/man && \
    tar czf /yaroze-toolchain.tar.gz -C / ${PREFIX#/}
