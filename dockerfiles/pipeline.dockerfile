# This dockerfile is for running the github pipeline for object diffing.
# It expects an archive to exist at the local path ROM/ containing a .7z file with the game files. 
# The .7z should contain a .bin and .cue pair, which will be converted to an ISO and unpacked into /rom.
# First build the compiler images and the shared toolchain from the repository root:
# docker build -t lom-toolchain -f dockerfiles/toolchain.dockerfile .
# Then run the following from the repo root (replace the version string):
# docker build -t ghcr.io/celophi/slus-01013:v5.0 -f dockerfiles/pipeline.dockerfile dockerfiles

FROM lom-toolchain:latest

# Only the pipeline image needs to unpack a disc image.
RUN apt-get update && apt-get install -y --no-install-recommends p7zip-full bchunk \
    && rm -rf /var/lib/apt/lists/*

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
