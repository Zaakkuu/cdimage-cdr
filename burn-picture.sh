#!/bin/sh
# Make a track and burn it onto a Verbatim CD-R Extra Protection (CMC Magnetics 97m26s66f).
# Tested with an LG GP60NS50 drive. Set DEV=/dev/srN for another drive (default /dev/sr0).
set -e
[ -f "$1" ] || { echo "usage: $0 <picture>"; exit 1; }
dir=$(dirname "$0")
cdrdao disk-info --device ${DEV:-/dev/sr0} 2>&1 | grep -q "CD-R empty *: yes" || { echo "Put a blank disc in first."; exit 1; }
"$dir/build-cli/cdimage-cli" "$1" /tmp/track0 23347.02 1.3899902 25.2766 2>&1 | grep -v "out of range"
wodim -v dev=${DEV:-/dev/sr0} speed=10 -dao -audio /tmp/track0
