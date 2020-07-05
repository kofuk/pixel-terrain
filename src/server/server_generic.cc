#include <iostream>

#include "reader_generic.hh"
#include "request.hh"
#include "server.hh"
#include "server_generic.hh"
#include "writer_generic.hh"

namespace pixel_terrain::server {
    server_generic::server_generic() {}

    void server_generic::start_server() {
        for (;;) {
            if (std::cin.bad()) return;

            reader *r = new reader_generic();
            request *req = new request(r);
            writer *w = new writer_generic();

            handle_request(req, w);
            delete w;
            delete req;
            delete r;
        }
    }

} // namespace pixel_terrain::server
