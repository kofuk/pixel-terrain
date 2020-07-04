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
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include <optlib/optlib.h>

#include "../../logger/logger.hh"
#include "../../nbt/region.hh"
#include "request.hh"
#include "server.hh"
#if _WIN32
#include "server_generic.hh"
#else
#include "server_unix_socket.hh"
#endif
#include "writer.hh"

namespace pixel_terrain::commands::server {
    std::string overworld_dir;
    std::string nether_dir;
    std::string end_dir;

    namespace {
        void print_help(optlib_parser *opt) {
            std::cout << "usage: mcmap server [OPTIONS...]" << std::endl;

            std::cout << "Possible options are:" << std::endl;
            optlib_print_help(opt, stderr);

            std::cout << "Help for block info server's protocol and config"
                      << std::endl
                      << std::endl;

            std::cout << "PROTOCOL:" << std::endl;
            std::cout << " example request and response;" << std::endl;
            std::cout << " `>' means request and `<' means response"
                      << std::endl;
            std::cout << "  >GET MMP/1.0" << std::endl;
            std::cout << "  >Dimension: overworld" << std::endl;
            std::cout << "  >Coord-X: 1" << std::endl;
            std::cout << "  >Coord-Z: 1" << std::endl;
            std::cout << "  >" << std::endl;
            std::cout << "  <MMP/1.0 200" << std::endl;
            std::cout << "  <" << std::endl;
            std::cout << R"(  <{"altitude": 63, "block": "minecraft:dirt"})"
                      << std::endl
                      << std::endl;
            std::cout << " Response Codes:" << std::endl;
            std::cout << "  200  OK. The request is handled properly and"
                      << std::endl
                      << "       altitude returned" << std::endl;
            std::cout << "  400  Bad Request. Probably request parse error"
                      << std::endl;
            std::cout << "  404  Out of Range. No chunk existing on specified "
                         "corrdinate"
                      << std::endl;
            std::cout << "  500  Internal Server Error. Serverside error"
                      << std::endl;
        }

        class response {
            int response_code = 500;
            int altitude = 0;
            std::string block;
            bool response_wrote = false;

            response(response const &) = delete;
            response operator=(response const &) = delete;

        public:
            response() {}

            ~response() {
                if (!response_wrote) {
                    logger::e("BUG: Response object discarded without writing "
                              "its data");
                }
            }

            void write_to(writer *w) {
                response_wrote = true;

                w->write_data("MMP/1.0 ");
                w->write_data(response_code);
                w->write_data("\r\n\r\n");
                if (response_code != 200) return;

                w->write_data(R"({"altitude": )");
                w->write_data(altitude);
                w->write_data(R"(, "block": ")");
                w->write_data(block);
                w->write_data(R"("})");
                w->write_data("\r\n");
            }

            response *set_response_code(int code) {
                response_code = code;
                return this;
            }

            response *set_altitude(int altitide) {
                this->altitude = altitide;
                return this;
            }

            response *set_block(std::string const &block) {
                this->block = block;
                return this;
            }
        };

        inline int positive_mod(int a, int b) {
            int mod = a % b;
            if (mod < 0) mod += b;

            return mod;
        }

