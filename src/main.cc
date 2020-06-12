/*
 * Entry point for pixel-terrain.
 * Parse option and all call specific function.
 */

#include <algorithm>
#include <array>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include <optlib/optlib.h>

#include "commands/generate/generate_main.hh"
#include "commands/server/server.hh"

using namespace std;

namespace pixel_terrain {
    namespace {
        void print_usage() {
            cout << "Usage: mcmap generate [OPTION]... SRC_DIR" << endl;
            cout << "       mcmap server [OPTION]..." << endl;
            cout << "       mcmap --version" << endl;
            cout << "For help for each mode, execute these mode with `--help' "
                    "option."
                 << endl;
        }

        void print_version() {
            cout << "mcmap 3.0" << endl;
            cout << "Copyright (C) 2020, Koki Fukuda." << endl;
            cout << "This program includes C++ re-implementation of" << endl
                 << "anvil-parser and nbt, originally written in Python."
                 << endl
                 << "Visit https://github.com/kofuk/minecraft-image-gemerator"
                 << endl
                 << "for more information and the source code." << endl;
        }

        void handle_commands(int argc, char **argv) {
            if (!strcmp(argv[0], "generate")) {
                commands::generate::main(argc, argv);
            } else if (!strcmp(argv[0], "server")) {
                commands::server::main(argc, argv);
            } else {
                cout << "unrecognized option: " << argv[0] << endl;
                print_usage();
                exit(1);
            }
        }
    } // namespace
} // namespace pixel_terrain

int main(int argc, char **argv) {
    if (argc < 2) {
        pixel_terrain::print_usage();
        exit(1);
    }

    if (!strcmp(argv[1], "--version")) {
        pixel_terrain::print_version();
        exit(0);
    } else if (argv[1][0] == '-' || argv[1][0] == '/') {
        pixel_terrain::print_usage();
        exit(0);
    } else {
        pixel_terrain::handle_commands(--argc, ++argv);
    }

    return 0;
}
