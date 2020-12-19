// SPDX-License-Identifier: MIT

#ifndef SERVER_GENERIC_HH
#define SERVER_GENERIC_HH

#include "server/server_base.hh"

namespace pixel_terrain::server {
    class server_generic : public server_base {
        bool daemon_mode;

    public:
        server_generic();
        void start_server();
    };
} // namespace pixel_terrain::server

#endif
