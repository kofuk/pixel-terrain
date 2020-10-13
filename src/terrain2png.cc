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

#include <iostream>
#include <thread>

#include <regetopt/regetopt.h>

#include "image/blocks.hh"
#include "image/generator.hh"
#include "utils/logger/logger.hh"
#include "utils/logger/pretty_printer.hh"
#include "utils/path_hack.hh"
#include "version.hh"

namespace pixel_terrain::image {
    namespace {
        void write_range_file(int start_x, int start_z, int end_x, int end_z) {
            std::filesystem::path out_path(option_out_dir);
            out_path /= PATH_STR_LITERAL("chunk_range.json");

            std::ofstream out(out_path);
            if (!out) return;

            out << "[" << start_x << ", " << start_z << ", " << end_x << ", "
                << end_z << "]\n";
        }

        void generate_all(std::string src_dir) {
            if (!option_cache_dir.empty()) {
                try {
                    std::filesystem::create_directories(option_cache_dir);
                } catch (std::filesystem::filesystem_error const &e) {
                    using namespace std::literals::string_literals;
                    logger::e("cannot create cache directory: "s + e.what());

                    exit(1);
                }
            }

            std::filesystem::directory_iterator dirents(src_dir);
            int nfiles = std::distance(begin(dirents), end(dirents));
            pretty_printer::set_total(nfiles);

            int min_x = std::numeric_limits<int>::max();
            int min_z = std::numeric_limits<int>::max();
            int max_x = std::numeric_limits<int>::min();
            int max_z = std::numeric_limits<int>::min();

            for (std::filesystem::directory_entry const &path :
                 std::filesystem::directory_iterator(src_dir)) {
                if (path.is_directory()) continue;

                std::string name = path.path().filename().string();

                if (name[0] != 'r' || name[1] != '.') continue;
                std::size_t i = 2;
                while (i < name.size() &&
                       (name[i] == '-' || ('0' <= name[i] && name[i] <= '9')))
                    ++i;

                if (name[i] == '.' && i + 1 >= name.size()) continue;
                ++i;

                std::size_t start_z = i;
                while (i < name.size() &&
                       (name[i] == '-' || ('0' <= name[i] && name[i] <= '9')))
                    ++i;

                int x = stoi(name.substr(2, i - 2));
                int z = stoi(name.substr(start_z, i - start_z));

                if (option_generate_range) {
                    if (x < min_x) min_x = x;
                    if (x > max_x) max_x = x;
                    if (z < min_z) min_z = z;
                    if (z > max_z) max_z = z;
                }

                anvil::region *r;
                try {
                    if (option_cache_dir.empty()) {
                        r = new anvil::region(path.path());
                    } else {
                        r = new anvil::region(path.path(), option_cache_dir);
                    }
                } catch (std::exception const &e) {
                    logger::e("failed to read region: " + path.path().string());
                    logger::e(e.what());

                    continue;
                }

                std::shared_ptr<region_container> rc(
                    new region_container(r, x, z));
                generate_region(std::shared_ptr<queued_item>(new queued_item(rc)));

                pretty_printer::increment_progress_bar();
            }

            pretty_printer::finish_progress_bar();

            if (option_generate_range) {
                /* max_* is exclusive */
                ++max_x;
                ++max_z;
                min_x *= 2;
                min_z *= 2;
                max_x *= 2;
                max_z *= 2;
                write_range_file(min_x, min_z, max_x, max_z);
            }

            if (option_show_stat) {
                logger::show_stat();
            }
        }
    } // namespace
} // namespace pixel_terrain::image

namespace {
    void print_usage() {
        std::cout << R"(usage: terrain2png [OPTION]... [--] DIR
Load save data in DIR, and generate image.

  -j N, --jobs=N           Deleted option.
  -c DIR, --cache-dir DIR  Use DIR as cache direcotry.
  -n, --nether             Use image generator optimized to nether.
  -o DIR, --out DIR        Save generated images to DIR.
  -r, --gen-range          Generate JSON file indicates X and Z range block exists.
  -s, --statistics         Show statistics about generation.
  -V, --verbose            Enable verbose log output.
      --help               Print this usage and exit.
      --version            Print version and exit.
)";
    }

    void print_version() {
        std::cout << "terrain2png (" PROJECT_NAME " " VERSION_MAJOR
                     "." VERSION_MINOR "." VERSION_REVISION ")\n";
        std::cout << R"(
Copyright (C) 2020  Koki Fukuda.
This program includes C++ re-implementation of anvil-parser, originally written
in Python.  Visit  https://github.com/kofuk/minecraft-image-gemerator  for more
information and the source code.
)";
    }

    struct re_option long_options[] = {
        {"jobs", re_required_argument, nullptr, 'j'},
        {"cache-dir", re_required_argument, nullptr, 'c'},
        {"nether", re_no_argument, nullptr, 'n'},
        {"out", re_required_argument, nullptr, 'o'},
        {"gen-range", re_no_argument, nullptr, 'r'},
        {"statistics", re_no_argument, nullptr, 's'},
        {"verbose", re_no_argument, nullptr, 'V'},
        {"help", re_no_argument, nullptr, 'h'},
        {"version", re_no_argument, nullptr, 'v'},
        {0, 0, 0, 0}};
} // namespace

int main(int argc, char **argv) {
    pixel_terrain::image::option_out_dir = PATH_STR_LITERAL(".");

    for (;;) {
        int opt = regetopt(argc, argv, "j:c:no:rsV", long_options, nullptr);
        if (opt < 0) {
            break;
        }

        switch (opt) {
        case 'V':
            pixel_terrain::image::option_verbose = true;
            break;

        case 'j':
            std::cout << "Warning: Ignoring deleted option `-j'.\n";
            break;

        case 'n':
            pixel_terrain::image::option_nether = true;
            break;

        case 'o':
            pixel_terrain::image::option_out_dir = re_optarg;
            break;

        case 'r':
            pixel_terrain::image::option_generate_range = true;
            break;

        case 's':
            pixel_terrain::image::option_show_stat = true;
            break;

        case 'c':
            pixel_terrain::image::option_cache_dir = re_optarg;
            break;

        case 'h':
            print_usage();
            ::exit(0);

        case 'v':
            print_version();
            ::exit(0);

        default:
            return 1;
        }
    }

    if (re_optind != argc - 1) {
        print_usage();
        std::exit(1);
    }

    pixel_terrain::image::init_block_list();

    pixel_terrain::image::generate_all(argv[re_optind]);

    return 0;
}
