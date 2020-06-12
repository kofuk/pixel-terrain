/* Wrapper of memory-mapped file. */

#ifndef FILE_HH
#define FILE_HH

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

namespace pixel_terrain {
    template <typename T> class file {
        bool mmapped = false;
        size_t data_len = 0;
        T *data;

    public:
        /* open specified file in read-only mode. */
        file(filesystem::path const &filename) {
            int fd = open(filename.c_str(), O_RDONLY);
            if (fd < 0) {
                throw runtime_error(strerror(errno));
            }
            struct stat statbuf;
            if (fstat(fd, &statbuf) != 0) {
                throw runtime_error(strerror(errno));
            }

            if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
                void *mem = mmap(nullptr, statbuf.st_size,
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
                close(fd);
                if (mem == MAP_FAILED) {
                    throw runtime_error(strerror(errno));
                }

                mmapped = true;
                data_len = statbuf.st_size;
                data = reinterpret_cast<T *>(mem);
            } else {
                vector<T> content;
                char tmp[sizeof(T)];
                ssize_t n_read;
                while ((n_read = read(fd, tmp, sizeof(T))) > 0) {
                    if (n_read != sizeof(T)) {
                        close(fd);
                        throw runtime_error(strerror(errno));
                    }
                    content.push_back(*reinterpret_cast<T *>(tmp));
                }
                if (n_read < 0) {
                    throw runtime_error("file size % sizeof(T) != 0");
                }

                data = new T[content.size()];
                copy(begin(content), end(content), data);
            }
        }

        file(filesystem::path const &filename, size_t nmemb, string const &mode)
            : mmapped(true), data_len(sizeof(T) * nmemb) {
            if (!mode.size()) {
                throw invalid_argument("mode cannot be empty");
            }
            bool readable = false;
            bool writable = false;
            if (mode.size() > 1 && mode[1] == '+') {
                readable = true;
                writable = true;
            } else {
                if (mode[1] == 'r') {
                    readable = true;
                } else if (mode[1] == 'w') {
                    writable = true;
                } else {
                    throw invalid_argument("invalid mode");
                }
            }

            int omode = 0;
            if (readable && writable) {
                omode = O_RDWR;
            } else if (readable) {
                omode = O_RDONLY;
            } else if (writable) {
                omode = O_WRONLY;
            }
            omode |= O_CREAT;
            int fd = open(filename.c_str(), omode,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd < 0) {
                throw runtime_error(strerror(errno));
            }

            struct stat statbuf;
            if (fstat(fd, &statbuf) == 0) {
                if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
                    throw logic_error("file is not a regular file"s);
                }
            }

            if (posix_fallocate(fd, 0, data_len) != 0) {
                int errsave = errno;
                close(fd);
                throw runtime_error(strerror(errsave));
            }
            void *mem = mmap(nullptr, data_len, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, 0);
            if (mem == MAP_FAILED) {
                int errsave = errno;
                close(fd);
                throw runtime_error(strerror(errsave));
            }
            data = reinterpret_cast<T *>(mem);
        }

        ~file() {
            if (mmapped) {
                munmap(data, data_len);
            } else {
                delete[] data;
            }
        }

        T &operator[](size_t off) { return data[off]; }

        size_t size() { return data_len; }

        T *get_raw_data() { return data; }
    };
} // namespace pixel_terrain

#endif
