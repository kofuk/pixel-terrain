#!/usr/bin/env bash

if [ "$#" -lt 1 ]; then
    echo 'output path required.'
    exit 1
fi

# DATA STRUCTURE
#
# |-------------------|-------------------------|
# | block_id (string) | block id (byte by byte) |
# | .                 |                         |
# | .                 |                         |
# | .                 |                         |
# | 0                 | sentinel value          |
# | color[0]          |                         |
# | color[1]          |                         |
# | color[2]          |                         |
# | color[3]          |                         |
# |-------------------+-------------------------|
# |                   |                         |
# | .                 |                         |
# | .                 |                         |
# | .                 |                         |
# |                   |                         |
# |-------------------+-------------------------|
# | 0                 | sentinel value          |
# |-------------------+-------------------------|
#
# order of color 0--3 is byte-order-dependent.
# this script puts the bytes to construct 32 bits of int
# represents RRGGBBAA.
# for now, I support only little endian because I've no
# motivation to support big endian machines.

# TODO: support big endian and middle endian archtecture ??

(
    echo '#ifdef MCMAP_BLOCK_COLORS_DATA_HH'
    echo '#error "block colors data header is already included."'
    echo '#else'
    echo '#define MCMAP_BLOCK_COLORS_DATA_HH'

    echo 'static unsigned char block_colors_data[] = { '

    DIR="$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)/minecraft"
    for F in "$DIR/block_colors_"*".tsv"; do
        tail -n +2 -- "$F" | awk -F$'\t' '{for(i=1;i<=length($1);++i)printf "'"'%s', "'",substr($1,i,1);printf "0, ";for(i=5;i>1;--i)printf "0x%s, ",$i;printf "\n"}'
    done

    echo '0 };'

    echo '#endif'
) >"$1"
