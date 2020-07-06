#include <cstdlib>
#include <iostream>

#include <optlib/optlib.h>

#include "server/server.hh"
#include "version.hh"

namespace {
    void print_usage(optlib_parser *opt) {
        std::cout << "usage: blockd [OPTIONS...]\n";

        std::cout << "Possible options are:" << std::endl;
        optlib_print_help(opt, stderr);

        std::cout << R"(
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
in Python.  Visit  https://github.com/kofuk/minecraft-image-gemerator  for more
information and the source code.
)";
    }
} // namespace

int main(int argc, char **argv) {
    bool daemon_mode = false;

    optlib_parser *parser = optlib_parser_new(argc, argv);
    optlib_parser_add_option(parser, "daemon", 'd', false,
                             "Run block server as daemon.");
    optlib_parser_add_option(parser, "overworld", 'o', true,
                             "Specify data directory for the overworld.");
    optlib_parser_add_option(parser, "nether", 'n', true,
                             "Specify data directory for the nether.");
    optlib_parser_add_option(parser, "end", 'e', true,
                             "Specify data directory for the end.");
    optlib_parser_add_option(parser, "help", 'h', false,
                             "Print this message and exit.");
    optlib_parser_add_option(parser, "version", 'v', false,
                             "Print version and exit.");

    for (;;) {
        optlib_option *opt = optlib_next(parser);

        if (parser->finished) {
            break;
        }
        if (!opt) {
            print_usage(parser);
            ::exit(1);
        }

        switch (opt->short_opt) {
        case 'd':
            daemon_mode = true;
            break;

        case 'h':
            print_usage(parser);
            ::optlib_parser_free(parser);
            ::exit(0);

        case 'o':
            pixel_terrain::server::overworld_dir = opt->argval;
            break;

        case 'n':
            pixel_terrain::server::nether_dir = opt->argval;
            break;

        case 'e':
            pixel_terrain::server::end_dir = opt->argval;
            break;

        case 'v':
            print_version();
            ::optlib_parser_free(parser);
            ::exit(0);
        }
    }

    if (argc - parser->optind == 0) {
        ::optlib_parser_free(parser);
        pixel_terrain::server::launch_server(daemon_mode);
    } else {
        print_usage(parser);
        ::optlib_parser_free(parser);
        ::exit(1);
    }

    return 0;
}
