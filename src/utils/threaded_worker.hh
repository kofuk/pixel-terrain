#ifndef WORKER_THREAD_HH
#define WORKER_THREAD_HH

/* Generic implementation of threaded worker. */

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#endif

#include "../utils/logger/logger.hh"

/* XXX */
/*
 * This file contains workaround to force the code to work.
 * `std::condition_variable::wait_for()` should be replaced by
 * `std::condition_variable::wait()`.
 */

namespace pixel_terrain {
    namespace {
        enum class signal_type { JOB, TERMINATE };

        template <typename T> struct worker_signal {
            signal_type type;
            T data;

            worker_signal(signal_type type, T data) : type(type), data(data) {}

            worker_signal(signal_type type) : type(type) {}
        };
    } // namespace

    template <typename T> class threaded_worker {
        std::queue<worker_signal<T> *> job_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cond;
        std::mutex cond_mutex;
        std::condition_variable queue_cap_cond;
        std::mutex queue_cap_cond_mutex;
        int jobs;
        std::function<void(T)> handler;
        std::vector<std::thread *> threads;

        bool finished = false;
        bool waited = false;

        worker_signal<T> *fetch_job() {
            for (;;) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (job_queue.empty()) {
                    lock.unlock();

                    /* FIXME: if job is queued on this line,
                     * std::condition_variable::wait will never return, without
                     * timeout. */
#ifndef NDEBUG
                    if (!job_queue.empty()) {
                        std::cerr << "BUG: race" << std::endl;
                    }
#endif

                    std::unique_lock<std::mutex> lock(cond_mutex);
                    queue_cond.wait_for(lock, std::chrono::milliseconds(500));

                    continue;
                }

                worker_signal<T> *job = job_queue.front();
                job_queue.pop();
                queue_cap_cond.notify_one();
                return job;
            }
        }

        void run_worker() {
            for (;;) {
                worker_signal<T> *job = fetch_job();
                if (job->type == signal_type::TERMINATE) {
                    delete job;
                    break;
                }

                handler(job->data);
                delete job;
            }
        }

        void internal_queue_job(worker_signal<T> *sig) {
            for (;;) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (job_queue.size() < static_cast<size_t>(jobs * 2)) {
                    job_queue.push(sig);
                    queue_cond.notify_one();
                    break;
                } else {
                    lock.unlock();
                    std::unique_lock<std::mutex> lock(queue_cap_cond_mutex);
                    queue_cap_cond.wait_for(lock,
                                            std::chrono::milliseconds(500));
                }
            }
        }

        threaded_worker(threaded_worker<T> const &) = delete;
        threaded_worker<T> operator=(threaded_worker<T> const &) = delete;

    public:
        threaded_worker(int jobs, std::function<void(T)> handler)
            : jobs(jobs), handler(handler) {
            if (jobs < 1) {
                throw std::logic_error("JOBS must be greater than 0");
            }

            threads.reserve(jobs);
        }

        ~threaded_worker() {
            if (!finished) {
                finish();
            }
            if (!waited) {
                wait();
            }
        }

        void start() {
#ifndef NDEBUG
            if (threads.size()) {
                throw std::logic_error("Tried to start worker already started");
            }
#endif
            try {
                for (int i = 0; i < jobs; ++i) {
                    threads.push_back(
                        new std::thread(&threaded_worker::run_worker, this));
                }
            } catch (std::system_error const &e) {
#ifndef NDEBUG
                std::cerr << "some thread(s) cannot be launched\n";
#endif
            }
        }

        void queue_job(T item) {
#ifndef NDEBUG
            if (finished) {
                throw std::logic_error("Tried to queue job on finished worker");
            }
            if (!threads.size()) {
                std::cerr << "worker is not started\n";
            }
#endif
            worker_signal<T> *sig =
                new worker_signal<T>(signal_type::JOB, item);
            internal_queue_job(sig);
        }

        void wait() {
#ifndef NDEBUG
            if (waited) {
                throw std::logic_error("The worker already waited");
            }
            if (!threads.size()) {
                throw std::logic_error("worker is not started");
            }
#endif
            for (std::thread *t : threads) {
                if (t->joinable()) {
                    t->join();
                }
            }

            waited = true;
        }

        void finish() {
#ifndef NDEBUG
            if (finished) {
                throw std::logic_error("Tried to finish on finished worker");
            }
            if (!threads.size()) {
                throw std::logic_error("worker is not started");
            }
#endif
            for (int i = 0; i < jobs; ++i) {
                worker_signal<T> *sig =
                    new worker_signal<T>(signal_type::TERMINATE);
                internal_queue_job(sig);
            }

            finished = true;
        }
    };
} // namespace pixel_terrain

#endif
