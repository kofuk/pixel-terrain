// SPDX-License-Identifier: MIT

#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server/reader.hh"
#include "server/reader_unix.hh"
#include "server/request.hh"
#include "server/server.hh"
#include "server/server_unix_socket.hh"
#include "server/writer.hh"
#include "server/writer_unix.hh"
#include "utils/threaded_worker.hh"

namespace pixel_terrain::server {
    namespace {
        threaded_worker<int> *worker;
        std::mutex worker_mutex;

        void terminate_server() {
            ::unlink("/tmp/mcmap.sock");
            std::exit(0);
        }

        void handle_signals(sigset_t *sigs) {
            int sig;
            for (;;) {
                if (::sigwait(sigs, &sig) != 0) {
                    std::exit(1);
                }

                std::unique_lock<std::mutex> lock(worker_mutex);
                if (sig == SIGUSR1) {
                    if (worker != nullptr) {
                        worker->finish();
                        delete worker;
                    }
                    terminate_server();
                }
            }
        }

        void prepare_signel_handle_thread() {
            ::sigset_t sigs;
            ::sigemptyset(&sigs);
            ::sigaddset(&sigs, SIGPIPE);
            ::sigaddset(&sigs, SIGUSR1);
            ::sigaddset(&sigs, SIGINT);
            ::pthread_sigmask(SIG_BLOCK, &sigs, nullptr);

            std::thread t(&handle_signals, &sigs);
            t.detach();
        }

        void handle_request_unix(int const fd) {
            reader *r = new reader_unix(fd);
            auto *req = new request(r);
            writer *w = new writer_unix(fd);

            handle_request(req, w);
            delete w;
            delete req;
            delete r;
            ::close(fd);
        }

    } // namespace

    server_unix_socket::server_unix_socket(bool const daemon)
        : daemon_mode(daemon) {}

    void server_unix_socket::start_server() {
        if (daemon_mode) {
            if (::daemon(0, 0) == -1) {
                std::cerr << "cannot run in daemon mode\n";

                std::exit(1);
            }
        }

        prepare_signel_handle_thread();
        worker = new threaded_worker<int>(std::thread::hardware_concurrency(),
                                          &handle_request_unix);
        worker->start();

        int ssock;

        ::sockaddr_un sa;
        std::memset(&sa, 0, sizeof(sa));
        ::sockaddr_un sa_peer;
        std::memset(&sa_peer, 0, sizeof(sa));

        ::socklen_t addr_len = sizeof(sockaddr_un);

        if ((ssock = ::socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            std::perror("server");

            std::exit(1);
        }

        sa.sun_family = AF_UNIX;
        std::strcpy(sa.sun_path, "/tmp/mcmap.sock");

        if (::bind(ssock, reinterpret_cast<::sockaddr *>(&sa),
                   sizeof(::sockaddr_un)) == -1) {
            goto fail;
        }

        if (::listen(ssock, 4) == -1) {
            goto fail;
        }

        if (::chmod("/tmp/mcmap.sock", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                           S_IROTH | S_IWOTH) == -1) {
            goto fail;
        }

        for (;;) {
            int fd;
            if ((fd = ::accept(ssock, reinterpret_cast<::sockaddr *>(&sa_peer),
                               &addr_len)) > 0) {
                std::unique_lock<std::mutex> lock(worker_mutex);
                worker->queue_job(fd);
            }
        }

    fail:
        std::perror("server");
        ::close(ssock);
        if (worker != nullptr) {
            worker->finish();
        }
    }
} // namespace pixel_terrain::server
