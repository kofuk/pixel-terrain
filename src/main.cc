#include <algorithm>
#include <array>
#include <bits/getopt_core.h>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#ifdef __unix__
#include <unistd.h>

#ifdef _GNU_SOURCE
#include <getopt.h>
#endif /* _GNU_SOURCE */

#else
#include "getopt_dos.hh"
#endif /* __unix__ */

#include <cpptoml.h>

#include "Region.hh"
#include "blocks.hh"
#include "server.hh"
#include "worker.hh"

using namespace std;

static void write_progress_file(int progress) {
    filesystem::path out_path(option_out_dir);
    out_path /= "gen_progress.txt";

    ofstream out(out_path.string());
    out << progress << endl;
}

static void write_range_file(int start_x, int start_z, int end_x, int end_z) {
    filesystem::path out_path(option_out_dir);
    out_path /= "chunk_range.json";

    ofstream out(out_path.string());
    if (!out) return;

    out << "[" << start_x << ", " << start_z << ", " << end_x << ", " << end_z
        << "]" << endl;
}

static void generate_all(string src_dir) {
    if (!option_journal_dir.empty()) {
        try {
            filesystem::create_directories(option_journal_dir);
        } catch(filesystem::filesystem_error const &e) {
            cerr << "cannot create journal directory: " << e.what() << endl;

            exit(1);
        }
    }

    start_worker();

    filesystem::directory_iterator dir =
        filesystem::directory_iterator(src_dir);

    int progress = -1;
    int nfile = distance(begin(dir), end(dir));
    int n_processed = 0;

    int min_x = numeric_limits<int>::max();
    int min_z = numeric_limits<int>::max();
    int max_x = numeric_limits<int>::min();
    int max_z = numeric_limits<int>::min();

    for (filesystem::directory_entry const &path :
         filesystem::directory_iterator(src_dir)) {
        ++n_processed;

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

        Anvil::Region *r;
        if (option_journal_dir.empty()) {
            r = new Anvil::Region(path.path().string());
        } else {
            r = new Anvil::Region(path.path().string(), option_journal_dir);
        }

        RegionContainer *rc = new RegionContainer(r, x, z);
        rc->set_ref_count(4);

        for (int off_x = 0; off_x < 2; ++off_x) {
            for (int off_z = 0; off_z < 2; ++off_z) {
                QueuedItem *item = new QueuedItem(rc, off_x, off_z);
                queue_item(item);
            }
        }

        if (option_generate_progress) {
            int cur_progress = n_processed * 100 / nfile;
            if (cur_progress != progress) {
                progress = cur_progress;
                write_progress_file(progress);
            }
        }
    }

    for (int i = 0; i < option_jobs; ++i) {
        QueuedItem *item = new QueuedItem(nullptr, 0, 0);
        queue_item(item);
    }

    wait_for_worker();

    if (option_generate_progress) {
        filesystem::path progress_file(option_out_dir);
        progress_file /= "gen_progress.txt";
        filesystem::remove(progress_file);
    }

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

static bool load_config(string filename) {
    try {
        shared_ptr<cpptoml::table> config = cpptoml::parse_file(filename);

        if (config->contains("jobs")) {
            option_jobs = *config->get_as<int>("jobs");
        }
        if (config->contains("nether")) {
            option_nether = *config->get_as<bool>("nether");
        }
        if (config->contains("out")) {
            option_out_dir = *config->get_as<string>("out");
        }
        if (config->contains("gen-progress")) {
            option_generate_progress = *config->get_as<bool>("gen-progress");
        }
        if (config->contains("gen-range")) {
            option_generate_range = *config->get_as<bool>("gen-range");
        }
        if (config->contains("journal")) {
            option_journal_dir = *config->get_as<string>("journal");
        }
        if (config->contains("verbose")) {
            option_verbose = *config->get_as<bool>("verbose");
        }
    } catch (cpptoml::parse_exception const &e) {
        cerr << e.what() << endl;

        return false;
    }

    return true;
}

static void print_usage() {
#ifdef __unix__
    cout << "Usage: mcmap generate [OPTION]... SRC_DIR" << endl;
    cout << "       mcmap server CONFIG" << endl;
    cout << "Modes:" << endl;
    cout << " generate  image generating mode" << endl;
    cout << " server    block info provider server" << endl << endl;
    cout << "Global options:" << endl;
    cout << " --help     display this help and exit" << endl;
    cout << " --version  display version information and exit" << endl << endl;
    cout << "GENERATE mode options:" << endl;
    cout << " -c, --config FILE read config file." << endl;
    cout << " -j N, --jobs=N    generate N images concurrently. (default: "
            "processor count)"
         << endl;
    cout << " -n --nether       Use nether image generation algorythm "
            "(experimental)."
         << endl;
    cout << " -o --out DIR      specify output directory. (default: current "
            "directory)"
         << endl;
    cout << " -p --gen-progess  output progress to gen_progess.txt" << endl;
    cout << " -r --gen-range    output chunk range to chunk_range.json" << endl;
    cout << " -U --journal DIR  read journal from DIR" << endl;
    cout << " -v --verbose      output verbose log" << endl << endl;
    cout << "SERVER mode options:" << endl;
    cout << " --daemon  run server process in background" << endl;
    cout << " --help    display protocol and config detail and exit" << endl
         << endl;
    cout << "Note: long options is supported only on GNU system" << endl;
#else
    cout << "Usage: mcmap [OPTION]... SRC_DIR" << endl << endl;
    cout
        << " /j N    generate N images concurrently. (default: processor count)"
        << endl;
    cout << " /n      Use nether image generation algorythm (experimental)."
         << endl;
    cout << " /o DIR  specify output directory. (default: current directory)"
         << endl;
    cout << " /p      output progress to gen_progress.txt" << endl;
    cout << " /r      output chunk range to chunk_range.json" << endl;
    cout << " /v      output verbose log" << endl;
    cout << " /h      display this help and exit." << endl;
    cout << " /V      display version information and exit" << endl;
#endif /* __unix__ */
}

static void print_version() {
    cout << "mcmap 2.0" << endl;
    cout << "Copyright (C) 2020, Koki Fukuda." << endl;
    cout << "This program includes C++ re-implementation of" << endl
         << "anvil-parser and nbt, originally written in Python." << endl
         << "Visit https://github.com/kofuk/minecraft-image-gemerator" << endl
         << "for more information and the source code." << endl;
}

#ifdef _GNU_SOURCE
static option generate_command_options[] = {
    {"config", required_argument, 0, 'c'},
    {"jobs", required_argument, 0, 'j'},
    {"journal", required_argument, 0, 'U'},
    {"nether", no_argument, 0, 'n'},
    {"out", required_argument, 0, 'o'},
    {"gen-progress", no_argument, 0, 'p'},
    {"gen-range", no_argument, 0, 'r'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}};

static option server_command_options[] = {{"daemon", no_argument, 0, 'd'},
                                          {"help", no_argument, 0, 'h'},
                                          {0, 0, 0, 0}};
#endif /* _GNU_SOURCE */

static int generate_command(int argc, char **argv) {
    option_jobs = thread::hardware_concurrency();
    option_out_dir = ".";

    int c;
    for (;;) {
#ifdef _GNU_SOURCE
        c = getopt_long(argc, argv, "c:j:no:prU:v", generate_command_options,
                        nullptr);
#elif defined(__unix__)
        c = getopt(argc, argv, "c:j:no:prU:v");
#else
        c = getopt_dos(argc, argv, "c:j:no:prU:v");
#endif /* _GNU_SOURCE */
        if (c == -1) break;

        switch (c) {
        case 'V':
            print_version();
            exit(0);
            break;

        case 'v':
            option_verbose = true;
            break;

        case 'c':
            if (!load_config(optarg)) {
                print_usage();
                exit(1);
            }
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

        case 'p':
            option_generate_progress = true;
            break;

        case 'r':
            option_generate_range = true;
            break;

        case 'U':
            option_journal_dir = optarg;
            break;

        default:
            print_usage();
            exit(1);
        }
    }

    if (optind >= argc) {
        print_usage();

        return 0;
    }

    init_block_list();

    generate_all(argv[optind]);

    return 0;
}

static int server_command(int argc, char **argv) {
#ifdef __unix__
    bool daemon_mode = false;
    int opt;
    for (;;) {
#ifdef _GNU_SOURCE
        opt = getopt_long(argc, argv, "dh", server_command_options, nullptr);
#else
        opt = getopt(argc, argv, "dh");
#endif /* _GNU_SOURCE */

        if (opt == -1) break;

        switch (opt) {
        case 'd':
            daemon_mode = true;
            break;

        case 'h':
            Server::print_protocol_detail();
            exit(0);

        default:
            print_usage();
            exit(1);
        }
    }

    if (argc - optind == 1) {
        Server::launch_server(argv[optind], daemon_mode);
    } else {
        print_usage();
        exit(1);
    }
#else
    cout << "server mode on non *nix system is not supported." << endl;

    return 1;
#endif /* __unix__ */

    return 0;
}

int main(int argc, char **argv) {
#ifdef __unix__
    if (argc < 2) {
        print_usage();
        exit(1);
    }

    if (!strcmp(argv[1], "--help")) {
        print_usage();
        exit(0);
    } else if (!strcmp(argv[1], "--version")) {
        print_version();
        exit(0);
    } else if (!strcmp(argv[1], "generate")) {
        generate_command(--argc, ++argv);
    } else if (!strcmp(argv[1], "server")) {
        server_command(--argc, ++argv);
    } else {
        cout << "unrecognized option: " << argv[1] << endl;
        print_usage();
        exit(1);
    }

#else

    generate_command(argc, argv);

#endif /* __unix__ */

    return 0;
}
