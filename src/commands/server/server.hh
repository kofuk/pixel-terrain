#ifndef SERVER_HH
#define SERVER_HH

#include <string>

#include "request.hh"
#include "writer.hh"

namespace pixel_terrain::commands::server {
    extern std::string overworld_dir;
    extern std::string nether_dir;
    extern std::string end_dir;

    void handle_request(request *req, writer *w);
    int main(int argc, char **argv);
} // namespace pixel_terrain::commands::server

#endif
