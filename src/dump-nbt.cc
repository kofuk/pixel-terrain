// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <regetopt.h>

#include "config.h"
#include "nbt/region.hh"
#include "pixel-terrain.hh"
#include "utils/array.hh"
#include "utils/path_hack.hh"

namespace {
    auto dump_coord(const std::string &filename, const std::string &outfmt,
                    int x, int z) -> bool {
        std::filesystem::path p(filename);
        std::string in_filename = p.filename().string();
        if (in_filename.size() > 4 &&
            in_filename.find(".mca") == in_filename.size() - 4) {
            in_filename.erase(in_filename.end() - 4, in_filename.end());
        }

        std::string outname = outfmt;

        auto i = outname.begin();
        for (;;) {
            i = std::find(i, outname.end(), '%');
            if (i == outname.end()) {
                break;
            }
            switch (*(i + 1)) {
            case '1':
                outname.erase(i, i + 2);
                outname.insert(i, in_filename.begin(), in_filename.end());
                i += in_filename.size();
                break;

            case '2': {
                std::string x_str = std::to_string(x);
                outname.erase(i, i + 2);
                outname.insert(i, x_str.begin(), x_str.end());
                i += x_str.size();
                break;
            }

            case '3': {
                std::string z_str = std::to_string(z);
                outname.erase(i, i + 2);
                outname.insert(i, z_str.begin(), z_str.end());
                i += z_str.size();
                break;
            }
            }
        }

        try {
            std::filesystem::path infile(filename);
            pixel_terrain::anvil::region r =
                pixel_terrain::anvil::region(infile);
            auto [data, len] = r.chunk_data(x, z);

            std::filesystem::path outfile(outname);
            std::ofstream out(outfile, std::ios::binary);
            if (!out) {
                std::cerr << outname << ": unable to open output file\n";
                return false;
            }

            out.write(reinterpret_cast<char *>(data.get()), len);
        } catch (...) {
            std::cerr << outname << ": error dumping nbt\n";
            return false;
        }
        return true;
    }

    auto dump_all(const std::string &filename, const std::string &outfmt)
        -> bool {
        constexpr int chunk_size = 32;
        for (int x = 0; x < chunk_size; ++x) {
            for (int z = 0; z < chunk_size; ++z) {
                if (!dump_coord(filename, outfmt, x, z)) {
                    return false;
                }
            }
        }
        return true;
    }

    void print_usage() {
        std::cout << &R"(
Usage: pixel-terrain dump-nbt [option]... [--] <in file>...

  -o DIR, --out=DIR        Set output filename format. %1 is replaced by input filename,
                           %2  and %3 are replaced by chunk x, z coordinate.
  -s (X,Z), --where=(X,Z)  Select chunk. If not specified, dump all chunks.
     --help                Print this usage and exit.
)"[1];
    }

    auto long_options = pixel_terrain::make_array<::re_option>(
        ::re_option{"out", re_required_argument, nullptr, 'o'},
        ::re_option{"where", re_required_argument, nullptr, 's'},
        ::re_option{"help", re_no_argument, nullptr, 'h'},
        ::re_option{nullptr, 0, nullptr, 0});
} // namespace

namespace pixel_terrain {
    auto dump_nbt_main(int argc, char **argv) -> int {
        const char *outfmt = "%1+%2+%3.nbt";
        std::vector<std::pair<int, int>> coords;

        for (;;) {
            int opt =
                regetopt(argc, argv, "o:s:", long_options.data(), nullptr);
            if (opt < 0) {
                break;
            }
            switch (opt) {
            case 'o':
                outfmt = re_optarg;
                break;

            case 'O': {
                int x;
                int z;
                if (std::sscanf(re_optarg, "( %d , %d )", &x, &z) != 2) {
                    std::cout << "Malformed coordinate. (example: \"(1,2)\")";
                    return 1;
                }
                coords.emplace_back(x, z);
            } break;

            case 'h':
                print_usage();
                return 0;

            default:
                return 1;
            }
        }

        for (int i = re_optind; i < argc; ++i) {
            if (coords.empty()) {
                dump_all(argv[i], outfmt);
            } else {
                for (const std::pair<int, int> &coord : coords) {
                    dump_coord(argv[i], outfmt, coord.first, coord.second);
                }
            }
        }

        return 0;
    }
} // namespace pixel_terrain
