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

#include <algorithm>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <regetopt/regetopt.h>

#include "nbt/region.hh"
#include "utils/path_hack.hh"
#include "version.hh"

namespace {
    bool dump_coord(const std::string &filename, const std::string &outfmt,
                    int x, int z) {
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
            if (i == outname.end()) break;
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

    bool dump_all(const std::string &filename, const std::string &outfmt) {
        for (int x = 0; x < 32; ++x) {
            for (int z = 0; z < 32; ++z) {
                if (!dump_coord(filename, outfmt, x, z)) {
                    return false;
                }
            }
        }
        return true;
    }

    void print_usage() {
        std::cout << R"(usage: dumpnbt [OPTION]... [--] IN_FILE...

  -o DIR, --out DIR        Set output filename format. %1 is replaced by input filename,
                           %2  and %3 are replaced by chunk x, z coordinate.
  -s (X,Z), --where (X,Z)  Select chunk. If not specified, dump all chunks.
     --help                Print this usage and exit.
     --version             Print version and exit.
)";
    }

    void print_version() {
        std::cout << "dumpnbt (" << PROJECT_NAME << ' ' << VERSION_MAJOR << '.'
                  << VERSION_MINOR << '.' << VERSION_REVISION << ")\n";
    }

    struct re_option long_options[] = {
        {"out", re_required_argument, nullptr, 'o'},
        {"where", re_required_argument, nullptr, 's'},
        {"help", re_no_argument, nullptr, 'h'},
        {"version", re_no_argument, nullptr, 'v'},
        {0, 0, 0, 0}};
} // namespace

int main(int argc, char **argv) {
    const char *outfmt = "%1+%2+%3.nbt";
    std::vector<std::pair<int, int>> coords;

    for (;;) {
        int opt = regetopt(argc, argv, "o:s:", long_options, nullptr);
        if (opt < 0) {
            break;
        }
        switch (opt) {
        case 'o':
            outfmt = re_optarg;
            break;

        case 'O': {
            int x, z;
            if (std::sscanf(re_optarg, "( %d , %d )", &x, &z) != 2) {
                std::cout << "Malformed coordinate. (example: \"(1,2)\")";
                return 1;
            }
            coords.push_back({x, z});
        } break;

        case 'h':
            print_usage();
            return 0;

        case 'v':
            print_version();
            return 0;

        default:
            print_usage();
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
}
