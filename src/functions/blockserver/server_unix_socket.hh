#ifndef SERVER_UNIX_SOCKET_HH
#define SERVER_UNIX_SOCKET_HH

#include "server_base.hh"

namespace mcmap::server {
    class server_unix_socket : public server_base {
        bool daemon_mode;
    public:
        server_unix_socket(bool const daemon);
        void start_server();
    };
} // namespace mcmap::server

#endif
