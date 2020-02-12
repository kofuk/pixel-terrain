#include <array>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include <getopt.h>
#include <unistd.h>

#include "Region.hh"
#include "blocks.hh"
#include "worker.hh"

using namespace std;

static void generate_all(string src_dir, string out_dir, int x, int z, int jobs,
                         bool verbose) {
    filesystem::path path(src_dir);
    path /= "r." + to_string(x) + "." + to_string(z) + ".mca";

    Anvil::Region *r = new Anvil::Region(path);

    init_worker(r, x, z, out_dir, verbose);

    for (int off_x = 0; off_x < 2; ++off_x) {
        for (int off_z = 0; off_z < 2; ++off_z) {
            queue_offset(new pair<int, int>(off_x, off_z));
        }
    }

    start_worker(jobs);

    delete r;
}

static void print_usage() {
    cout << "Usage: gen_map [OPTION]... [--] SRC_DIR REGION_X REGION_Z" << endl
         << endl;
    cout << " -j N, --jobs=N  generate N images concurrently. (default: "
            "processor count)"
         << endl;
    cout << " -o --out        specify output directory. (default: current "
            "directory)"
         << endl;
    cout << " -v --verbose    output verbose log" << endl;
    cout << " -h --help       display this help and exit." << endl;
    cout << " -V --version    display version information and exit" << endl;
}

static void print_version() {}

static option command_options[] = {
    {"help", no_argument, 0, 'h'},      {"version", no_argument, 0, 'V'},
    {"verbose", no_argument, 0, 'v'},   {"jobs", required_argument, 0, 'j'},
    {"out", required_argument, 0, 'o'}, {0, 0, 0, 0}};

int main(int argc, char **argv) {
    bool verbose = false;
    int jobs = thread::hardware_concurrency();
    string out_dir = ".";

    int c;
    for (;;) {
        c = getopt_long(argc, argv, "hvVj:o:", command_options, nullptr);

        if (c == -1) break;

        switch (c) {
        case 'h':
            print_usage();
            exit(0);
            break;

        case 'v':
            verbose = true;
            break;

        case 'V':
            print_version();
            exit(0);
            break;

        case 'j':
            jobs = stoi(optarg);
            break;

        case 'o':
            out_dir = optarg;
            break;

        default:
            print_usage();
            exit(1);
        }
    }

    if (optind + 2 >= argc) {
        print_usage();

        return 0;
    }

    init_block_list();

    generate_all(argv[optind], out_dir, stoi(argv[optind + 1]),
                 stoi(argv[optind + 2]), jobs, verbose);

    return 0;
}
