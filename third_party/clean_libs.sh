#!/usr/bin/env bash

# SPDX-License-Identifier: MIT

cd "$(dirname "${BASH_SOURCE:-$0}")"

# remove all tarballs
rm -f *.tar.gz

# remove extracted dirs
rm -rf libpng regetopt zlib
