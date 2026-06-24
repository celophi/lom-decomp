# ─────────────────────────────────────────────────────────────────────────────
# dev.dockerfile — primary development container for the Legend of Mana decomp.
#
# This is the image you build and work inside day-to-day: it bundles every PSX
# compiler toolchain, the PSYQ 4.1 SDK, and the Python tooling needed to split,
# build, and diff the ROM. Unlike pipeline.dockerfile (the CI image), it does NOT
# embed any ROM data — you mount the repo into the running container instead.
#
# Build from the repo root (context must be the repo root for requirements.txt
# and tools/ to be copied in):
#   docker build -t lom-dev -f dockerfiles/dev.dockerfile .
#
# See the README "Build the Development Environment" section for full usage.
# ─────────────────────────────────────────────────────────────────────────────

# Stage 1: grab compiler artifacts
FROM old-gcc/gcc-2.8.0-psx AS psx-gcc-2.8.0
FROM old-gcc/gcc-2.6.0-psx AS psx-gcc-2.6.0
FROM old-gcc/gcc-2.7.2-cdk AS psx-gcc-2.7.2-cdk
FROM ghcr.io/celophi/gcc-2.7.2-psx-gnu:v1.1 AS psx-gcc-2.7.2-gnuas

# Use Ubuntu LTS for stability and wide package availability
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y -o APT::Immediate-Configure=false \
    build-essential \
    python3 \
    python3-pip \
    gcc-mips-linux-gnu \
    binutils-mips-linux-gnu \
    gcc-mipsel-linux-gnu \
    binutils-mipsel-linux-gnu \
    less make vim git wget curl file \
    dos2unix \
    && rm -rf /var/lib/apt/lists/*

# Copy GCC pieces from the toolchain stage

## PSX gcc 2.8.0 (ASPSX)
COPY --from=psx-gcc-2.8.0 /cpp     /opt/psx-gcc-2.8.0/cpp
COPY --from=psx-gcc-2.8.0 /cc1     /opt/psx-gcc-2.8.0/cc1
COPY --from=psx-gcc-2.8.0 /gcc     /opt/psx-gcc-2.8.0/gcc
COPY --from=psx-gcc-2.8.0 /cc1plus /opt/psx-gcc-2.8.0/cc1plus
COPY --from=psx-gcc-2.8.0 /g++     /opt/psx-gcc-2.8.0/g++

## PSX gcc 2.6.0 (ASPSX)
COPY --from=psx-gcc-2.6.0 /cpp     /opt/psx-gcc-2.6.0/cpp
COPY --from=psx-gcc-2.6.0 /cc1     /opt/psx-gcc-2.6.0/cc1
COPY --from=psx-gcc-2.6.0 /gcc     /opt/psx-gcc-2.6.0/gcc
COPY --from=psx-gcc-2.6.0 /cc1plus /opt/psx-gcc-2.6.0/cc1plus
COPY --from=psx-gcc-2.6.0 /g++     /opt/psx-gcc-2.6.0/g++

## PSX gcc 2.7.2 (Cygnus CDK)
COPY --from=psx-gcc-2.7.2-cdk /cpp     /opt/psx-gcc-2.7.2-cdk/cpp
COPY --from=psx-gcc-2.7.2-cdk /cc1     /opt/psx-gcc-2.7.2-cdk/cc1
COPY --from=psx-gcc-2.7.2-cdk /gcc     /opt/psx-gcc-2.7.2-cdk/gcc
COPY --from=psx-gcc-2.7.2-cdk /cc1plus /opt/psx-gcc-2.7.2-cdk/cc1plus
COPY --from=psx-gcc-2.7.2-cdk /g++     /opt/psx-gcc-2.7.2-cdk/g++

## PSX gcc 2.7.2 (GNU as)
COPY --from=psx-gcc-2.7.2-gnuas /cpp      /opt/psx-gcc-2.7.2-gnuas/cpp
COPY --from=psx-gcc-2.7.2-gnuas /cc1      /opt/psx-gcc-2.7.2-gnuas/cc1
COPY --from=psx-gcc-2.7.2-gnuas /gcc      /opt/psx-gcc-2.7.2-gnuas/gcc
COPY --from=psx-gcc-2.7.2-gnuas /cc1plus  /opt/psx-gcc-2.7.2-gnuas/cc1plus
COPY --from=psx-gcc-2.7.2-gnuas /g++      /opt/psx-gcc-2.7.2-gnuas/g++
COPY --from=psx-gcc-2.7.2-gnuas /as       /opt/psx-gcc-2.7.2-gnuas/as
COPY --from=psx-gcc-2.7.2-gnuas /ld       /opt/psx-gcc-2.7.2-gnuas/ld
COPY --from=psx-gcc-2.7.2-gnuas /objdump  /opt/psx-gcc-2.7.2-gnuas/objdump

# Wibo (WINE alternative)
COPY --from=ghcr.io/decompals/wibo:1.0.1 /usr/local/bin/wibo /usr/bin/

RUN mkdir /opt/psyq4.1

RUN wget -O psyq4.1.tar.gz "https://github.com/mkst/esa/releases/download/psyq-binaries/psyq4.1.tar.gz"
RUN tar xvzf psyq4.1.tar.gz --strip-components=1 -C /opt/psyq4.1

RUN wget -O psyq-obj-parser.tar.gz https://github.com/decompme/compilers/releases/download/compilers/psyq-obj-parser.tar.gz?2025-03-18
RUN tar xvzf psyq-obj-parser.tar.gz -C /opt/psyq4.1/

RUN cat <<'EOF' > /opt/psyq4.1/SN.INI
[ccpsx]
compiler_path=/opt/psyq4.1
assembler_path=/opt/psyq4.1
tmpdir=/tmp
EOF

ENV SN_PATH=/opt/psyq4.1

COPY requirements.txt /build-lom/requirements.txt
COPY tools            /build-lom/tools

WORKDIR /build-lom
RUN pip install -r /build-lom/requirements.txt

RUN mkdir /lom
WORKDIR /lom