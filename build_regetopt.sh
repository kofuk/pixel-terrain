#!/usr/bin/env bash

_DIR="$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)"

cd "$_DIR"

. third_party/dep

if ! command -v curl &>/dev/null || ! command -v tar &>/dev/null || \
        ! command -v  sha256sum &>/dev/null || ! command -v patch &>/dev/null || \
        !command -v ninja &>/dev/null; then
    echo 'Fatal: install following command and execute again:'
    echo '	curl  tar  sha256sum  patch'
    exit 1
fi

curl -Lo "${extract_dests[2]}.tar.gz" "${download_urls[2]}" || { echo 'Fatal: donwload failure'; exit 1; }
tar -xf "${extract_dests[2]}.tar.gz" -C '/tmp' && rm -f "${extract_dests[2]}.tar.gz"

mkdir -p build/regetopt && cd $_ || { echo 'echo: Unable to make build directory'; exit 1; }

cmake "$@" -GNinja -DCMAKE_INSTALL_PREFIX="$_DIR/build/third_party" "/tmp/${extracted_dir_names[2]}" && ninja install || \
        { echo 'Fatal: build failure'; exit 1; }

echo 'Library has been built successfully!'
echo 'Configure main project with following option:'
echo "	-DREGETOPT_CONFIG_CMAKE=$_DIR/build/third_party/lib/regetopt/regetopt-config.cmake"
