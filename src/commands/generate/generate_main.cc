#include <filesystem>
#include <thread>

#include <optlib/optlib.h>

#include "../../logger/logger.hh"
#include "../../logger/pretty_printer.hh"
#include "../../nbt/Region.hh"
#include "blocks.hh"
#include "generate_main.hh"
#include "generator.hh"
#include "worker.hh"

namespace pixel_terrain::commands::generate {
    namespace {
        /* TODO: Should move to functions/imagegen */
        void write_range_file(int start_x, int start_z, int end_x, int end_z) {
            filesystem::path out_path(option_out_dir);
            out_path /= "chunk_range.json"s;

            ofstream out(out_path.string());
            if (!out) return;

            out << "[" << start_x << ", " << start_z << ", " << end_x << ", "
                << end_z << "]" << endl;
        }

        /* TODO: Should move to functions/imagegen */
        void generate_all(string src_dir) {
            if (!option_journal_dir.empty()) {
                try {
                    filesystem::create_directories(option_journal_dir);
                } catch (filesystem::filesystem_error const &e) {
                    logger::e("cannot create journal directory: "s + e.what());

                    exit(1);
                }
            }

            start_worker();

            filesystem::directory_iterator dirents(src_dir);
            int nfiles = distance(begin(dirents), end(dirents));
            pretty_printer::set_total(nfiles);

            int min_x = numeric_limits<int>::max();
            int min_z = numeric_limits<int>::max();
            int max_x = numeric_limits<int>::min();
            int max_z = numeric_limits<int>::min();

            for (filesystem::directory_entry const &path :
                 filesystem::directory_iterator(src_dir)) {
                if (path.is_directory()) continue;

                string name = path.path().filename().string();

                if (name[0] != 'r' || name[1] != '.') continue;
                size_t i = 2;
                while (i < name.size() &&
                       (name[i] == '-' || ('0' <= name[i] && name[i] <= '9')))
                    ++i;

                if (name[i] == '.' && i + 1 >= name.size()) continue;
                ++i;

                size_t start_z = i;
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

                anvil::Region *r;
                try {
                    if (option_journal_dir.empty()) {
                        r = new anvil::Region(path.path().string());
                    } else {
                        r = new anvil::Region(path.path().string(),
                                              option_journal_dir);
                    }
                } catch (exception const &e) {
                    logger::e("failed to read region: "s +
                              path.path().string());
                    logger::e(e.what());

                    continue;
                }

                shared_ptr<RegionContainer> rc(new RegionContainer(r, x, z));

                for (int off_x = 0; off_x < 2; ++off_x) {
                    for (int off_z = 0; off_z < 2; ++off_z) {
                        queue_item(shared_ptr<QueuedItem>(
                            new QueuedItem(rc, off_x, off_z)));
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

    int main(int argc, char **argv) {
        option_jobs = thread::hardware_concurrency();
        option_out_dir = ".";

        optlib_parser *parser = optlib_parser_new(argc, argv);
        optlib_parser_add_option(
            parser, "jobs", 'j', true,
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
        optlib_parser_add_option(parser, "verbose", 'v', false,
                                 "Output verbose (debug) log.");

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
            case 'v':
                option_verbose = true;
                break;

            case 'j':
                try {
                    option_jobs = stoi(opt->argval);
                    if (option_jobs <= 0) {
                        throw out_of_range("concurrency is negative");
                    }
                } catch (invalid_argument const &e) {
                    cout << "Invalid concurrency." << endl;
                    optlib_print_help(parser, stdout);
                    optlib_parser_free(parser);
                    exit(1);
                } catch (out_of_range const &e) {
                    cout << "Concurrency is out of permitted range." << endl;
                    optlib_parser_free(parser);
                    exit(1);
                }
                break;

            case 'n':
                option_nether = true;
                break;

            case 'o':
                option_out_dir = opt->argval;
                break;

            case 'r':
                option_generate_range = true;
                break;

            case 'U':
                option_journal_dir = opt->argval;
                break;
            }
        }

        if (optind != argc - 1) {
            optlib_print_help(parser, stdout);
            optlib_parser_free(parser);
            exit(1);
        }

        init_block_list();

        generate_all(argv[optind]);

        return 0;
    }
} // namespace pixel_terrain::commands::generate
