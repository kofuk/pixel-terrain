/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdlib>
#include <iostream>

#include <regetopt/regetopt.h>

#include "server/server.hh"
#include "version.hh"

namespace {
    void print_usage() {
        std::cout << R"(usage: blockd [OPTIONS...]
  -d, --daemon             Run block server as daemon.
  -o DIR, --overworld DIR  Read overworld data from DIR.
  -n DIR, --nether DIR     Read nether data from DIR.
  -e DIR, --end DIR        Read end data from DIR.
          --help           Print this usage and exit.
          --version        Print version and exit.

Help for block info server's protocol and config

PROTOCOL:
 example request and response:
 `>' means request and `<' means response
  >GET MMP/1.0
  >Dimension: overworld
  >Coord-X: 1
  >Coord-Z: 1
  >
  <MMP/1.0 200
  <
  <{"altitude": 63, "block": "minecraft:dirt"}


 Response Codes:
  200  OK. The request is handled properly and altitude returned.
  400  Bad Request. Probably request parse error
  404  Out of Range. No chunk existing on specified corrdinate.
  500  Internal Server Error. Serverside error
)";
    }

    void print_version() {
        std::cout << "blockd (" PROJECT_NAME " " VERSION_MAJOR "." VERSION_MINOR
                     "." VERSION_REVISION ")\n";
        std::cout << R"(
Copyright (C) 2020  Koki Fukuda.
This program includes C++ re-implementation of anvil-parser, originally written
in Python. Visit https://github.com/kofuk/pixel-terrain for more information
and the source code.
)";
    }

    struct re_option long_options[] = {
        {"daemon", re_no_argument, nullptr, 'd'},
        {"overworld", re_required_argument, nullptr, 'o'},
        {"nether", re_required_argument, nullptr, 'n'},
        {"end", re_required_argument, nullptr, 'e'},
        {"help", re_no_argument, nullptr, 'h'},
        {"version", re_no_argument, nullptr, 'v'},
        {0, 0, 0, 0}};
} // namespace

int main(int argc, char **argv) {
    bool daemon_mode = false;

    for (;;) {
        int opt = regetopt(argc, argv, "do:n:e:", long_options, nullptr);
        if (opt < 0) {
            break;
        }

        switch (opt) {
        case 'd':
            daemon_mode = true;
            break;

        case 'h':
            print_usage();
            ::exit(0);

        case 'o':
            pixel_terrain::server::overworld_dir = re_optarg;
            break;

        case 'n':
            pixel_terrain::server::nether_dir = re_optarg;
            break;

        case 'e':
            pixel_terrain::server::end_dir = re_optarg;
            break;

        case 'v':
            print_version();
            ::exit(0);

        default:
            return 1;
        }
    }

    if (argc - re_optind == 0) {
        pixel_terrain::server::launch_server(daemon_mode);
    } else {
        print_usage();
        ::exit(1);
    }

    return 0;
}
