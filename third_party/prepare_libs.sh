#!/usr/bin/env bash

# Copyright (c) 2020 Koki Fukuda
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

download_urls=(
    'http://zlib.net/zlib-1.2.11.tar.gz'
    'http://prdownloads.sourceforge.net/libpng/libpng-1.6.37.tar.gz?download'
    'https://github.com/kofuk/regetopt/archive/v0.0.2.tar.gz'
)
archive_checksums=(
    'c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1'
    'daeb2620d829575513e35fecc83f0d3791a620b9b93d800b763542ece9390fb4'
    'SKIP'
)
extracted_dir_names=(
    'zlib-1.2.11'
    'libpng-1.6.37'
    'regetopt-0.0.2'
)
extract_dests=(
    'zlib'
    'libpng'
    'regetopt'
)

if ! command -v curl &>/dev/null || ! command -v tar &>/dev/null || \
        ! command -v  sha256sum &>/dev/null || ! command -v patch &>/dev/null; then
    echo 'Fatal: install following command and execute again:'
    echo '	curl  tar  sha256sum  patch'
    exit 1
fi

cd "$(dirname "${BASH_SOURCE:-$0}")"

for ((i=0; i<3; ++i)); do
    curl -Lo "${extract_dests[i]}.tar.gz" "${download_urls[i]}"
    if [ $? -ne 0 ]; then
        echo "Fatal: unable to download ${extract_dests[i]}"
        exit 1
    fi

    if [ "${archive_checksums[i]}" != 'SKIP' ]; then
        if ! sha256sum -c <(echo "${archive_checksums[i]} ${extract_dests[i]}.tar.gz") \
             <"${extract_dests[i]}.tar.gz" &>/dev/null; then
            echo "Fatal: checksum verification error"
            exit 1
        fi
    fi

    tar -xf "${extract_dests[i]}.tar.gz" && mv "${extracted_dir_names[i]}" "${extract_dests[i]}" && \
        rm -f "${extract_dests[i]}.tar.gz"
done

(
    cd zlib
    patch -p1 <../zlib-export-targets.patch
)

(
    cd libpng
    patch <../png-export-include-dir.patch
)
