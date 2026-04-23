# This dockerfile is for running the github pipeline for object diffing.

# Stage 1: grab compiler artifacts
FROM old-gcc/gcc-2.8.0-psx AS toolchain-psx
FROM old-gcc/gcc-2.6.0-psx AS toolchain-2.6.0-psx
FROM old-gcc/gcc-2.7.2-cdk AS toolchain-cdk
FROM ghcr.io/celophi/gcc-2.7.2-psx-gnu:v1.1 AS toolchain-gnu

# Use Ubuntu LTS for stability and wide package availability
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install required tools
RUN apt-get update && apt-get install -y -o APT::Immediate-Configure=false \
    build-essential \
    python3 \
    python3-pip \
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

## PSX gcc 2.8.0
COPY --from=toolchain-psx /cpp     /opt/psx-gcc/cpp
COPY --from=toolchain-psx /cc1     /opt/psx-gcc/cc1
COPY --from=toolchain-psx /gcc     /opt/psx-gcc/gcc
COPY --from=toolchain-psx /cc1plus /opt/psx-gcc/cc1plus
COPY --from=toolchain-psx /g++     /opt/psx-gcc/g++

# PSX gcc 2.6.0
COPY --from=toolchain-2.6.0-psx /cpp     /opt/gcc-2.6.0-psx/cpp
COPY --from=toolchain-2.6.0-psx /cc1     /opt/gcc-2.6.0-psx/cc1
COPY --from=toolchain-2.6.0-psx /gcc     /opt/gcc-2.6.0-psx/gcc
COPY --from=toolchain-2.6.0-psx /cc1plus /opt/gcc-2.6.0-psx/cc1plus
COPY --from=toolchain-2.6.0-psx /g++     /opt/gcc-2.6.0-psx/g++

# Cygnus CDK gcc 2.7.2
COPY --from=toolchain-cdk /cpp     /opt/cdk-gcc/cpp
COPY --from=toolchain-cdk /cc1     /opt/cdk-gcc/cc1
COPY --from=toolchain-cdk /gcc     /opt/cdk-gcc/gcc
COPY --from=toolchain-cdk /cc1plus /opt/cdk-gcc/cc1plus
COPY --from=toolchain-cdk /g++     /opt/cdk-gcc/g++

## PSX GNU gcc 2.7.2
COPY --from=toolchain-gnu /cpp      /opt/psx-gnu-gcc/cpp
COPY --from=toolchain-gnu /cc1      /opt/psx-gnu-gcc/cc1
COPY --from=toolchain-gnu /gcc      /opt/psx-gnu-gcc/gcc
COPY --from=toolchain-gnu /cc1plus  /opt/psx-gnu-gcc/cc1plus
COPY --from=toolchain-gnu /g++      /opt/psx-gnu-gcc/g++
COPY --from=toolchain-gnu /as       /opt/psx-gnu-gcc/as
COPY --from=toolchain-gnu /ld       /opt/psx-gnu-gcc/ld
COPY --from=toolchain-gnu /objdump  /opt/psx-gnu-gcc/objdump

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

# Copy your PSX game files into the container
COPY extracted/ ./
