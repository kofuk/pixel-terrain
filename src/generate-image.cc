// SPDX-License-Identifier: MIT

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <regetopt.h>

#include "image/image.hh"
#include "logger/logger.hh"
#include "logger/pretty_printer.hh"
#include "nbt/utils.hh"
#include "pixel-terrain.hh"
#include "utils/path_hack.hh"
#include "version.hh"

namespace pixel_terrain::image {
    namespace {
        void write_range_file(int start_x, int start_z, int end_x, int end_z,
                              options opt) {
            std::filesystem::path out_path(opt.out_dir);
            out_path /= PATH_STR_LITERAL("chunk_range.json");

            std::ofstream out(out_path);
            if (!out) return;

            out << "[" << start_x << ", " << start_z << ", " << end_x << ", "
                << end_z << "]\n";
        }

        void generate_single_region(std::filesystem::path const &region_file,
                                    std::filesystem::path const &out_file,
                                    image_generator *generator, options opt) {
            anvil::region *r;
            try {
                if (opt.cache_dir.empty()) {
                    r = new anvil::region(region_file);
                } else {
                    r = new anvil::region(region_file, opt.cache_dir);
                }
            } catch (std::exception const &e) {
                logger::L(logger::ERROR, "failed to read region: %s\n",
                          region_file.string().c_str());
                logger::L(logger::ERROR, "%s\n", e.what());

                return;
            }

            generator->queue(std::shared_ptr<region_container>(
                new region_container(r, out_file)));
        }

        void generate_all(std::string src_dir, options opt) {
            if (!opt.cache_dir.empty()) {
                try {
                    std::filesystem::create_directories(opt.cache_dir);
                } catch (std::filesystem::filesystem_error const &e) {
                    using namespace std::literals::string_literals;
                    logger::L(logger::ERROR,
                              "cannot create cache directory: %s\n", e.what());

                    exit(1);
                }
            }

            image_generator generator(opt);
            generator.start();

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

                if (opt.generate_range) {
                    if (x < min_x) min_x = x;
                    if (x > max_x) max_x = x;
                    if (z < min_z) min_z = z;
                    if (z > max_z) max_z = z;
                }

                std::filesystem::path out_file(opt.out_dir);
                path_string out_name;
                out_name.append(std::filesystem::path(std::to_string(x)));
                out_name.append(PATH_STR_LITERAL(","));
                out_name.append(std::filesystem::path(std::to_string(z)));
                out_name.append(PATH_STR_LITERAL(".png"));
                out_file /= out_name;
                generate_single_region(path.path(), out_file, &generator, opt);

                pretty_printer::increment_progress_bar();
            }

            logger::L(logger::DEBUG, "Waiting for worker to finish...\n");
            generator.finish();
            pretty_printer::finish_progress_bar();

            if (opt.generate_range) {
                /* max_* is exclusive */
                ++max_x;
                ++max_z;
                write_range_file(min_x, min_z, max_x, max_z, opt);
            }

            logger::show_stat();
        }
    } // namespace
} // namespace pixel_terrain::image

namespace {
    void print_usage() {
        std::cout << &R"(
Usage: pixel-terrain generate-image [option]... [--] <dir>
Load save data in <dir>, and generate image.

  -c DIR, --cache-dir DIR  Use DIR as cache direcotry.
  -j N, --jobs=N           Execute N jobs concurrently.
  -n, --nether             Use image generator optimized to nether.
  -o DIR, --out DIR        Save generated images to DIR.
  -r, --gen-range          Generate JSON file indicates X and Z range block exists.
  -V, -VV, -VVV            Set log level. Specifying multiple times increases log level.
      --help               Print this usage and exit.
)"[1];
    }

    struct re_option long_options[] = {
        {"jobs", re_required_argument, nullptr, 'j'},
        {"cache-dir", re_required_argument, nullptr, 'c'},
        {"nether", re_no_argument, nullptr, 'n'},
        {"out", re_required_argument, nullptr, 'o'},
        {"gen-range", re_no_argument, nullptr, 'r'},
        {"help", re_no_argument, nullptr, 'h'},
        {0, 0, 0, 0}};
} // namespace

namespace pixel_terrain {
    int image_main(int argc, char **argv) {
        pixel_terrain::image::options options;

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
                    options.n_jobs = std::stoi(re_optarg);
                    if (options.n_jobs <= 0) {
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
                options.nether = true;
                break;

            case 'o':
                options.out_dir = re_optarg;
                break;

            case 'r':
                options.generate_range = true;
                break;

            case 'c':
                options.cache_dir = re_optarg;
                break;

            case 'h':
                print_usage();
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

        pixel_terrain::image::generate_all(argv[re_optind], options);

        return 0;
    }
} // namespace pixel_terrain
