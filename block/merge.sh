#!/usr/bin/env bash
DIR="$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)"
cat "$DIR/block_colors_"* >"$DIR/block_colors.merged"
