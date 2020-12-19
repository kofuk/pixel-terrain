#!/usr/bin/env bash

# SPDX-License-Identifier: MIT

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
