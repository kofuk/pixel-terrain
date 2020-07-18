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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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

#include <optlib/optlib.h>

#include "image/blocks.hh"
#include "image/generator.hh"
#include "image/worker.hh"
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
            if (!option_journal_dir.empty()) {
                try {
                    std::filesystem::create_directories(option_journal_dir);
                } catch (std::filesystem::filesystem_error const &e) {
                    using namespace std::literals::string_literals;
                    logger::e("cannot create journal directory: "s + e.what());

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
                    if (option_journal_dir.empty()) {
                        r = new anvil::region(path.path());
                    } else {
                        r = new anvil::region(path.path(), option_journal_dir);
                    }
                } catch (std::exception const &e) {
                    logger::e("failed to read region: " + path.path().string());
                    logger::e(e.what());

                    continue;
                }

                std::shared_ptr<region_container> rc(
                    new region_container(r, x, z));

                for (int off_x = 0; off_x < 2; ++off_x) {
                    for (int off_z = 0; off_z < 2; ++off_z) {
                        queue_item(std::shared_ptr<queued_item>(
                            new queued_item(rc, off_x, off_z)));
                    }
                }

                pretty_printer::increment_progress_bar();
            }

            finish_worker();
            wait_for_worker();

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
        }
    } // namespace
} // namespace pixel_terrain::image

namespace {
    void print_usage(::optlib_parser *parser) {
        std::cout << "usage: terrain2png [OPTION]... DIR\n";
        std::cout << "Load save data in DIR, and generate image." << std::endl;
        ::optlib_print_help(parser, stdout);
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
} // namespace

int main(int argc, char **argv) {
    pixel_terrain::image::option_jobs = std::thread::hardware_concurrency();
    pixel_terrain::image::option_out_dir = PATH_STR_LITERAL(".");

    optlib_parser *parser = optlib_parser_new(argc, argv);
    optlib_parser_add_option(parser, "jobs", 'j', true,
                             "Specify max concurrency for image generation.");
    optlib_parser_add_option(
        parser, "journal", 'U', true,
        "Specify cache directory to generate images efficiently.");
    optlib_parser_add_option(parser, "nether", 'n', false,
                             "Use image generator optimized to nether.");
    optlib_parser_add_option(parser, "out", 'o', true,
                             "Specify image output directory.");
    optlib_parser_add_option(
        parser, "gen-range", 'r', false,
        "Generate JSON file indicates X and Z range block exists.");
    optlib_parser_add_option(parser, "verbose", 'V', false,
                             "Output verbose (debug) log.");
    optlib_parser_add_option(parser, "help", 'h', false,
                             "Print usage and exit.");
    optlib_parser_add_option(parser, "version", 'v', false,
                             "Print version and exit.");

    for (;;) {
        optlib_option *opt = optlib_next(parser);
        if (parser->finished) {
            break;
        }
        if (!opt) {
            optlib_print_help(parser, stdout);
            optlib_parser_free(parser);
            exit(1);
        }

        switch (opt->short_opt) {
        case 'V':
            pixel_terrain::image::option_verbose = true;
            break;

        case 'j':
            try {
                pixel_terrain::image::option_jobs = std::stoi(opt->argval);
                if (pixel_terrain::image::option_jobs <= 0) {
                    throw std::out_of_range("concurrency is negative");
                }
            } catch (std::invalid_argument const &e) {
                std::cout << "Invalid concurrency.\n";
                optlib_print_help(parser, stdout);
                optlib_parser_free(parser);
                std::exit(1);
            } catch (std::out_of_range const &e) {
                std::cout << "Concurrency is out of permitted range.\n";
                optlib_parser_free(parser);
                std::exit(1);
            }
            break;

        case 'n':
            pixel_terrain::image::option_nether = true;
            break;

        case 'o':
            pixel_terrain::image::option_out_dir = opt->argval;
            break;

        case 'r':
            pixel_terrain::image::option_generate_range = true;
            break;

        case 'U':
            pixel_terrain::image::option_journal_dir = opt->argval;
            break;

        case 'h':
            print_usage(parser);
            ::optlib_parser_free(parser);
            ::exit(0);

        case 'v':
            print_version();
            ::optlib_parser_free(parser);
            ::exit(0);
        }
    }

    if (parser->optind != argc - 1) {
        optlib_print_help(parser, stdout);
        optlib_parser_free(parser);
        std::exit(1);
    }

    pixel_terrain::image::init_block_list();

    pixel_terrain::image::generate_all(argv[parser->optind]);

    return 0;
}
