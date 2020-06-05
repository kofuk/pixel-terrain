#include <algorithm>
#include <unistd.h>

#include "reader_unix.hh"

namespace mcmap::server {
    reader_unix::reader_unix(int fd) : fd(fd) {}

    ssize_t reader_unix::fill_buffer(char *buf, size_t len, size_t off) {
        return read(fd, buf + off, len - off);
    }

} // namespace mcmap::server