# dev.dockerfile: primary development container for the Legend of Mana decomp.
#
# This is the image you build and work inside day-to-day: it bundles every PSX
# compiler toolchain and the Python tooling needed to split, build, and diff
# the ROM. Unlike pipeline.dockerfile (the CI image), it does NOT
# embed any ROM data; you mount the repo into the running container instead.
#
# Build from the repo root (context must be the repo root for requirements.txt
# and tools/ to be copied in):
#   docker build -t lom-toolchain -f dockerfiles/toolchain.dockerfile .
#   docker build -t lom-dev -f dockerfiles/dev.dockerfile .
#
# See the README "Build the Development Environment" section for full usage.

FROM lom-toolchain:latest

# Install the pinned objdiff release independently of the checkout.
COPY tools/objdiff/install_objdiff.sh /tmp/install_objdiff.sh
RUN sh /tmp/install_objdiff.sh /usr/local/bin/objdiff-cli \
    && objdiff-cli --version \
    && rm /tmp/install_objdiff.sh

COPY requirements.txt /build-lom/requirements.txt
# Only these editable Python packages need to live in the image. Project tools
# run from the repository mounted at /lom.
COPY tools/external/splat /build-lom/tools/external/splat
COPY tools/external/m2c   /build-lom/tools/external/m2c

WORKDIR /build-lom
RUN pip install -r /build-lom/requirements.txt

RUN mkdir /lom
WORKDIR /lom
