#ifndef SERVER_HH
#define SERVER_HH

#include <string>

using namespace std;

namespace mcmap {
    namespace server {
        extern string overworld_dir;
        extern string nether_dir;
        extern string end_dir;

        void print_protocol_detail();

        void launch_server(bool daemon_mode);
    } // namespace server
} // namespace mcmap

#endif
