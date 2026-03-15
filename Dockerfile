# ── Stage 1: PSX gcc 2.8.0 ──────────────────────────────────────────────────
FROM old-gcc/gcc-2.8.0-psx AS toolchain-psx

# ── Stage 2: Cygnus CDK gcc 2.7.2 ───────────────────────────────────────────
FROM old-gcc/gcc-2.7.2-cdk AS toolchain-cdk

# ── Stage 3: final runtime image ────────────────────────────────────────────
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
    && rm -rf /var/lib/apt/lists/*

# PSX gcc 2.8.0
COPY --from=toolchain-psx /cpp     /opt/psx-gcc/cpp
COPY --from=toolchain-psx /cc1     /opt/psx-gcc/cc1
COPY --from=toolchain-psx /gcc     /opt/psx-gcc/gcc
COPY --from=toolchain-psx /cc1plus /opt/psx-gcc/cc1plus
COPY --from=toolchain-psx /g++     /opt/psx-gcc/g++

# Cygnus CDK gcc 2.7.2
COPY --from=toolchain-cdk /cpp     /opt/cdk-gcc/cpp
COPY --from=toolchain-cdk /cc1     /opt/cdk-gcc/cc1
COPY --from=toolchain-cdk /gcc     /opt/cdk-gcc/gcc
COPY --from=toolchain-cdk /cc1plus /opt/cdk-gcc/cc1plus
COPY --from=toolchain-cdk /g++     /opt/cdk-gcc/g++

ENV PATH="/opt/psx-gcc:$PATH"

COPY requirements.txt /build-lom/requirements.txt
COPY tools            /build-lom/tools

WORKDIR /build-lom
RUN pip install -r /build-lom/requirements.txt

RUN mkdir /lom
WORKDIR /lom