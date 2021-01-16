// SPDX-License-Identifier: MIT

/* Wrapper of memory-mapped file. */

#ifndef FILE_HH
#define FILE_HH

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

#ifdef OS_WIN
#include <fstream>
#elif defined(OS_LINUX)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "utils/path_hack.hh"

namespace pixel_terrain {
    template <typename T>
    class file {
        bool mmapped = false;
        std::size_t nmemb = 0;
        T *data;
#ifdef OS_WIN
        path_string filename;
        bool write_mode = false;
#endif

    public:
        /* open specified file in read-only mode. */
        file(std::filesystem::path const &filename) {
#ifdef OS_WIN
            std::ifstream ifs(filename, std::ios::binary);
            if (!ifs) {
                throw std::runtime_error("Unable to open file");
            }

            std::vector<T> d;
            T buf[1024];
            do {
                ifs.read(reinterpret_cast<std::uint8_t *>(buf),
                         sizeof(T) * 1024);
                if (ifs.gcount() % sizeof(T) != 0) {
                    throw std::runtime_error("Corrupted data.");
                }
                d.insert(d.end(), buf, buf + ifs.gcount() / sizeof(T));
            } while (!ifs.eof());

            data = new T[d.size()];
            copy(d.cbegin(), d.cend(), data);
            nmemb = d.size() / sizeof(T);
#elif defined(OS_LINUX)
            int fd = ::open(filename.c_str(), O_RDONLY);
            if (fd < 0) {
                throw std::runtime_error(strerror(errno));
            }
            struct ::stat statbuf;
            if (fstat(fd, &statbuf) != 0) {
                ::close(fd);
                throw std::runtime_error(strerror(errno));
            }

            if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
                void *mem = ::mmap(nullptr, statbuf.st_size,
                                   PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
                ::close(fd);
                if (mem == MAP_FAILED) {
                    throw std::runtime_error(strerror(errno));
                }

                mmapped = true;
                nmemb = statbuf.st_size / sizeof(T);
                data = reinterpret_cast<T *>(mem);
            } else {
                std::vector<T> content;
                std::array<std::uint8_t, sizeof(T)> buf;
                ::ssize_t n_read;
                while ((n_read = ::read(fd, buf.data(), sizeof(T))) > 0) {
                    if (n_read != sizeof(T)) {
                        ::close(fd);
                        throw std::runtime_error(strerror(errno));
                    }
                    T tmp;
                    std::memcpy(&tmp, buf.data(), buf.size());
                    content.push_back(tmp);
                }
                if (n_read < 0) {
                    throw std::runtime_error("file size % sizeof(T) != 0");
                }

                data = new T[content.size()];
                std::copy(content.cbegin(), content.cend(), data);
            }
#endif
        }

        file(std::filesystem::path const &filename, std::size_t nmemb,
             std::string const &mode)
            : mmapped(true), nmemb(nmemb) {
            if (mode.empty()) {
                throw std::invalid_argument("mode cannot be empty");
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
                    throw std::invalid_argument("invalid mode");
                }
            }

#ifdef OS_WIN
            write_mode = writable;
            this->filename = filename;
            std::ifstream ifs(filename, std::ios::binary);
            if (!ifs) {
                if (writable) {
                    data = new T[data_len / sizeof(T)];
                } else {
                    throw std::runtime_error("Unable to open file");
                }
                return;
            }

            std::vector<T> d;
            T buf[1024];
            do {
                ifs.read(reinterpret_cast<char *>(buf), sizeof(T) * 1024);
                if (ifs.gcount() % sizeof(T) != 0) {
                    throw std::runtime_error("Corrupted data.");
                }
                d.insert(d.end(), buf, buf + ifs.gcount() / sizeof(T));
            } while (!ifs.eof());
            if (d.size() > data_len / sizeof(T)) {
                throw std::runtime_error("File too long");
            }

            data = new T[d.size()];
            std::copy(d.cbegin(), d.cend(), data);
#elif defined(OS_LINUX)
            int omode = 0;
            if (readable && writable) {
                omode = O_RDWR;
            } else if (readable) {
                omode = O_RDONLY;
            } else if (writable) {
                omode = O_WRONLY;
            }
            omode |= O_CREAT;
            int fd = ::open(filename.c_str(), omode,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd < 0) {
                throw std::runtime_error(strerror(errno));
            }

            struct ::stat statbuf;
            if (::fstat(fd, &statbuf) == 0) {
                if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
                    ::close(fd);
                    throw std::logic_error("file is not a regular file");
                }
            }

            if (::posix_fallocate(fd, 0, nmemb * sizeof(T)) != 0) {
                int errsave = errno;
                ::close(fd);
                throw std::runtime_error(strerror(errsave));
            }
            void *mem = ::mmap(nullptr, nmemb * sizeof(T),
                               PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            ::close(fd);
            if (mem == MAP_FAILED) {
                int errsave = errno;
                throw std::runtime_error(strerror(errsave));
            }
            data = reinterpret_cast<T *>(mem);
#endif
        }

        ~file() {
#ifdef OS_WIN
            if (write_mode) {
                std::ofstream ofs(filename);
                if (!ofs) {
                    delete[] data;
                    return;
                }

                ofs.write(reinterpret_cast<std::uint8_t *>(data),
                          sizeof(T) * nmemb);
                /* TODO: check if ofstream::write success. */
            }
            delete[] data;
#elif defined(OS_LINUX)
            if (mmapped) {
                ::munmap(data, sizeof(T) * nmemb);
            } else {
                delete[] data;
            }
#endif
        }

        [[nodiscard]] auto operator[](size_t off) -> T & { return data[off]; }

        [[nodiscard]] auto size() const -> size_t { return nmemb; }

        [[nodiscard]] auto get_raw_data() -> T * { return data; }
    };
} // namespace pixel_terrain

#endif
