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

#ifdef _WIN32
#include <fstream>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace std;

namespace pixel_terrain {
    template <typename T> class file {
        bool mmapped = false;
        size_t data_len = 0;
        T *data;
#ifdef _WIN32
        string filename;
        bool write_mode = false;
#endif

    public:
        /* open specified file in read-only mode. */
        file(filesystem::path const &filename) {
#ifdef _WIN32
            ifstream ifs(filename, std::ios::binary);
            if (!ifs) {
                throw runtime_error("Unable to open file");
            }

            vector<T> d;
            T buf[1024];
            do {
                ifs.read(reinterpret_cast<char *>(buf), sizeof(T) * 1024);
                if (ifs.gcount() % sizeof(T) != 0) {
                    throw runtime_error("Corrupted data.");
                }
                d.insert(d.end(), buf, buf + ifs.gcount() / sizeof(T));
            } while (!ifs.eof());

            data = new T[d.size()];
            copy(d.begin(), d.end(), data);
            data_len = d.size() / sizeof(T);
#else
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
#endif
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

#ifdef _WIN32
            write_mode = writable;
            this->filename = filename.string();
            ifstream ifs(filename, std::ios::binary);
            if (!ifs) {
                if (writable) {
                    data = new T[data_len / sizeof(T)];
                } else {
                    throw runtime_error("Unable to open file");
                }
                return;
            }

            vector<T> d;
            T buf[1024];
            do {
                ifs.read(reinterpret_cast<char *>(buf), sizeof(T) * 1024);
                if (ifs.gcount() % sizeof(T) != 0) {
                    throw runtime_error("Corrupted data.");
                }
                d.insert(d.end(), buf, buf + ifs.gcount() / sizeof(T));
            } while (!ifs.eof());
            if (d.size() > data_len / sizeof(T)) {
                throw runtime_error("File too long");
            }

            data = new T[d.size()];
            copy(d.begin(), d.end(), data);
#else
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
#endif
        }

        ~file() {
#ifdef _WIN32
            if (write_mode) {
                ofstream ofs(filename);
                if (!ofs) {
                    delete[] data;
                    return;
                }

                ofs.write(reinterpret_cast<char *>(data), data_len);
                /* TODO: check if ofstream::write success. */
            }
            delete[] data;
#else
            if (mmapped) {
                munmap(data, data_len);
            } else {
                delete[] data;
            }
#endif
        }

        T &operator[](size_t off) { return data[off]; }

        size_t size() { return data_len; }

        T *get_raw_data() { return data; }
    };
} // namespace pixel_terrain

#endif
