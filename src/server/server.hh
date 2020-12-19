// SPDX-License-Identifier: MIT

#ifndef SERVER_HH
#define SERVER_HH

#include <string>

#include "server/request.hh"
#include "server/writer.hh"

namespace pixel_terrain::server {
    extern std::string overworld_dir;
    extern std::string nether_dir;
    extern std::string end_dir;

    void handle_request(request *req, writer *w);
    void launch_server(bool daemon_mode);
} // namespace pixel_terrain::server

#endif
