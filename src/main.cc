#include <algorithm>
#include <array>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include <getopt.h>
#include <unistd.h>

#include "functions/blockserver/server.hh"
#include "functions/imagegen/blocks.hh"
#include "functions/imagegen/worker.hh"
#include "logger/logger.hh"
#include "logger/pretty_printer.hh"
#include "nbt/Region.hh"

using namespace std;

static void write_range_file (int start_x, int start_z, int end_x, int end_z) {
    filesystem::path out_path (option_out_dir);
    out_path /= "chunk_range.json"s;

    ofstream out (out_path.string ());
    if (!out) return;

    out << "[" << start_x << ", " << start_z << ", " << end_x << ", " << end_z
        << "]" << endl;
}

static void generate_all (string src_dir) {
    if (!option_journal_dir.empty ()) {
        try {
            filesystem::create_directories (option_journal_dir);
        } catch (filesystem::filesystem_error const &e) {
            logger::e ("cannot create journal directory: "s + e.what ());

            exit (1);
        }
    }

    start_worker ();

    filesystem::directory_iterator dirents (src_dir);
    int nfiles = distance (begin (dirents), end (dirents));
    pretty_printer::set_total (nfiles);

    int min_x = numeric_limits<int>::max ();
    int min_z = numeric_limits<int>::max ();
    int max_x = numeric_limits<int>::min ();
    int max_z = numeric_limits<int>::min ();

    for (filesystem::directory_entry const &path :
         filesystem::directory_iterator (src_dir)) {
        if (path.is_directory ()) continue;

        string name = path.path ().filename ().string ();

        if (name[0] != 'r' || name[1] != '.') continue;
        size_t i = 2;
        while (i < name.size () &&
               (name[i] == '-' || ('0' <= name[i] && name[i] <= '9')))
            ++i;

        if (name[i] == '.' && i + 1 >= name.size ()) continue;
        ++i;

        size_t start_z = i;
        while (i < name.size () &&
               (name[i] == '-' || ('0' <= name[i] && name[i] <= '9')))
            ++i;

        int x = stoi (name.substr (2, i - 2));
        int z = stoi (name.substr (start_z, i - start_z));

        if (option_generate_range) {
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (z < min_z) min_z = z;
            if (z > max_z) max_z = z;
        }

        anvil::Region *r;
        try {
            if (option_journal_dir.empty ()) {
                r = new anvil::Region (path.path ().string ());
            } else {
                r = new anvil::Region (path.path ().string (),
                                       option_journal_dir);
            }
        } catch (exception const &e) {
            logger::e ("failed to read region: "s + path.path ().string ());
            logger::e (e.what ());

            continue;
        }

        shared_ptr<RegionContainer> rc (new RegionContainer (r, x, z));

        for (int off_x = 0; off_x < 2; ++off_x) {
            for (int off_z = 0; off_z < 2; ++off_z) {
                queue_item (new QueuedItem (rc, off_x, off_z));
            }
        }

        pretty_printer::increment_progress_bar ();
    }

    for (int i = 0; i < option_jobs; ++i) {
        QueuedItem *item = new QueuedItem (nullptr, 0, 0);
        queue_item (item);
    }

    wait_for_worker ();

    if (option_generate_range) {
        /* max_* is exclusive */
        ++max_x;
        ++max_z;
        min_x *= 2;
        min_z *= 2;
        max_x *= 2;
        max_z *= 2;
        write_range_file (min_x, min_z, max_x, max_z);
    }
}

