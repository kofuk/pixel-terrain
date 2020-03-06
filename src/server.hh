#ifndef SERVER_HH
#define SERVER_HH

#ifdef __unix__

#include <string>

using namespace std;

namespace server {
    void print_protocol_detail ();

    void launch_server (string config_filename, bool daemon_mode);
} // namespace server

#endif /* __unix__ */

#endif
