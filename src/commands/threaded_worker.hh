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
#include <system_error>
#include <thread>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#endif

#include "../logger/logger.hh"

using namespace std;

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
        queue<worker_signal<T> *> job_queue;
        mutex queue_mutex;
        condition_variable queue_cond;
        mutex cond_mutex;
        condition_variable queue_cap_cond;
        mutex queue_cap_cond_mutex;
        int jobs;
        function<void(T)> handler;
        vector<thread *> threads;

        bool finished = false;
        bool waited = false;

        worker_signal<T> *fetch_job() {
            for (;;) {
                unique_lock<mutex> lock(queue_mutex);
                if (job_queue.empty()) {
                    lock.unlock();

                    /* FIXME: if job is queued on this line,
                     * std::condition_variable::wait will never return, without
                     * timeout. */
#ifndef NDEBUG
                    if (!job_queue.empty()) {
                        cerr << "BUG: race" << endl;
                    }
#endif

                    unique_lock<mutex> lock(cond_mutex);
                    queue_cond.wait_for(lock, chrono::milliseconds(500));

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
                unique_lock<mutex> lock(queue_mutex);
                if (job_queue.size() < static_cast<size_t>(jobs * 2)) {
                    job_queue.push(sig);
                    queue_cond.notify_one();
                    break;
                } else {
                    lock.unlock();
                    unique_lock<mutex> lock(queue_cap_cond_mutex);
                    queue_cap_cond.wait_for(lock, chrono::milliseconds(500));
                }
            }
        }

        threaded_worker(threaded_worker<T> const &) = delete;
        threaded_worker<T> operator=(threaded_worker<T> const &) = delete;

    public:
        threaded_worker(int jobs, function<void(T)> handler)
            : jobs(jobs), handler(handler) {
            if (jobs < 1) {
                throw logic_error("JOBS must be greater than 0"s);
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
                throw logic_error("Tried to start worker already started");
            }
#endif
            try {
                for (int i = 0; i < jobs; ++i) {
                    threads.push_back(
                        new thread(&threaded_worker::run_worker, this));
                }
            } catch (system_error const &e) {
#ifndef NDEBUG
                cerr << "some thread(s) cannot be launched" << endl;
#endif
            }
        }

        void queue_job(T item) {
#ifndef NDEBUG
            if (finished) {
                throw logic_error("Tried to queue job on finished worker");
            }
            if (!threads.size()) {
                cerr << "worker is not started" << endl;
            }
#endif
            worker_signal<T> *sig =
                new worker_signal<T>(signal_type::JOB, item);
            internal_queue_job(sig);
        }

        void wait() {
#ifndef NDEBUG
            if (waited) {
                throw logic_error("The worker already waited");
            }
            if (!threads.size()) {
                throw logic_error("worker is not started");
            }
#endif
            for (thread *t : threads) {
                if (t->joinable()) {
                    t->join();
                }
            }

            waited = true;
        }

        void finish() {
#ifndef NDEBUG
            if (finished) {
                throw logic_error("Tried to finish on finished worker");
            }
            if (!threads.size()) {
                throw logic_error("worker is not started");
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
