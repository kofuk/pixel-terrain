#!/usr/bin/env bash
set -eu

if [ $# -lt 1 ]; then
    echo "usage: $0 IN_FILE"
    exit 1
fi

DIR="$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)"

rm "$DIR/block_colors_"* &>/dev/null || true

cat "$1" | while read line; do
    prefix="$(echo "$line" | sed -E 's/^colors\[\"([a-z_]).+$/\1/')"
    echo "$line" >> "$DIR/block_colors_$prefix"
done

for f in "$DIR/block_colors_"*; do
    cat "$f" | sort -uk1 | sponge "$f"

    echo '#include "'"$(basename "$f")"'"'
done > 'require_all.hh'
