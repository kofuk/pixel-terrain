/* Server to provide block id and coordinate server. */

#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../logger/logger.hh"
#include "../../nbt/Region.hh"
#include "../threaded_worker.hh"
#include "server.hh"

using namespace std;

namespace server {
    void print_protocol_detail () {
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
        cout << "  500  Internal Server Error. Serverside error" << endl;
    }

    string overworld_dir;
    string nether_dir;
    string end_dir;

    namespace {
        ThreadedWorker<int> *worker;
        mutex worker_mutex;

        class Response {
            int response_code = 500;
            int altitude = 0;
            string block;
            bool response_wrote = false;

            Response (Response const &) = delete;
            Response operator= (Response const &) = delete;

        public:
            Response () {}

            ~Response () {
                if (!response_wrote) {
                    logger::e (
                        "BUG: Response object discarded without writing its data"s);
                }
            }

            void write_to (FILE *out) {
                response_wrote = true;

                fputs ("MMP/1.0 ", out);
                fputs (to_string (response_code).c_str (), out);
                fputs ("\r\n\r\n", out);
                if (response_code != 200) return;

                fprintf (out,
                         R"({"altitide": %d, "block": "%s"})"
                         "\r\n",
                         altitude, block.c_str ());
            }

            Response *set_response_code (int code) {
                response_code = code;
                return this;
            }

            Response *set_altitude (int altitide) {
                this->altitude = altitide;
                return this;
            }

            Response *set_block (string const &block) {
                if (block.find (':') == string::npos) {
                    this->block = "minecraft:"s + block;
                } else {
                    this->block = block;
                }
                return this;
            }
        };

        void terminate_server () {
            unlink ("/tmp/mcmap.sock");
            exit (0);
        }

        inline int positive_mod (int a, int b) {
            int mod = a % b;
            if (mod < 0) mod += b;

            return mod;
        }

        void resolve_block (FILE *f, string dimen, long long int x,
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

            int chunk_x = positive_mod (x, 512) / 16;
            int chunk_z = positive_mod (z, 512) / 16;
            int x_in_chunk = positive_mod (x, 512) % 16;
            int z_in_chunk = positive_mod (x, 512) % 16;

            filesystem::path region_file;

            if (dimen == "nether"s) {
                if (nether_dir.empty ()) {
                    Response ().set_response_code (404)->write_to (f);

                    return;
                }

                region_file = nether_dir;
            } else if (dimen == "end"s) {
                if (end_dir.empty ()) {
                    Response ().set_response_code (404)->write_to (f);

                    return;
                }

                region_file = end_dir;
            } else {
                if (overworld_dir.empty ()) {
                    Response ().set_response_code (404)->write_to (f);

                    return;
                }

                region_file = overworld_dir;
            }

            region_file /= "r."s + to_string (region_x) + "."s +
                           to_string (region_z) + ".mca"s;
            if (!filesystem::exists (region_file)) {
                Response ().set_response_code (404)->write_to (f);

                return;
            }

            anvil::Region *r = new anvil::Region (region_file.string ());
            anvil::Chunk *chunk = r->get_chunk (chunk_x, chunk_z);
            if (chunk == nullptr) {
                Response ().set_response_code (404)->write_to (f);

                return;
            }
            if (dimen == "nether"s) {
                bool air_found = false;
                for (int y = 127; y >= 0; --y) {
                    string block = chunk->get_block (x_in_chunk, y, z_in_chunk);
                    if (block == "minecraft:air"s || block == "minecraft:cave_air"s ||
                        block == "minecraft:void_air"s) {
                        if (y == 0) {
                            Response ().set_response_code (404)->write_to (f);

                            break;
                        }

                        air_found = true;

                        continue;
                    }

                    if (!air_found) {
                        if (y == 0) {
                            Response ().set_response_code (404)->write_to (f);

                            break;
                        }
                        continue;
                    }

                    if (block.empty ()) {
                        Response ().set_response_code (404)->write_to (f);
                    } else {
                        Response ()
                            .set_response_code (200)
                            ->set_altitude (y)
                            ->set_block (block)
                            ->write_to (f);
                    }

                    break;
                }
            } else {
                for (int y = 255; y >= 0; --y) {
                    string block = chunk->get_block (x_in_chunk, y, z_in_chunk);
                    if (block == "minecraft:air"s || block == "minecraft:cave_air"s ||
                        block == "minecraft:void_air"s) {
                        if (y == 0) {
                            Response ().set_response_code (404)->write_to (f);

                            break;
                        }

                        continue;
                    }

                    if (block.empty ()) {
                        Response ().set_response_code (404)->write_to (f);
                    } else {
                        Response ()
                            .set_response_code (200)
                            ->set_altitude (y)
                            ->set_block (block)
                            ->write_to (f);
                    }

                    break;
                }
            }

            delete chunk;
            delete r;
        }