static void print_usage () {
    cout << "Usage: mcmap generate [OPTION]... SRC_DIR" << endl;
    cout << "       mcmap server CONFIG" << endl;
    cout << "Modes:" << endl;
    cout << " generate  image generating mode" << endl;
    cout << " server    block info provider server" << endl << endl;
    cout << "Global options:" << endl;
    cout << " --help     display this help and exit" << endl;
    cout << " --version  display version information and exit" << endl << endl;
    cout << "GENERATE mode options:" << endl;
    cout << " -j N, --jobs=N    generate N images concurrently. (default: "
            "processor count)"
         << endl;
    cout << " -n --nether       Use nether image generation algorythm "
            "(experimental)."
         << endl;
    cout << " -o --out DIR      specify output directory. (default: current "
            "directory)"
         << endl;
    cout << " -r --gen-range    output chunk range to chunk_range.json" << endl;
    cout << " -U --journal DIR  read journal from DIR" << endl;
    cout << " -v --verbose      output verbose log" << endl << endl;
    cout << "SERVER mode options:" << endl;
    cout << " --daemon         run server process in background" << endl;
    cout << " --overworld DIR  specify directory for overworld" << endl;
    cout << " --nether DIR     specify directory for nether" << endl;
    cout << " --end DIR        specify directory for end" << endl;
    cout << " --help    display protocol and config detail and exit" << endl;
}

static void print_version () {
    cout << "mcmap 3.0" << endl;
    cout << "Copyright (C) 2020, Koki Fukuda." << endl;
    cout << "This program includes C++ re-implementation of" << endl
         << "anvil-parser and nbt, originally written in Python." << endl
         << "Visit https://github.com/kofuk/minecraft-image-gemerator" << endl
         << "for more information and the source code." << endl;
}

static option generate_command_options[] = {
    {"jobs", required_argument, 0, 'j'},
    {"journal", required_argument, 0, 'U'},
    {"nether", no_argument, 0, 'n'},
    {"out", required_argument, 0, 'o'},
    {"gen-range", no_argument, 0, 'r'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}};

static option server_command_options[] = {
    {"daemon", no_argument, 0, 'd'},
    {"overworld", required_argument, 0, 'o'},
    {"nether", required_argument, 0, 'n'},
    {"end", required_argument, 0, 'e'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};

static int generate_command (int argc, char **argv) {
    option_jobs = thread::hardware_concurrency ();
    option_out_dir = ".";

    int c;
    for (;;) {
        c = getopt_long (argc, argv, "j:no:rU:v", generate_command_options,
                         nullptr);
        if (c == -1) break;

        switch (c) {
        case 'V':
            print_version ();
            exit (0);
            break;

        case 'v':
            option_verbose = true;
            break;

        case 'j':
            option_jobs = stoi (optarg);
            break;

        case 'n':
            option_nether = true;
            break;

        case 'o':
            option_out_dir = optarg;
            break;

        case 'r':
            option_generate_range = true;
            break;

        case 'U':
            option_journal_dir = optarg;
            break;

        default:
            print_usage ();
            exit (1);
        }
    }

    if (optind >= argc) {
        print_usage ();

        return 0;
    }

    init_block_list ();

    generate_all (argv[optind]);

    return 0;
}

static int server_command (int argc, char **argv) {
    bool daemon_mode = false;
    int opt;
    for (;;) {
        opt = getopt_long (argc, argv, "dho:n:e:", server_command_options,
                           nullptr);

        if (opt == -1) break;

        switch (opt) {
        case 'd':
            daemon_mode = true;
            break;

        case 'h':
            server::print_protocol_detail ();
            exit (0);

        case 'o':
            server::overworld_dir = optarg;
            break;

        case 'n':
            server::nether_dir = optarg;
            break;

        case 'e':
            server::end_dir = optarg;
            break;

        default:
            print_usage ();
            exit (1);
        }
    }

    if (argc - optind == 0) {
        server::launch_server (daemon_mode);
    } else {
        print_usage ();
        exit (1);
    }

    return 0;
}

static void handle_commands (int argc, char **argv) {
    if (!strcmp (argv[0], "generate")) {
        generate_command (argc, argv);
    } else if (!strcmp (argv[0], "server")) {
        server_command (argc, argv);
    } else {
        cout << "unrecognized option: " << argv[0] << endl;
        print_usage ();
        exit (1);
    }
}

int main (int argc, char **argv) {
    if (argc < 2) {
        print_usage ();
        exit (1);
    }

    if (!strcmp (argv[1], "--help")) {
        print_usage ();
        exit (0);
    } else if (!strcmp (argv[1], "--version")) {
        print_version ();
        exit (0);
    } else {
        handle_commands (--argc, ++argv);
    }

    return 0;
}
