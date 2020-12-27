// SPDX-License-Identifier: MIT

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <regetopt.h>

#include "image/image.hh"
#include "logger/logger.hh"
#include "nbt/utils.hh"
#include "pixel-terrain.hh"
#include "utils/path_hack.hh"
#include "version.hh"

namespace {
    void generate_image(std::string const &src,
                        pixel_terrain::image::options options) {
        using namespace pixel_terrain;

        if (options.label().empty()) options.set_label(src);

        image::image_generator generator(options);
        generator.start();

        std::filesystem::path src_path(src);
        if (std::filesystem::is_directory(src_path)) {
            generator.queue_all_in_dir(src, options);
        } else {
            generator.queue_region(src, options);
        }

        logger::L(logger::DEBUG, "Waiting for worker to finish...\n");
        generator.finish();

        logger::show_stat();
    }
} // namespace

namespace {
    void print_usage() {
        std::cout << &R"(
Usage: pixel-terrain image [option]... [--] <dir>    OR
       pixel-terrain image [option]... [--] <input file>

Generate map image.

  -c DIR, --cache-dir=DIR   Use DIR as cache direcotry.
  -j N, --jobs=N            Execute N jobs concurrently.
      --label               Label current configuration.
  -n, --nether              Use image generator optimized to nether.
  -o PATH, --out=PATH       Save generated images to PATH.
                            If PATH is a file, write output image to PATH eve if
                            there are multiple input file. Otherwise, output
                            filename is decided by format of --outname-format or
                            input filename.
      --outname-format=FMT  Specify format for output filename. Default value is
                            original filename with extension appended. Note that
                            proper extension will be appended automatically.
  -V, -VV, -VVV             Set log level. Specifying multiple times increases log level.
      --help                Print this usage and exit.

Output name format speficier:
 %X    X-coordinate of the region.
 %Z    Z-coordinate of the region.
 %%    A '%' character.
 )"[1];
    }

    struct re_option long_options[] = {
        {"jobs", re_required_argument, nullptr, 'j'},
        {"cache-dir", re_required_argument, nullptr, 'c'},
        {"nether", re_no_argument, nullptr, 'n'},
        {"out", re_required_argument, nullptr, 'o'},
        {"outname-format", re_required_argument, nullptr, 'F'},
        {"label", re_required_argument, nullptr, 'l'},
        {"help", re_no_argument, nullptr, 'h'},
        {0, 0, 0, 0}};
} // namespace

namespace pixel_terrain {
    int image_main(int argc, char **argv) {
        pixel_terrain::image::options options;

        bool should_generate = true;

        for (;;) {
            int opt = regetopt(argc, argv, "j:c:no:F:V", long_options, nullptr);
            if (opt < 0) {
                break;
            }

            should_generate = true;

            switch (opt) {
            case 'V':
                ++pixel_terrain::logger::log_level;
                break;

            case 'j':
                try {
                    options.set_n_jobs(std::stoi(::re_optarg));
                } catch (std::invalid_argument const &e) {
                    std::cout << "Invalid concurrency.\n";
                    std::exit(1);
                } catch (std::out_of_range const &e) {
                    std::cout << "Concurrency is out of permitted range.\n";
                    std::exit(1);
                }
                break;

            case 'n':
                options.set_is_nether(true);
                break;

            case 'o':
                options.set_out_path(::re_optarg);
                break;

            case 'F':
                options.set_outname_format(::re_optarg);
                break;

            case 'c':
                options.set_cache_dir(::re_optarg);
                break;

            case 'l':
                options.set_label(::re_optarg);
                break;

            case 'h':
                print_usage();
                std::exit(0);

            default:
                return 1;
            }
        }

        if (should_generate && ::re_optind == argc) {
            print_usage();
            std::exit(1);
        }

        pixel_terrain::image::init_block_list();

        generate_image(argv[::re_optind], options);

        return 0;
    }
} // namespace pixel_terrain
