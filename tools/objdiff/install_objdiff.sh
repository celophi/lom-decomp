#!/bin/sh
# Usage: sh tools/objdiff/install_objdiff.sh /path/to/objdiff-cli
set -eu

version=3.6.0
sha256=11ec450e744cabcc825e0b5afaedb9ec7ae61381a5bcfbb38abf034e7edb6803
destination=${1:?Usage: install_objdiff.sh /path/to/objdiff-cli}

# This release asset supports x86_64 Linux, as do the historical toolchains.
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != x86_64 ]; then
    echo 'This installer requires x86_64 Linux.' >&2
    exit 1
fi

mkdir -p "$(dirname "$destination")"
download=$(mktemp "$destination.download.XXXXXX")
trap 'rm -f "$download"' EXIT
trap 'exit 1' HUP INT TERM

curl --fail --location --retry 3 \
    "https://github.com/encounter/objdiff/releases/download/v$version/objdiff-cli-linux-x86_64" \
    -o "$download"
printf '%s  %s\n' "$sha256" "$download" | sha256sum -c -
chmod 755 "$download"
mv -f "$download" "$destination"
