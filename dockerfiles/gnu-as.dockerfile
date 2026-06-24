FROM --platform=linux/amd64 ubuntu:24.04 AS binutils-builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential wget automake \
    && rm -rf /var/lib/apt/lists/*

RUN wget -O /binutils-2.7.tar.gz https://ftp.gnu.org/gnu/binutils/binutils-2.7.tar.gz --no-check-certificate && \
    tar xzf /binutils-2.7.tar.gz -C /

# Replace old config.sub/config.guess so modern x86_64 hosts are recognised
RUN cp /usr/share/automake-*/config.sub /binutils-2.7/config.sub && \
    cp /usr/share/automake-*/config.guess /binutils-2.7/config.guess

# Build libiberty, bfd, opcodes, gas, binutils, and ld.
# CFLAGS are set for compatibility with binutils-2.7 on modern GCC:
#   -std=gnu89  – allow implicit function declarations / old C idioms
#   -fcommon    – allow multiply-defined tentative definitions
RUN cd /binutils-2.7 && \
    mkdir build && \
    cd build && \
    CFLAGS="-std=gnu89 -fcommon" \
    ../configure \
        --target=mipsel-unknown-elf \
        --disable-nls && \
    make -C libiberty && \
    make -C bfd headers && \
    make -C bfd && \
    make -C opcodes && \
    make -C gas && \
    make -C binutils && \
    make -C ld

FROM ubuntu:focal as build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y build-essential gcc gcc-multilib wget

ENV VERSION=2.7.2
ENV GNUPATH=old-gnu

WORKDIR /work
RUN wget https://ftp.gnu.org/${GNUPATH}/gcc/gcc-${VERSION}.tar.gz
RUN tar xzf gcc-${VERSION}.tar.gz

WORKDIR /work/gcc-${VERSION}

COPY patches /work/patches
RUN sed -i -- 's/include <varargs.h>/include <stdarg.h>/g' *.c

RUN patch -u -p1 obstack.h -i ../patches/obstack-2.7.2.h.patch
RUN patch -u -p1 configure -i ../patches/configure.patch
RUN patch -u -p1 config.sub -i ../patches/config.sub.patch
RUN patch -u -p1 config/mips/mips.h -i ../patches/mipsel-2.7.patch
RUN patch -su -p1 < ../patches/psx-2.5.7.patch

RUN ./configure \
    --target=mips-sony-psx \
    --prefix=/opt/cross \
    --with-endian-little \
    --with-gnu-as \
    --disable-gprof \
    --disable-gdb \
    --disable-werror \
    --host=i386-pc-linux \
    --build=i386-pc-linux

RUN make --jobs $(nproc) cpp cc1 xgcc cc1plus g++ CFLAGS="-std=gnu89 -m32 -static"

COPY tests /work/tests
RUN ./cc1 -quiet -O2 /work/tests/little_endian.c && grep -E 'lbu\s\$2,0\(\$4\)' /work/tests/little_endian.s
RUN ./cc1 -quiet -O2 /work/tests/section_attribute.c
RUN ./cc1 -quiet -help </dev/null 2>&1 | grep -- -msoft-float

RUN mv xgcc gcc
RUN mkdir /build && cp cpp cc1 gcc cc1plus g++ /build/

FROM scratch AS export
COPY --from=build /build/* .
COPY --from=binutils-builder /binutils-2.7/build/gas/as.new ./as
COPY --from=binutils-builder /binutils-2.7/build/binutils/objdump ./objdump
COPY --from=binutils-builder /binutils-2.7/build/ld/ld.new ./ld