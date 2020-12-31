// SPDX-License-Identifier: MIT

#ifndef SERVER_UNIX_SOCKET_HH
#define SERVER_UNIX_SOCKET_HH

#include "server/server_base.hh"

namespace pixel_terrain::server {
    class server_unix_socket : public server_base {
        bool daemon_mode;

    public:
        server_unix_socket(bool daemon);
        void start_server() override;
    };
} // namespace pixel_terrain::server

#endif
