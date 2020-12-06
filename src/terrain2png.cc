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

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <regetopt/regetopt.h>

#include "image/blocks.hh"
#include "image/generator.hh"
#include "image/worker.hh"
#include "logger/logger.hh"
#include "logger/pretty_printer.hh"
#include "nbt/utils.hh"
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

        void generate_single_region(std::filesystem::path const &region_file,
                                    std::filesystem::path const &out_file) {
            anvil::region *r;
            try {
                if (option_cache_dir.empty()) {
                    r = new anvil::region(region_file);
                } else {
                    r = new anvil::region(region_file, option_cache_dir);
                }
            } catch (std::exception const &e) {
                logger::L(logger::ERROR, "failed to read region: %s\n",
                          region_file.string().c_str());
                logger::L(logger::ERROR, "%s\n", e.what());

                return;
            }

            queue_item(std::shared_ptr<region_container>(
                new region_container(r, out_file)));
        }

        void generate_all(std::string src_dir) {
            if (!option_cache_dir.empty()) {
                try {
                    std::filesystem::create_directories(option_cache_dir);
                } catch (std::filesystem::filesystem_error const &e) {
                    using namespace std::literals::string_literals;
                    logger::L(logger::ERROR,
                              "cannot create cache directory: %s\n", e.what());

                    exit(1);
                }
            }

            start_worker();

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

                int x, z;
                try {
                    auto [rx, rz] =
                        pixel_terrain::nbt::utils::parse_region_file_path(
                            path.path());
                    x = rx;
                    z = rz;
                } catch (std::invalid_argument const &e) {
                    logger::L(
                        logger::INFO, "%s: Skipping non-region file: %s\n",
                        path.path().filename().string().c_str(), e.what());
                    continue;
                }

                if (option_generate_range) {
                    if (x < min_x) min_x = x;
                    if (x > max_x) max_x = x;
                    if (z < min_z) min_z = z;
                    if (z > max_z) max_z = z;
                }

                std::filesystem::path out_file(option_out_dir);
                path_string out_name;
                out_name.append(std::filesystem::path(std::to_string(x)));
                out_name.append(PATH_STR_LITERAL(","));
                out_name.append(std::filesystem::path(std::to_string(z)));
                out_name.append(PATH_STR_LITERAL(".png"));
                out_file /= out_name;
                generate_single_region(path.path(), out_file);

                pretty_printer::increment_progress_bar();
            }

            finish_worker();
            pretty_printer::finish_progress_bar();
            logger::L(logger::DEBUG, "Waiting for worker to finish...\n");
            wait_for_worker();

            if (option_generate_range) {
                /* max_* is exclusive */
                ++max_x;
                ++max_z;
                write_range_file(min_x, min_z, max_x, max_z);
            }

            logger::show_stat();
        }
    } // namespace
} // namespace pixel_terrain::image

namespace {
    void print_usage() {
        std::cout << R"(usage: terrain2png [OPTION]... [--] DIR
Load save data in DIR, and generate image.

  -j N, --jobs=N           Execute N jobs concurrently.
  -c DIR, --cache-dir DIR  Use DIR as cache direcotry.
  -n, --nether             Use image generator optimized to nether.
  -o DIR, --out DIR        Save generated images to DIR.
  -r, --gen-range          Generate JSON file indicates X and Z range block exists.
  -s, --statistics         Show statistics about generation.
  -V                       Set log level. Specifying multiple times increases log level.
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
in Python. Visit https://github.com/kofuk/pixel-terrain for more information
and the source code.
)";
    }

    struct re_option long_options[] = {
        {"jobs", re_required_argument, nullptr, 'j'},
        {"cache-dir", re_required_argument, nullptr, 'c'},
        {"nether", re_no_argument, nullptr, 'n'},
        {"out", re_required_argument, nullptr, 'o'},
        {"gen-range", re_no_argument, nullptr, 'r'},
        {"help", re_no_argument, nullptr, 'h'},
        {"version", re_no_argument, nullptr, 'v'},
        {0, 0, 0, 0}};
} // namespace

int main(int argc, char **argv) {
    pixel_terrain::image::option_jobs = std::thread::hardware_concurrency();
    pixel_terrain::image::option_out_dir = PATH_STR_LITERAL(".");

    for (;;) {
        int opt = regetopt(argc, argv, "j:c:no:rV", long_options, nullptr);
        if (opt < 0) {
            break;
        }

        switch (opt) {
        case 'V':
            ++pixel_terrain::logger::log_level;
            break;

        case 'j':
            try {
                pixel_terrain::image::option_jobs = std::stoi(re_optarg);
                if (pixel_terrain::image::option_jobs <= 0) {
                    throw std::out_of_range("concurrency is negative");
                }
            } catch (std::invalid_argument const &e) {
                std::cout << "Invalid concurrency.\n";
                std::exit(1);
            } catch (std::out_of_range const &e) {
                std::cout << "Concurrency is out of permitted range.\n";
                std::exit(1);
            }
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