        void resolve_block(writer *w, std::string dimen, long long int x,
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

            std::filesystem::path region_file;

            if (dimen == "nether") {
                if (nether_dir.empty()) {
                    response().set_response_code(404)->write_to(w);

                    return;
                }

                region_file = nether_dir;
            } else if (dimen == "end") {
                if (end_dir.empty()) {
                    response().set_response_code(404)->write_to(w);

                    return;
                }

                region_file = end_dir;
            } else {
                if (overworld_dir.empty()) {
                    response().set_response_code(404)->write_to(w);

                    return;
                }

                region_file = overworld_dir;
            }

            region_file /= "r." + std::to_string(region_x) + "." +
                           std::to_string(region_z) + ".mca";
            if (!std::filesystem::exists(region_file)) {
                response().set_response_code(404)->write_to(w);

                return;
            }

            anvil::region *r = new anvil::region(region_file.string());
            anvil::chunk *chunk = r->get_chunk(chunk_x, chunk_z);
            if (chunk == nullptr) {
                response().set_response_code(404)->write_to(w);

                return;
            }
            if (dimen == "nether") {
                bool air_found = false;
                for (int y = 127; y >= 0; --y) {
                    std::string block =
                        chunk->get_block(x_in_chunk, y, z_in_chunk);
                    if (block == "minecraft:air" ||
                        block == "minecraft:cave_air" ||
                        block == "minecraft:void_air") {
                        if (y == 0) {
                            response().set_response_code(404)->write_to(w);

                            break;
                        }

                        air_found = true;

                        continue;
                    }

                    if (!air_found) {
                        if (y == 0) {
                            response().set_response_code(404)->write_to(w);

                            break;
                        }
                        continue;
                    }

                    if (block.empty()) {
                        response().set_response_code(404)->write_to(w);
                    } else {
                        response()
                            .set_response_code(200)
                            ->set_altitude(y)
                            ->set_block(block)
                            ->write_to(w);
                    }

                    break;
                }
            } else {
                for (int y = 255; y >= 0; --y) {
                    std::string block =
                        chunk->get_block(x_in_chunk, y, z_in_chunk);
                    if (block == "minecraft:air" ||
                        block == "minecraft:cave_air" ||
                        block == "minecraft:void_air") {
                        if (y == 0) {
                            response().set_response_code(404)->write_to(w);

                            break;
                        }

                        continue;
                    }

                    if (block.empty()) {
                        response().set_response_code(404)->write_to(w);
                    } else {
                        response()
                            .set_response_code(200)
                            ->set_altitude(y)
                            ->set_block(block)
                            ->write_to(w);
                    }

                    break;
                }
            }

            delete chunk;
            delete r;
        }

        void launch_server(bool daemon_mode) {
#ifdef _WIN32
            if (daemon_mode) {
                cout << "Warning: Daemon mode has no effect on Windows."
                     << endl;
            }
            server_base *s = new server_generic();
#else
            server_base *s = new server_unix_socket(daemon_mode);
#endif
            s->start_server();
        }
    } // namespace

    void handle_request(request *req, writer *w) {
        req->parse_all();

        if (req->get_method() != "GET" || req->get_protocol() != "MMP" ||
            req->get_version() != "1.0") {
            response().set_response_code(400)->write_to(w);
            return;
        }

        std::string dimen = req->get_request_field("Dimension");
        if (!(dimen == "overworld" || dimen == "nether" || dimen == "end")) {
            response().set_response_code(400)->write_to(w);
            return;
        }

        long long int x;
        long long int z;
        try {
            x = stoi(req->get_request_field("Coord-X"));
            z = stoi(req->get_request_field("Coord-Z"));
        } catch (std::invalid_argument const &) {
            response().set_response_code(400)->write_to(w);
            return;
        } catch (std::out_of_range const &) {
            response().set_response_code(400)->write_to(w);
            return;
        }

        resolve_block(w, dimen, x, z);
    }

    int main(int argc, char **argv) {
        bool daemon_mode = false;

        optlib_parser *parser = optlib_parser_new(argc, argv);
        optlib_parser_add_option(parser, "daemon", 'd', false,
                                 "Run block server as daemon.");
        optlib_parser_add_option(parser, "overworld", 'o', true,
                                 "Specify data directory for the overworld.");
        optlib_parser_add_option(parser, "nether", 'n', true,
                                 "Specify data directory for the nether.");
        optlib_parser_add_option(parser, "end", 'e', true,
                                 "Specify data directory for the end.");
        optlib_parser_add_option(parser, "help", 'e', false,
                                 "Print this message and exit.");

        for (;;) {
            optlib_option *opt = optlib_next(parser);

            if (parser->finished) {
                break;
            }
            if (!opt) {
                print_help(parser);
                exit(1);
            }

            switch (opt->short_opt) {
            case 'd':
                daemon_mode = true;
                break;

            case 'h':
                print_help(parser);
                exit(0);

            case 'o':
                overworld_dir = opt->argval;
                break;

            case 'n':
                nether_dir = opt->argval;
                break;

            case 'e':
                end_dir = opt->argval;
                break;
            }
        }

        if (argc - parser->optind == 0) {
            optlib_parser_free(parser);
            launch_server(daemon_mode);
        } else {
            print_help(parser);
            optlib_parser_free(parser);
            exit(1);
        }

        return 0;
    }
} // namespace pixel_terrain::commands::server
