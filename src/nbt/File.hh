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

template <typename T> class File {
    bool mmapped = false;
    size_t data_len = 0;
    T *data;

public:
    File (filesystem::path const &filename) {
        struct stat statbuf;
        if (stat (filename.c_str (), &statbuf) != 0) {
            throw runtime_error (strerror (errno));
        }

        if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
            int fd = open (filename.c_str (), O_RDWR);
            if (fd < 0) {
                throw runtime_error (strerror (errno));
            }

            void *mem = mmap (nullptr, statbuf.st_size, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE, fd, 0);
            close (fd);
            if (mem == MAP_FAILED) {
                throw runtime_error (strerror (errno));
            }

            mmapped = true;
            data_len = statbuf.st_size;
            data = reinterpret_cast<T *> (mem);
        } else {
            ifstream strm (filename);
            if (!strm) {
                throw runtime_error (strerror (errno));
            }
            vector<T> content;
            char tmp[sizeof (T)];
            do {
                strm.read (tmp, sizeof (T));
                if (strm.gcount () != sizeof (T)) {
                    throw logic_error (strerror (errno));
                }
                content.push_back (*reinterpret_cast<T *> (tmp));
            } while (!strm.fail ());

            data = new T[content.size ()];
            copy (begin (content), end (content), data);
        }
    }

    File (filesystem::path const &filename, size_t nmemb)
        : mmapped (true), data_len (sizeof (T) * nmemb) {
        struct stat statbuf;
        if (stat (filename.c_str (), &statbuf) == 0) {
            if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
                throw logic_error ("file is not a regular file"s);
            }
        }

        int fd = open (filename.c_str (), O_RDWR | O_CREAT,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0) {
            throw runtime_error (strerror (errno));
        }
        if (posix_fallocate (fd, 0, data_len) != 0) {
            int errsave = errno;
            close (fd);
            throw runtime_error (strerror (errsave));
        }
        void *mem =
            mmap (nullptr, data_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED) {
            int errsave = errno;
            close (fd);
            throw runtime_error (strerror (errsave));
        }
        data = reinterpret_cast<T *> (mem);
    }

    ~File () {
        if (mmapped) {
            munmap (data, data_len);
        } else {
            delete[] data;
        }
    }

    T &operator[] (size_t off) { return data[off]; }

    size_t size () { return data_len; }

    T *get_raw_data () { return data; }
};

#endif
