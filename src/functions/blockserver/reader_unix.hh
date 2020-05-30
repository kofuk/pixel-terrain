#ifndef READER_UNIX_HH
#define READER_UNIX_HH

#include <cstddef>

#include "reader.hh"

using namespace std;

namespace mcmap::server {
    class reader_unix : public reader {
        int fd;

    public:
        reader_unix(int fd);

        long int fill_buffer(char *buf, size_t len, size_t off);
    };
} // namespace mcmap::server

#endif
