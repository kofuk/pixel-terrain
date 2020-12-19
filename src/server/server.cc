// SPDX-License-Identifier: MIT

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

#include "logger/logger.hh"
#include "nbt/region.hh"
#include "server/request.hh"
#include "server/server.hh"
#include "server/writer.hh"
#if _WIN32
#include "server/server_generic.hh"
#else
#include "server/server_unix_socket.hh"
#endif

namespace pixel_terrain::server {
    std::string overworld_dir;
    std::string nether_dir;
    std::string end_dir;

    namespace {

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
                logger::L(logger::ERROR,
                          "BUG: Response object discarded without writing "
                          "its data");
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

            anvil::region *r = new anvil::region(region_file);
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

    } // namespace

    void launch_server(bool daemon_mode) {
#ifdef _WIN32
        if (daemon_mode) {
            std::cout << "Warning: Daemon mode has no effect on Windows.\n";
        }
        server_base *s = new server_generic();
#else
        server_base *s = new server_unix_socket(daemon_mode);
#endif
        s->start_server();
    }

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

} // namespace pixel_terrain::server