        void handle_request (int fd) {
            FILE *f = fdopen (fd, "rb+");
            if (f == NULL) {
                close (fd);

                return;
            }

            char *line = nullptr;
            size_t cap = 0;
            if (getline (&line, &cap, f) < 0) {
                Response ().set_response_code (400)->write_to (f);
                fclose (f);

                return;
            }
            size_t len = strlen (line);
            for (size_t i = 0; i < len; ++i) {
                if (line[i] == '\r' || line[i] == '\n') {
                    line[i] = 0;
                    break;
                }
            }

            if (strcmp (line, "GET MMP/1.0")) {
                Response ().set_response_code (400)->write_to (f);
                fclose (f);

                return;
            }

            string dimen;
            long long int x;
            bool x_set = false;
            long long int z;
            bool z_set = false;
            try {
                while (getline (&line, &cap, f) >= 0) {
                    len = strlen (line);
                    for (size_t i = 0; i < len; ++i) {
                        if (line[i] == '\r' || line[i] == '\n') {
                            line[i] = 0;

                            break;
                        }
                    }

                    if (line[0] == 0) break;

                    char *pos_colon = strchr (line, ':');
                    if (pos_colon == nullptr) {
                        Response ().set_response_code (400)->write_to (f);
                    }
                    *pos_colon = 0;

                    string key = line;
                    ++pos_colon;
                    while (*pos_colon == ' ')
                        ++pos_colon;

                    string value = pos_colon;
                    if (key == "Coord-X"s) {
                        x_set = true;
                        x = stoll (value);
                    } else if (key == "Coord-Z"s) {
                        z_set = true;
                        z = stoll (value);
                    } else if (key == "Dimension"s) {
                        dimen = value;
                    }
                }
            } catch (invalid_argument const &) {
                Response ().set_response_code (400)->write_to (f);
                fclose (f);

                return;
            } catch (out_of_range const &) {
                Response ().set_response_code (400)->write_to (f);
                fclose (f);

                return;
            }

            if (dimen != "overworld"s) {
                cerr << "error" << endl;
            }

            if (!x_set || !z_set ||
                (dimen != "overworld"s && dimen != "nether"s &&
                 dimen != "end"s)) {
                Response ().set_response_code (400)->write_to (f);
                fclose (f);

                return;
            }

            resolve_block (f, dimen, x, z);

            fclose (f);
        }

        void handle_signals (sigset_t *sigs) {
            int sig;
            for (;;) {
                if (sigwait (sigs, &sig) != 0) {
                    exit (1);
                }

                unique_lock<mutex> lock (worker_mutex);
                if (sig == SIGUSR1) {
                    if (worker != nullptr) {
                        worker->finish ();
                        worker->wait ();
                        delete worker;
                    }
                    terminate_server ();
                }
            }
        }

        void prepare_signel_handle_thread () {
            sigset_t sigs;
            sigemptyset (&sigs);
            sigaddset (&sigs, SIGPIPE);
            sigaddset (&sigs, SIGUSR1);
            sigaddset (&sigs, SIGINT);
            pthread_sigmask (SIG_BLOCK, &sigs, nullptr);

            thread t (&handle_signals, &sigs);
            t.detach ();
        }
    } // namespace

    void launch_server (bool daemon_mode) {
        if (daemon_mode) {
            if (daemon (0, 0) == -1) {
                cerr << "cannot run in daemon mode" << endl;

                exit (1);
            }
        }

        prepare_signel_handle_thread ();
        worker = new ThreadedWorker<int> (thread::hardware_concurrency (),
                                          &handle_request);
        worker->start ();

        int ssock;

        sockaddr_un sa;
        memset (&sa, 0, sizeof (sa));
        sockaddr_un sa_peer;
        memset (&sa_peer, 0, sizeof (sa));

        socklen_t addr_len = sizeof (sockaddr_un);

        if ((ssock = socket (AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror ("server");

            exit (1);
        }

        sa.sun_family = AF_UNIX;
        strcpy (sa.sun_path, "/tmp/mcmap.sock");

        if (bind (ssock, reinterpret_cast<sockaddr *> (&sa),
                  sizeof (sockaddr_un)) == -1) {
            goto fail;
        }

        if (listen (ssock, 4) == -1) {
            goto fail;
        }

        if (chmod ("/tmp/mcmap.sock", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                          S_IROTH | S_IWOTH) == -1) {
            goto fail;
        }

        for (;;) {
            int fd;
            if ((fd = accept (ssock, reinterpret_cast<sockaddr *> (&sa_peer),
                              &addr_len)) > 0) {
                unique_lock<mutex> lock (worker_mutex);
                worker->queue_job (fd);
            }
        }

    fail:
        perror ("server");
        close (ssock);
        if (worker != nullptr) {
            worker->finish ();
            worker->wait ();
        }
    }
} // namespace server
