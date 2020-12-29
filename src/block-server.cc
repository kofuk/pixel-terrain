// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <regetopt.h>

#include "config.h"
#include "pixel-terrain.hh"
#include "server/server.hh"

namespace {
    void print_usage() {
        std::fputs(&R"(
usage: pixel-terrain server [OPTIONS...]
  -d, --daemon             Run block server as daemon.
  -o DIR, --overworld DIR  Read overworld data from DIR.
  -n DIR, --nether DIR     Read nether data from DIR.
  -e DIR, --end DIR        Read end data from DIR.
          --help           Print this usage and exit.

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
)"[1],
                   stdout);
    }

    struct re_option long_options[] = {
        {"daemon", re_no_argument, nullptr, 'd'},
        {"overworld", re_required_argument, nullptr, 'o'},
        {"nether", re_required_argument, nullptr, 'n'},
        {"end", re_required_argument, nullptr, 'e'},
        {"help", re_no_argument, nullptr, 'h'},
        {0, 0, 0, 0}};
} // namespace

namespace pixel_terrain {
    int server_main(int argc, char **argv) {
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
} // namespace pixel_terrain
