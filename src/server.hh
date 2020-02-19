#ifndef SERVER_HH
#define SERVER_HH

#ifdef __unix__

#include <string>

using namespace std;

namespace Server {
    void print_protocol_detail();

    void launch_server(string config_filename);
}

#endif /* __unix__ */

#endif
