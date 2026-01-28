# Stage 1: grab compiler artifacts
FROM old-gcc/gcc-2.7.2-psx AS toolchain

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
    && rm -rf /var/lib/apt/lists/*

# Copy GCC pieces from the toolchain stage
COPY --from=toolchain /cpp      /opt/psx-gcc/cpp
COPY --from=toolchain /cc1      /opt/psx-gcc/cc1
COPY --from=toolchain /gcc      /opt/psx-gcc/gcc
COPY --from=toolchain /cc1plus  /opt/psx-gcc/cc1plus
COPY --from=toolchain /g++      /opt/psx-gcc/g++

ENV PATH="/opt/psx-gcc:$PATH"

# Copy requirements.txt to a build-only location
COPY requirements.txt /build-lom/requirements.txt

# Also copy any referenced editable packages for pip install
COPY tools /build-lom/tools

# Install Python dependencies
WORKDIR /build-lom
RUN pip install -r /build-lom/requirements.txt

# Setup the mount point for the project
RUN mkdir /lom
WORKDIR /lom