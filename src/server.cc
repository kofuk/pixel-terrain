#ifdef __unix__

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cpptoml.h>

#include "Region.hh"
#include "server.hh"

using namespace std;

namespace Server {
    void print_protocol_detail() {
        cout << "Help for block info server's protocol and config" << endl
             << endl;

        cout << "PROTOCOL:" << endl;
        cout << " example request and response;" << endl;
        cout << " `>' means request and `<' means response" << endl;
        cout << "  >GET MMP/1.0" << endl;
        cout << "  >Dimension: overworld" << endl;
        cout << "  >Coord-X: 1" << endl;
        cout << "  >Coord-Z: 1" << endl;
        cout << "  >" << endl;
        cout << "  <MMP/1.0 200" << endl;
        cout << "  <" << endl;
        cout << R"(  <{"altitude": 63, "block": "minecraft:dirt"})" << endl
             << endl;
        cout << " Response Codes:" << endl;
        cout << "  200  OK. The request is handled properly and" << endl
             << "       altitude returned" << endl;
        cout << "  400  Bad Request. Probably request parse error" << endl;
        cout << "  404  Out of Range. No chunk existing on specified corrdinate"
             << endl;
        cout << "  500  Internal Server Error. Serverside error" << endl
             << endl;

        cout << "CONFIG:" << endl;
        cout << " [overworld]" << endl;
        cout << " dir = /path/to/overworld/save/data" << endl;
        cout << " [nether]" << endl;
        cout << " dir = /path/to/nether/save/data" << endl;
        cout << " [end]" << endl;
        cout << " dir = /path/to/end/save/data" << endl;
        cout << " ..." << endl;
    }

    static string overworld_dir;
    static string nether_dir;
    static string end_dir;

    static void parse_config(string config_filename) {
        try {
            shared_ptr<cpptoml::table> config =
                cpptoml::parse_file(config_filename);

            try {
                overworld_dir =
                    *config->get_qualified_as<string>("overworld.dir");
            } catch (out_of_range) {
                cerr << "warning: overworld.dir not specified" << endl;
            }

            try {
                nether_dir = *config->get_qualified_as<string>("nether.dir");
            } catch (out_of_range) {
                cerr << "warning: nether.dir not specified" << endl;
            }

            try {
                end_dir = *config->get_qualified_as<string>("end.dir");
            } catch (out_of_range) {
                cerr << "warning: end.dir not specified" << endl;
            }
        } catch (cpptoml::parse_exception const &e) {
            cerr << e.what() << endl;

            exit(1);
        }
    }

    static void handle_interrupt(int) {
        unlink("/tmp/mcmap.sock");
        exit(0);
    }

    static void send_response(FILE *f, int response_code, int altitude,
                              string *block) {
        fputs("MMP/1.0 ", f);
        putc('0' + response_code / 100, f);
        putc('0' + response_code / 10 % 10, f);
        putc('0' + response_code % 10, f);
        fputs("\r\n\r\n", f);

        string block_name = *block;

        if (block->find(":") == string::npos) {
            block_name = "minecraft:" + block_name;
        }

        if (response_code == 200) {
            fputs(string(R"({"altitude": )" + to_string(altitude) +
                         R"(, "block": ")" + block_name + R"("})" + "\n")
                      .c_str(),
                  f);
        }
    }

    static inline int positive_mod(int a, int b) {
        int mod = a % b;
        if (mod < 0) mod += b;

        return mod;
    }

    static void resolve_block(FILE *f, string dimen, long long int x,
                              long long int z) {
        int region_x;
        int region_z;

        if (x >= 0) {
            region_x = x / 512;
        } else {
            region_x = (x + 1) / 512 - 1;
        }

        if (z >= 0) {
            region_z = z / 512;
        } else {
            region_z = (z + 1) / 512 - 1;
        }

        int chunk_x = positive_mod(x, 512) / 16;
        int chunk_z = positive_mod(z, 512) / 16;
        int x_in_chunk = positive_mod(x, 512) % 16;
        int z_in_chunk = positive_mod(x, 512) % 16;

        filesystem::path region_file;

        if (dimen == "nether") {
            if (nether_dir.empty()) {
                send_response(f, 404, 0, nullptr);

                return;
            }

            region_file = nether_dir;
        } else if (dimen == "end") {
            if (end_dir.empty()) {
                send_response(f, 404, 0, nullptr);

                return;
            }

            region_file = end_dir;
        } else {
            if (overworld_dir.empty()) {
                send_response(f, 404, 0, nullptr);

                return;
            }

            region_file = overworld_dir;
        }

        region_file /=
            "r." + to_string(region_x) + "." + to_string(region_z) + ".mca";
        if (!filesystem::exists(region_file)) {
            send_response(f, 404, 0, nullptr);

            return;
        }

        Anvil::Region *r = new Anvil::Region(region_file.string());
        Anvil::Chunk *chunk = r->get_chunk(chunk_x, chunk_z);
        if (chunk == nullptr) {
            send_response(f, 404, 0, nullptr);

            return;
        }
        if (dimen == "nether") {
            bool air_found = false;
            for (int y = 127; y >= 0; --y) {
                string block = chunk->get_block(x_in_chunk, y, z_in_chunk);
                if (block == "air" || block == "cave_air" ||
                    block == "void_air") {
                    if (y == 0) {
                        send_response(f, 404, 0, nullptr);

                        break;
                    }

                    air_found = true;

                    continue;
                }

                if (!air_found) {
                    if (y == 0) {
                        send_response(f, 404, 0, nullptr);

                        break;
                    }
                    continue;
                }

                send_response(f, 200, y, &block);

                break;
            }
        } else {
            for (int y = 255; y >= 0; --y) {
                string block = chunk->get_block(x_in_chunk, y, z_in_chunk);
                if (block == "air" || block == "cave_air" ||
                    block == "void_air") {
                    if (y == 0) {
                        send_response(f, 404, 0, nullptr);

                        break;
                    }

                    continue;
                }

                send_response(f, 200, y, &block);

                break;
            }
        }

        delete chunk;
        delete r;
    }

