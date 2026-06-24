# This dockerfile is for running the github pipeline for object diffing.
# It expects an archive to exist at the local path ROM/ containing a .7z file with the game files. 
# The .7z should contain a .bin and .cue pair, which will be converted to an ISO and unpacked into /rom.
# To build this run the following from the repo root (replace the version string):
# docker build -t ghcr.io/celophi/slus-01013:v4.0 -f dockerfiles/pipeline.dockerfile dockerfiles

# Stage 1: grab compiler artifacts
FROM old-gcc/gcc-2.8.0-psx AS psx-gcc-2.8.0
FROM old-gcc/gcc-2.6.0-psx AS psx-gcc-2.6.0
FROM old-gcc/gcc-2.7.2-cdk AS psx-gcc-2.7.2-cdk
FROM ghcr.io/celophi/gcc-2.7.2-psx-gnu:v1.1 AS psx-gcc-2.7.2-gnuas

# Use Ubuntu LTS for stability and wide package availability
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install required tools
RUN apt-get update && apt-get install -y -o APT::Immediate-Configure=false \
    build-essential \
    python3 \
    python3-pip \
    p7zip-full \
    bchunk \
    gcc-mips-linux-gnu \
    binutils-mips-linux-gnu \
    gcc-mipsel-linux-gnu \
    binutils-mipsel-linux-gnu \
    less \
    make \
    vim \
    git \
    wget \
    curl \
    file \
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

# Create a directory for the game files
WORKDIR /rom

# Copy the compressed ROM (.7z containing a .bin/.cue pair) into the container
COPY ROM/ /tmp/rom-src/

# Extract the archive, convert the bin/cue to an ISO, then unpack the
# ISO9660 filesystem so /rom holds the game's actual files and folders.
# Expected SHA-256 of the extracted .bin
ENV ROM_BIN_SHA256=92806CF96D718415CCB614738071FB37BB99438D5037863C4C9F2997FB54048D

RUN set -e; \
    cd /tmp/rom-src; \
    7z x -y *.7z; \
    bin=$(ls *.bin); \
    cue=$(ls *.cue); \
    echo "$(echo "${ROM_BIN_SHA256}" | tr 'A-Z' 'a-z')  ${bin}" | sha256sum -c -; \
    bchunk "$bin" "$cue" track; \
    7z x -y -o/rom track01.iso; \
    rm -rf /tmp/rom-src
