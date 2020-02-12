#include <array>
#include <csetjmp>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

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

    start_worker(thread::hardware_concurrency());

    delete r;
}

static void print_usage() {
    cout << "usage: gen_map src_dir region_x region_z" << endl;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        print_usage();

        return 0;
    }

    init_block_list();

    generate_all(argv[1], stoi(argv[2]), stoi(argv[3]));

    return 0;
}