    static void handle_request(int fd) {
        FILE *f = fdopen(fd, "rb+");
        if (f == NULL) {
            close(fd);

            return;
        }

        char *line = nullptr;
        size_t cap = 0;
        if (getline(&line, &cap, f) < 0) {
            send_response(f, 400, 0, nullptr);
            fclose(f);

            return;
        }
        size_t len = strlen(line);
        for (size_t i = 0; i < len; ++i) {
            if (line[i] == '\r' || line[i] == '\n') {
                line[i] = 0;
                break;
            }
        }

        if (strcmp(line, "GET MMP/1.0")) {
            send_response(f, 400, 0, nullptr);
            fclose(f);

            return;
        }

        string dimen;
        long long int x;
        bool x_set = false;
        long long int z;
        bool z_set = false;
        try {
            while (getline(&line, &cap, f) >= 0) {
                len = strlen(line);
                for (size_t i = 0; i < len; ++i) {
                    if (line[i] == '\r' || line[i] == '\n') {
                        line[i] = 0;

                        break;
                    }
                }

                if (line[0] == 0) break;

                char *pos_colon = strchr(line, ':');
                if (pos_colon == nullptr) {
                    send_response(f, 400, 0, 0);
                }
                *pos_colon = 0;

                string key = line;
                ++pos_colon;
                while (*pos_colon == ' ')
                    ++pos_colon;

                string value = pos_colon;
                if (key == "Coord-X") {
                    x_set = true;
                    x = stoll(value);
                } else if (key == "Coord-Z") {
                    z_set = true;
                    z = stoll(value);
                } else if (key == "Dimension") {
                    dimen = value;
                }
            }
        } catch (invalid_argument const &) {
            send_response(f, 400, 0, nullptr);
            fclose(f);

            return;
        } catch (out_of_range const &) {
            send_response(f, 400, 0, nullptr);
            fclose(f);

            return;
        }

        if (dimen != "overworld") {
            cerr << "error" << endl;
        }

        if (!x_set || !z_set ||
            (dimen != "overworld" && dimen != "nether" && dimen != "end")) {
            send_response(f, 400, 0, nullptr);
            fclose(f);

            return;
        }

        resolve_block(f, dimen, x, z);

        fclose(f);
    }

    void launch_server(string config_filename, bool daemon_mode) {
        parse_config(config_filename);

        if (daemon_mode) {
            if (daemon(0, 0) == -1) {
                cerr << "cannot run in daemon mode" << endl;

                exit(1);
            }
        }

        int ssock;

        sockaddr_un sa;
        memset(&sa, 0, sizeof(sa));
        sockaddr_un sa_peer;
        memset(&sa_peer, 0, sizeof(sa));

        socklen_t addr_len = sizeof(sockaddr_un);

        if ((ssock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("server");

            exit(1);
        }

        sa.sun_family = AF_UNIX;
        strcpy(sa.sun_path, "/tmp/mcmap.sock");

        if (bind(ssock, (sockaddr *)&sa, sizeof(sockaddr_un)) == -1) {
            goto fail;
        }

        if (listen(ssock, 4) == -1) {
            goto fail;
        }

        if (chmod("/tmp/mcmap.sock", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                         S_IROTH | S_IWOTH) == -1) {
            goto fail;
        }

        signal(SIGINT, &handle_interrupt);

        for (;;) {
            int fd;
            if ((fd = accept(ssock, (sockaddr *)&sa_peer, &addr_len)) > 0) {
                pid_t pid;
                if ((pid = fork()) == 0) {
                    close(ssock);

                    pid = fork();
                    if (pid == 0) {
                        handle_request(fd);

                        exit(0);
                    }

                    exit(0);
                }

                waitpid(pid, nullptr, 0);
            }
            close(fd);
        }

    fail:
        perror("server");
        close(ssock);
    }
} // namespace Server

#endif /* __unix__ */
