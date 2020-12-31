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
#ifdef OS_WIN
#include "server/server_generic.hh"
#elif defined(OS_LINUX)
#include "server/server_unix_socket.hh"
#endif

namespace pixel_terrain::server {
    std::string overworld_dir;
    std::string nether_dir;
    std::string end_dir;

    namespace {
        constexpr int RESPONSE_INTERNAL_SERVER_ERROR = 500;
        constexpr int RESPONSE_OK = 200;
        constexpr int RESPONSE_NOT_FOUND = 404;
        constexpr int RESPONSE_BAD_REQUEST = 400;

        class response {
            int response_code = RESPONSE_INTERNAL_SERVER_ERROR;
            int altitude = 0;
            std::string block;
            bool response_wrote = false;

        public:
            response() = default;

            ~response() {
                ELOG("BUG: Response object discarded without writing its data");
            }

            response(response const &) = delete;
            auto operator=(response const &) -> response = delete;

            void write_to(writer *w) {
                response_wrote = true;

                w->write_data("MMP/1.0 ");
                w->write_data(response_code);
                w->write_data("\r\n\r\n");
                if (response_code != RESPONSE_OK) {
                    return;
                }

                w->write_data(R"({"altitude": )");
                w->write_data(altitude);
                w->write_data(R"(, "block": ")");
                w->write_data(block);
                w->write_data(R"("})");
                w->write_data("\r\n");
            }

            auto set_response_code(int code) -> response * {
                response_code = code;
                return this;
            }

            auto set_altitude(int altitide) -> response * {
                this->altitude = altitide;
                return this;
            }

            auto set_block(std::string const &block) -> response * {
                this->block = block;
                return this;
            }
        };

        inline auto positive_mod(int a, int b) -> int {
            int mod = a % b;
            if (mod < 0) {
                mod += b;
            }

            return mod;
        }

        void resolve_block(writer *w, std::string const &dimen, int x, int z) {
            int region_x;
            int region_z;

            constexpr int region_size = 512;
            constexpr int chunk_size = 16;

            if (x >= 0) {
                region_x = x / region_size;
            } else {
                region_x = (x + 1) / region_size - 1;
            }

            if (z >= 0) {
                region_z = z / region_size;
            } else {
                region_z = (z + 1) / region_size - 1;
            }

            int chunk_x = positive_mod(x, region_size) / chunk_size;
            int chunk_z = positive_mod(z, region_size) / chunk_size;
            int x_in_chunk = positive_mod(x, region_size) % chunk_size;
            int z_in_chunk = positive_mod(x, region_size) % chunk_size;

            std::filesystem::path region_file;

            if (dimen == "nether") {
                if (nether_dir.empty()) {
                    response()
                        .set_response_code(RESPONSE_NOT_FOUND)
                        ->write_to(w);

                    return;
                }

                region_file = nether_dir;
            } else if (dimen == "end") {
                if (end_dir.empty()) {
                    response()
                        .set_response_code(RESPONSE_NOT_FOUND)
                        ->write_to(w);

                    return;
                }

                region_file = end_dir;
            } else {
                if (overworld_dir.empty()) {
                    response()
                        .set_response_code(RESPONSE_NOT_FOUND)
                        ->write_to(w);

                    return;
                }

                region_file = overworld_dir;
            }

            region_file /= "r." + std::to_string(region_x) + "." +
                           std::to_string(region_z) + ".mca";
            if (!std::filesystem::exists(region_file)) {
                response().set_response_code(RESPONSE_NOT_FOUND)->write_to(w);

                return;
            }

            anvil::region *r;
            try {
                r = new anvil::region(region_file);
            } catch (...) {
                response().set_response_code(RESPONSE_NOT_FOUND)->write_to(w);
                return;
            }
            anvil::chunk *chunk = r->get_chunk(chunk_x, chunk_z);
            if (chunk == nullptr) {
                response().set_response_code(RESPONSE_NOT_FOUND)->write_to(w);

                return;
            }
            if (dimen == "nether") {
                constexpr int nether_max_y = 127;

                bool air_found = false;
                for (int y = nether_max_y; y >= 0; --y) {
                    std::string block =
                        chunk->get_block(x_in_chunk, y, z_in_chunk);
                    if (block == "minecraft:air" ||
                        block == "minecraft:cave_air" ||
                        block == "minecraft:void_air") {
                        if (y == 0) {
                            response()
                                .set_response_code(RESPONSE_NOT_FOUND)
                                ->write_to(w);

                            break;
                        }

                        air_found = true;

                        continue;
                    }

                    if (!air_found) {
                        if (y == 0) {
                            response()
                                .set_response_code(RESPONSE_NOT_FOUND)
                                ->write_to(w);

                            break;
                        }
                        continue;
                    }

                    if (block.empty()) {
                        response()
                            .set_response_code(RESPONSE_NOT_FOUND)
                            ->write_to(w);
                    } else {
                        response()
                            .set_response_code(RESPONSE_OK)
                            ->set_altitude(y)
                            ->set_block(block)
                            ->write_to(w);
                    }

                    break;
                }
            } else {
                constexpr int max_y = 255;

                for (int y = max_y; y >= 0; --y) {
                    std::string block =
                        chunk->get_block(x_in_chunk, y, z_in_chunk);
                    if (block == "minecraft:air" ||
                        block == "minecraft:cave_air" ||
                        block == "minecraft:void_air") {
                        if (y == 0) {
                            response()
                                .set_response_code(RESPONSE_NOT_FOUND)
                                ->write_to(w);

                            break;
                        }

                        continue;
                    }

                    if (block.empty()) {
                        response()
                            .set_response_code(RESPONSE_NOT_FOUND)
                            ->write_to(w);
                    } else {
                        response()
                            .set_response_code(RESPONSE_OK)
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
#ifdef OS_WIN
        if (daemon_mode) {
            std::cout << "Warning: Daemon mode has no effect on Windows.\n";
        }
        server_base *s = new server_generic();
#elif defined(OS_LINUX)
        server_base *s = new server_unix_socket(daemon_mode);
#endif
        s->start_server();
    }

    void handle_request(request *req, writer *w) {
        req->parse_all();

        if (req->get_method() != "GET" || req->get_protocol() != "MMP" ||
            req->get_version() != "1.0") {
            response().set_response_code(RESPONSE_BAD_REQUEST)->write_to(w);
            return;
        }

        std::string dimen = req->get_request_field("Dimension");
        if (!(dimen == "overworld" || dimen == "nether" || dimen == "end")) {
            response().set_response_code(RESPONSE_BAD_REQUEST)->write_to(w);
            return;
        }

        int x;
        int z;
        try {
            x = stoi(req->get_request_field("Coord-X"));
            z = stoi(req->get_request_field("Coord-Z"));
        } catch (std::invalid_argument const &) {
            response().set_response_code(RESPONSE_BAD_REQUEST)->write_to(w);
            return;
        } catch (std::out_of_range const &) {
            response().set_response_code(RESPONSE_BAD_REQUEST)->write_to(w);
            return;
        }

        resolve_block(w, dimen, x, z);
    }

} // namespace pixel_terrain::server
