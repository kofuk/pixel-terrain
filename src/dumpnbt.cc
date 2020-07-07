#include <algorithm>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <optlib/optlib.h>

#include "nbt/region.hh"
#include "version.hh"

namespace {
    bool dump_coord(const std::string &filename, const std::string &outfmt,
                    int x, int z) {
        std::filesystem::path p(filename);
        std::string in_filename = p.filename();
        if (in_filename.size() > 4 && in_filename.find(".mca") == in_filename.size() - 4) {
            in_filename.erase(in_filename.end() - 4, in_filename.end());
        }

        std::string outname = outfmt;

        auto i = outname.begin();
        for (;;) {
            i = std::find(i, outname.end(), '%');
            if (i == outname.end()) break;
            switch (*(i + 1)) {
            case '1':
                outname.erase(i, i + 2);
                outname.insert(i, in_filename.begin(), in_filename.end());
                i += filename.size();
                break;

            case '2': {
                std::string x_str = std::to_string(x);
                outname.erase(i, i + 2);
                outname.insert(i, x_str.begin(), x_str.end());
                i += x_str.size();
                break;
            }

            case '3': {
                std::string z_str = std::to_string(z);
                outname.erase(i, i + 2);
                outname.insert(i, z_str.begin(), z_str.end());
                i += z_str.size();
                break;
            }
            }
        }

        try {
            pixel_terrain::anvil::region r =
                pixel_terrain::anvil::region(filename);
            auto [data, len] = r.chunk_data(x, z);

            std::ofstream out(outname, std::ios::binary);
            if (!out) {
                std::cerr << outname << ": unable to open output file\n";
                return false;
            }

            out.write(reinterpret_cast<char *>(data.get()), len);
        } catch (...) {
            std::cerr << outname << ": error dumping nbt\n";
            return false;
        }
        return true;
    }

    bool dump_all(const std::string &filename, const std::string &outfmt) {
        for (int x = 0; x < 32; ++x) {
            for (int z = 0; z < 32; ++z) {
                if (!dump_coord(filename, outfmt, x, z)) {
                    return false;
                }
            }
        }
        return true;
    }

    void print_usage(::optlib_parser *p) {
        std::cout << R"(usage: dumpnbt [OPTION]... IN_FILE...)" << std::endl;
        ::optlib_print_help(p, stdout);
    }

    void print_version() {
        std::cout << "dumpnbt (" << PROJECT_NAME << ' ' << VERSION_MAJOR << '.'
                  << VERSION_MINOR << '.' << VERSION_REVISION << ")\n";
    }
} // namespace

int main(int argc, char **argv) {
    ::optlib_parser *p = ::optlib_parser_new(argc, argv);

    ::optlib_parser_add_option(p, "out", 'o', true,
                               "Set output format. %1 replaced by input "
                               "filename, %2 and %3 replaced by chunk x, z.");
    ::optlib_parser_add_option(p, "only", 'O', true,
                               "Only dump specified coord (x,z).");
    ::optlib_parser_add_option(p, "help", 'h', false,
                               "Print this help then exit.");
    ::optlib_parser_add_option(p, "version", 'v', false,
                               "Print version then exit.");

    std::string outfmt = "%1+%2+%3.nbt";
    std::vector<std::pair<int, int>> coords;

    for (;;) {
        ::optlib_option *opt = optlib_next(p);
        if (!opt) {
            break;
        }
        switch (opt->short_opt) {
        case 'o':
            outfmt = opt->argval;
            break;

        case 'O': {
            int x, z;
            if (std::sscanf(opt->argval, "( %d , %d )", &x, &z) != 2) {
                std::cout << "Malformed coordinate. (example: \"(1,2)\")";
                ::optlib_parser_free(p);
                return 1;
            }
            coords.push_back({x, z});
        } break;

        case 'h':
            print_usage(p);
            ::optlib_parser_free(p);
            return 0;

        case 'v':
            print_version();
            ::optlib_parser_free(p);
            return 0;

        default:
            print_usage(p);
            ::optlib_parser_free(p);
            return 1;
        }
    }

    for (int i = p->optind; i < argc; ++i) {
        if (coords.empty()) {
            dump_all(p->argv[i], outfmt);
        } else {
            for (const std::pair<int, int> &coord : coords) {
                dump_coord(p->argv[i], outfmt, coord.first, coord.second);
            }
        }
    }

    ::optlib_parser_free(p);
}
