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

cd "$(dirname "${BASH_SOURCE:-$0}")"

. ./dep

if ! command -v curl &>/dev/null || ! command -v tar &>/dev/null || \
        ! command -v  sha256sum &>/dev/null || ! command -v patch &>/dev/null; then
    echo 'Fatal: install following command and execute again:'
    echo '	curl  tar  sha256sum  patch'
    exit 1
fi

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
