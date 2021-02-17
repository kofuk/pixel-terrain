// SPDX-License-Identifier: MIT

#include <array>
#include <bits/ranges_algo.h>
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

        auto split_field_names(char const *arg) -> std::vector<std::string> {
            std::vector<std::string> fields;
            std::string current;

            while (*arg != '\0') {
                if (*arg == ',') {
                    if (!current.empty()) {
                        fields.push_back(current);
                        current.clear();
                    }
                } else {
                    current.push_back(*arg);
                }

                ++arg;
            }

            if (!current.empty()) {
                fields.push_back(current);
            }

            return fields;
        }

        std::array<std::string, 2> valid_fields = {"name", "path"};

        inline auto is_valid_field_name(std::string const &field) -> bool {
            return std::ranges::any_of(
                valid_fields,
                [&field](std::string const &f) { return f == field; });
        }

        void warn_and_drop_unknown_fields(std::vector<std::string> *fields) {
            std::vector<std::string> dropped;

            for (std::size_t i = 0, len = fields->size(); i < len; ++i) {
                std::size_t cur = len - 1 - i;
                std::string field = (*fields)[cur];
                if (!is_valid_field_name(field)) {
                    dropped.push_back(std::move(field));
                    fields->erase(fields->cbegin() + cur);
                }
            }

            for (auto itr = dropped.crbegin(), E = dropped.crend(); itr != E;
                 ++itr) {
                std::cerr << "Warning: Dropped invalid field: " << *itr << '\n';
            }
        }

        auto long_options = pixel_terrain::make_array<::re_option>(
            ::re_option{"fields", re_required_argument, nullptr, 'F'},
            ::re_option{"quiet", re_no_argument, nullptr, 'q'},
            ::re_option{"help", re_no_argument, nullptr, 'h'},
            ::re_option{nullptr, 0, nullptr, 0});
    } // namespace

    auto world_info_main(int argc, char **argv) -> int {
        char const *only_fields = "name,path";
        bool quiet = false;
        for (;;) {
            int c = regetopt(argc, argv, "F:qh", long_options.data(), nullptr);
            if (c < 0) {
                break;
            }

            switch (c) {
            case 'F':
                only_fields = re_optarg;
                break;

            case 'q':
                quiet = true;
                break;

            case 'h':
                print_help_and_exit(0);
                break;

            default:
                print_help_and_exit(1);
                break;
            }
        }

        std::vector<std::string> fields = split_field_names(only_fields);
        warn_and_drop_unknown_fields(&fields);

        if (!quiet) {
            for (std::string const &field_name : fields) {
                std::cout << field_name << '\t';
            }
            std::cout << '\n';
        }

        int status = 0;
        for (world_iterator worlds, E = worlds.end(); worlds != E; ++worlds) {
            try {
                world w = *worlds;

                for (std::string const &field_name : fields) {
                    if (field_name == "name") {
                        std::cout << w.get_name() << '\t';
                    } else if (field_name == "path") {
                        std::cout << w.get_save_file_path() << '\t';
                    }
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
