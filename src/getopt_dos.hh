#ifndef GETOPT_DOS_HH
#define GETOPT_DOS_HH

#include <cstring>
#include <iostream>

using namespace std;

#ifdef __unix__
#include <unistd.h>
#else
int optind = 1;
char *optarg;
#endif /* __unix__ */

/* getopt interface to parse DOS-style command line options. */
[[maybe_unused]] static inline int getopt_dos (int argc, char **argv,
                                               char *optstring) {
    if (optind >= argc) return -1;

    for (;;) {
        if (argv[optind][0] == '/') {
            break;
        } else {
            char *tmp = argv[optind];

            for (int i = optind; i < argc - 1; ++i) {
                argv[i] = argv[i + 1];
            }
            argv[argc - 1] = tmp;
        }
    }

    if (strlen (argv[optind]) != 2) {
        cerr << "Unrecognized option: " << (argv[optind] + 1) << endl;

        return '?';
    }

    char opt_char = argv[optind][1];
    size_t optlen = strlen (optstring);
    for (size_t i = 0; i < optlen; ++i) {
        if (optstring[i] == ':') continue;

        if (argv[optind][i] == opt_char) {
            if (i + 1 < optlen && optstring[i + 1] == ':') {
                if (optind + 1 < argc && argv[optind + 1][0] != '/') {
                    ++optind;
                    optarg = argv[optind];

                    return opt_char;
                } else {
                    cerr << "option " << opt_char << " requires an argument"
                         << endl;

                    return '?';
                }
            }

            break;
        }
    }

    ++optind;

    return opt_char;
}

#endif
