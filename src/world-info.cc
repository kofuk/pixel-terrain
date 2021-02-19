// SPDX-License-Identifier: MIT

#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <regetopt.h>

#include "pixel-terrain.hh"
#include "utils/array.hh"
#include "worlds/worlds.hh"

namespace pixel_terrain {
    namespace {
        [[noreturn]] void print_help_and_exit(int status) {
            std::fputs(&R"(
Usage: pixel-terrain world-info [option]...

  -F FIELDS, --fields=FIELDS  Comma-separated list of field to show. [name,path]
  -q, --quiet                 Don't print index line.
  -h, --help                  Print this help and exit.

Exit status:
 0 => OK.
 1 => Invalid options.
 2 => Tried to list worlds, but some errors occurred.
)"[1],
                       stdout);

            std::exit(status);
        }

        auto long_options = pixel_terrain::make_array<::re_option>(
            ::re_option{"help", re_no_argument, nullptr, 'h'},
            ::re_option{nullptr, 0, nullptr, 0});
    } // namespace

    auto world_info_main(int argc, char **argv) -> int {
        for (;;) {
            int c = regetopt(argc, argv, "qh", long_options.data(), nullptr);
            if (c < 0) {
                break;
            }

            switch (c) {
            case 'h':
                print_help_and_exit(0);
                break;

            default:
                print_help_and_exit(1);
                break;
            }
        }

        int status = 0;
        for (world_iterator worlds, E = worlds.end(); worlds != E; ++worlds) {
            try {
                world w = *worlds;

                std::cout << w.get_save_file_path() << '\n';
                std::cout << "  Name:                " << w.get_name() << '\n';
                std::cout << "  Game version:        " << w.get_game_version()
                          << '\n';
                std::cout << "  Hardcore:            "
                          << (w.is_hardcore() ? "Yes" : "No") << '\n';
                std::cout << "  Dimensions:         ";
                for (auto const &dim : w.get_dimensions()) {
                    std::cout << ' ' << dim;
                }
                std::cout << '\n';
                std::cout << "  Enabled data packs: ";
                for (auto const &dp : w.get_enabled_datapacks()) {
                    std::cout << ' ' << dp;
                }
                std::cout << '\n';
                std::cout << "  Disabled data packs:";
                for (auto const &dp : w.get_disabled_datapacks()) {
                    std::cout << ' ' << dp;
                }
                std::cout << '\n';
            } catch (...) {
                std::cout << "<error>\n";
                status = 2;
            }
        }

        return status;
    }
} // namespace pixel_terrain
