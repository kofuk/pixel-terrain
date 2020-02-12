#include <array>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#ifdef _GNU_SOURCE

#include <getopt.h>
#include <unistd.h>

#else

#include <cstring>

#endif /* _GNU_SOURCE */

#include "Region.hh"
#include "blocks.hh"
#include "worker.hh"

using namespace std;

static void generate_all(string src_dir, int x, int z) {
    filesystem::path path(src_dir);
    path /= "r." + to_string(x) + "." + to_string(z) + ".mca";

    Anvil::Region *r = new Anvil::Region(path);

    init_worker(r, x, z);

    for (int off_x = 0; off_x < 2; ++off_x) {
        for (int off_z = 0; off_z < 2; ++off_z) {
            queue_offset(new pair<int, int>(off_x, off_z));
        }
    }

    start_worker();

    delete r;
}

static void print_usage() {
#ifdef _GNU_SOURCE
    cout << "Usage: mcmap [OPTION]... [--] SRC_DIR REGION_X REGION_Z" << endl
         << endl;
    cout << " -j N, --jobs=N  generate N images concurrently. (default: "
            "processor count)"
         << endl;
    cout << " -o --out        specify output directory. (default: current "
            "directory)"
         << endl;
    cout << " -n --nether     Use nether image generation algorythm "
            "(experimental)."
         << endl;
    cout << " -v --verbose    output verbose log" << endl;
    cout << " -h --help       display this help and exit." << endl;
    cout << " -V --version    display version information and exit" << endl;
#else
    cout << "Usage: mcmap [OPTION]... SRC_DIR REGION_X REGION_Z" << endl
         << endl;
    cout
        << " /j N    generate N images concurrently. (default: processor count)"
        << endl;
    cout << " /o DIR  specify output directory. (default: current directory)"
         << endl;
    cout << " /n      Use nether image generation algorythm (experimental)."
         << endl;
    cout << " /v      output verbose log" << endl;
    cout << " /h      display this help and exit." << endl;
    cout << " /V      display version information and exit" << endl;

#endif
}

static void print_version() {
    cout << "mcmap 1.0" << endl;
    cout << "Copyright (C) 2020, Koki Fukuda." << endl;
    cout << "This program includes C++ re-implementation of" << endl
         << "anvil-parser and nbt, originally written in Python." << endl
         << "Visit https://github.com/kofuk/minecraft-image-gemerator" << endl
         << "for more information and the source code." << endl;
}

#ifdef _GNU_SOURCE
static option command_options[] = {{"help", no_argument, 0, 'h'},
                                   {"version", no_argument, 0, 'V'},
                                   {"verbose", no_argument, 0, 'v'},
                                   {"jobs", required_argument, 0, 'j'},
                                   {"nether", no_argument, 0, 'n'},
                                   {"out", required_argument, 0, 'o'},
                                   {0, 0, 0, 0}};
#endif /* _GNU_SOURCE */

int main(int argc, char **argv) {
    int mcmap_optind = 1;

    option_jobs = thread::hardware_concurrency();
    option_out_dir = ".";

#ifdef _GNU_SOURCE
    int c;
    for (;;) {
        c = getopt_long(argc, argv, "hVvj:no:", command_options, nullptr);

        if (c == -1) break;

        switch (c) {
        case 'h':
            print_usage();
            exit(0);
            break;

        case 'v':
            option_verbose = true;
            break;

        case 'V':
            print_version();
            exit(0);
            break;

        case 'j':
            option_jobs = stoi(optarg);
            break;

        case 'n':
            option_nether = true;
            break;

        case 'o':
            option_out_dir = optarg;
            break;

        default:
            print_usage();
            exit(1);
        }
    }

    mcmap_optind = optind;

#else
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "/j")) {
            if (i + 1 < argc) {
                ++i; ++mcmap_optind;
                option_jobs = stoi(argv[i]);
            } else {
                print_usage();

                exit(1);
            }
        } else if (!strcmp(argv[i], "/o")) {
            option_out_dir = argv[i];
        } else if (!strcmp(argv[i], "/n")) {
            option_nether = true;
        } else if (!strcmp(argv[i], "/v")) {
            option_verbose = true;
        } else if (!strcmp(argv[i], "/h")) {
            print_usage();

            exit(0);
        } else if (!strcmp(argv[i], "/V")) {
            print_version();

            exit(0);
        } else {
            break;
        }
        ++mcmap_optind;
    }
#endif /* _GNU_SOURCE */

    if (mcmap_optind + 2 >= argc) {
        print_usage();

        return 0;
    }

    init_block_list();

    generate_all(argv[mcmap_optind], stoi(argv[mcmap_optind + 1]),
                 stoi(argv[mcmap_optind + 2]));

    return 0;
}
