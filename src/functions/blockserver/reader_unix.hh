#ifndef READER_UNIX_HH
#define READER_UNIX_HH

#include <cstddef>

using namespace std;

namespace mcmap::server {
    class reader_unix {
        int fd;

    public:
        reader_unix(int fd);

        ssize_t fill_buffer(char *buf, size_t len, size_t off);
    };
} // namespace mcmap::server

#endif
