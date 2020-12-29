// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <string>

#include "pixel-terrain.hh"
#include "config.h"

namespace {
    struct subcommand {
        std::function<int(int, char **)> command_function;
        std::string description;
    };

    std::map<std::string, subcommand> subcommand_map = {
        {"image", {&pixel_terrain::image_main, "Generate map image."}},
        {"dump-nbt",
         {&pixel_terrain::dump_nbt_main, "Dump zlib-compressed NBT data."}},
        {"nbt-to-xml",
         {&pixel_terrain::nbt_to_xml_main, "Convert NBT data into XML."}},
        {"server",
         {&pixel_terrain::server_main, "Altitude and surface block server."}},
        {"world-info",
         {&pixel_terrain::world_info_main,
          "List saved worlds and their information."}},
    };

    inline bool is_non_option(char *arg) { return arg[0] != '-'; }

    [[noreturn]] void print_help_and_exit(int status) {
        std::fputs(&R"(
Usage: pixel-terrain --version
       pixel-terrain --help
       pixel-terrain <subcommand> [args]...

  --help     Display this help and exit.
  --version  Display version info and exit.

Exit status:
 0 => OK.
 1 => Invalid option.
 2 => Invalid subcommand.

Subcommands:
)"[1],
                   stdout);

        for (auto itr = subcommand_map.begin(), E = subcommand_map.end();
             itr != E; ++itr) {
            std::printf("  %s:  %s\n", itr->first.c_str(),
                        itr->second.description.c_str());
        }

        std::exit(status);
    }

    [[noreturn]] void print_version_and_exit() {
        std::printf("%s %d.%d.%d\n", PRODUCT_NAME, VERSION_MAJOR, VERSION_MINOR,
                    VERSION_REVISION);
        std::fputs(R"(
Copyright (C) 2020  Koki Fukuda
Pixel Terrain is Free and Open Source software distributed under the condition
of MIT License. You should have received license file with this executable, but
if not, you can read all text of the license on
https://github.com/kofuk/pixel-terrain/blob/master/LICENSE.

If you find a bug, please report it on https://github.com/kofuk/pixel-terrain/issues.
)",
                   stdout);

        std::exit(0);
    }
} // namespace

int main(int argc, char **argv) {
    int argoff = 1;
    bool after_subcommand = false;
    for (int i = 1; i < argc; ++i) {
        if (is_non_option(argv[argoff])) {
            after_subcommand = true;
        }

        if (!after_subcommand && !strcmp(argv[argoff], "--help")) {
            print_help_and_exit(0);
        } else if (!strcmp(argv[i], "--version")) {
            /* We process --version option for all subcommands here. */
            print_version_and_exit();
        } else if (!after_subcommand) {
            std::printf("Invalid option: %s\n", argv[argoff]);
            print_help_and_exit(1);
        }

        if (!after_subcommand) ++argoff;
    }

    if (argc <= argoff) {
        print_help_and_exit(1);
    }

    auto subcommand = subcommand_map.find(argv[argoff]);
    if (subcommand == subcommand_map.end()) {
        print_help_and_exit(2);
    }

    return (subcommand->second.command_function)(argc - argoff, argv + argoff);
}
