#!/usr/bin/env bash

if [ $# -lt 1 ]; then
    echo 'Region file directory must be provided' >&2

    exit 1
fi

nether=
if [ $# -ge 2 ] && [ "$2" = '--nether' ]; then
    nether='-n'
fi

n_files="$(ls $1/*.mca | wc -l)"

if [ "$n_files" -eq 0 ]; then
    echo 'The directory does not contain any region file' >&2

    exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)"

echo 0 > gen_progress.txt

((n_processed=0))

for f in $1/*.mca; do
    region="$(echo "$f" | sed -E 's/.*r\.([0-9-]+)\.([0-9-]+)\.mca$/\1 \2/')"

    "$script_dir/build/src/mcmap" $nether -- "$1" $region

    ((++n_processed))
    echo $((n_processed*100/n_files)) > gen_progress.txt
done

rm -f gen_progress.txt
