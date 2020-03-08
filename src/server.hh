#ifndef SERVER_HH
#define SERVER_HH

#include <string>

using namespace std;

namespace server {
    void print_protocol_detail ();

    void launch_server (bool daemon_mode);
} // namespace server

#endif
