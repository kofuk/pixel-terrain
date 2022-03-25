// SPDX-License-Identifier: MIT

#ifndef WORKER_THREAD_HH
#define WORKER_THREAD_HH

/* Generic implementation of threaded worker. */

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <system_error>
#include <thread>
#include <utility>

namespace pixel_terrain {
    namespace {
        enum class signal_type { JOB, TERMINATE };

        template <typename T>
        class worker_signal {
            signal_type type_;
            T data_;

        public:
            worker_signal(signal_type type, T data)
                : type_(type), data_(data) {}

            worker_signal(signal_type type) : type_(type) {}

            [[nodiscard]] auto type() const -> signal_type { return type_; }

            [[nodiscard]] auto data() const -> T const & { return data_; }
        };
    } // namespace

    template <typename T>
    class threaded_worker {
        unsigned int n_workers_;
        std::function<void(T)> handler_;

        std::vector<std::thread *> workers_;
        std::mutex queue_mtx_, signal_mtx_;
        std::condition_variable signal_cond_;
        bool finished_ = false;

        std::queue<T> job_queue_;

        auto fetch_job_block() -> worker_signal<T> {
            for (;;) {
                std::unique_lock<std::mutex> queue_lock(queue_mtx_);
                if (job_queue_.empty()) {
                    if (finished_) {
                        return worker_signal<T>(signal_type::TERMINATE);
                    }
                    queue_lock.unlock();

                    std::unique_lock<std::mutex> signal_lock(signal_mtx_);
                    signal_cond_.wait(signal_lock);
                    continue;
                }
                T item = job_queue_.front();
                job_queue_.pop();
                return worker_signal<T>(signal_type::JOB, item);
            }
        }

        void handle_jobs_internal() {
            for (;;) {
                worker_signal<T> sig = fetch_job_block();

                if (sig.type() == signal_type::TERMINATE) {
                    break;
                }

                handler_(sig.data());
            }
        }

    public:
        threaded_worker(unsigned int n_workers, std::function<void(T)> handler)
            : n_workers_(n_workers), handler_(std::move(handler)) {}

        ~threaded_worker() {
            if (!workers_.empty()) {
                finish();
            }
        }

        threaded_worker(threaded_worker<T> const &) = delete;
        auto operator=(threaded_worker<T> const &)
            -> threaded_worker<T> = delete;

        auto start() -> bool {
            for (unsigned int i = 0; i < n_workers_; ++i) {
                try {
                    auto *th = new std::thread(
                        &threaded_worker<T>::handle_jobs_internal, this);
                    workers_.push_back(th);
                } catch (std::system_error const &) {
                    finish();
                    return false;
                }
            }
            return true;
        }

        void queue_job(T item) {
            std::unique_lock<std::mutex> lock(queue_mtx_);
            job_queue_.push(std::move(item));
            signal_cond_.notify_one();
        }

        void finish() {
            {
                std::unique_lock<std::mutex> lock(queue_mtx_);
                finished_ = true;
            }

            for (std::thread *th : workers_) {
                signal_cond_.notify_all();
                th->join();
                delete th;
            }
            workers_.clear();
        }
    };
} // namespace pixel_terrain

#endif
