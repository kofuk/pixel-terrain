#ifndef SERVER_BASE_HH
#define SERVER_BASE_HH

namespace pixel_terrain::server {
    class server_base {
    public:
        virtual ~server_base() = default;
        virtual void start_server() = 0;
    };
} // namespace pixel_terrain::server

#endif
