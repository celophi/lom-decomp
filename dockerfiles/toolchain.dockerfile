# Shared historical compilers and system tools for both images.
# Build the four old-gcc images first, then run from the repository root:
#   docker build -t lom-toolchain -f dockerfiles/toolchain.dockerfile .

# Stage 1: grab compiler artifacts
FROM old-gcc/gcc-2.8.0-psx AS psx-gcc-2.8.0
FROM old-gcc/gcc-2.6.0-psx AS psx-gcc-2.6.0
FROM old-gcc/gcc-2.7.2-cdk AS psx-gcc-2.7.2-cdk
FROM old-gcc/gcc-2.7.2-psx-gnu AS psx-gcc-2.7.2-gnuas

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
