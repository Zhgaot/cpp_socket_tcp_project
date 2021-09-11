#pragma once

#include <mutex>
#include <future>
#include <thread>
#include <utility>
#include <vector>
#include "thread_safe_queue.hpp"

namespace TP {

using namespace TSQ;

/* --------------- Declaration --------------- */
class ThreadPool {
public: 
    ThreadPool (const int& thread_num);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    void init();
    void shutdown();

    /* --------------- Definition --------------- */
    template <typename F, typename... Args>
    auto submit(F &&pri_func, Args &&...args) -> std::future<decltype(pri_func(args...))> {
        // Create a function with bounded parameter ready to execute
        std::function<decltype(pri_func(args...))()> func = std::bind(std::forward<F>(pri_func), std::forward<Args>(args)...);

        // Encapsulate it into a shared pointer in order to be able to copy construct
        // auto == std::shared_ptr<std::packaged_task<decltype(f(args...))()>>
        auto task_ptr = std::make_shared<std::packaged_task<decltype(pri_func(args...))()>>(func);

        // Warp packaged task into void function
        std::function<void()> warpper_func = [task_ptr]() {
            (*task_ptr)();
        };

        // Push task into task queue
        this->queue_.push(warpper_func);

        // Wake up the first thread in the wait queue
        this->condition_.notify_one();

        return task_ptr->get_future();
    }
    /* --------------- Definition --------------- */

private:
    class ThreadWorker;

private:
    bool shutdown_;  // Is the ThreadPool closed?
    ThreadSafeQueue<std::function<void()>> queue_;  // Task queue
    std::vector<std::thread> threads_;  // thread-pool implemented using vector
    std::mutex condition_mutex_;  // mutex for condition variable
    std::condition_variable condition_;  // condition variable
};
/* --------------- Declaration --------------- */


/* --------------- Definition --------------- */
class ThreadPool::ThreadWorker{
public:
    ThreadWorker(ThreadPool* pool, const int& id) : pool_(pool), work_id_(id) {}
    void operator()() {
        std::function<void()> func;  // init reusing function
        bool pop_success = false;  // Whether the task was successfully poped
        while (!this->pool_->shutdown_) {
            {   
                // just unique_lock! cannot use lock_guard!
                std::unique_lock<std::mutex> myGuard(this->pool_->condition_mutex_);
                this->pool_->condition_.wait(myGuard, [this]() -> bool {
                    if (this->pool_->queue_.empty())
                        return false;
                    return true;
                });
                pop_success = this->pool_->queue_.pop(func);
            }
            if (pop_success)
                func();  // Executing tasks asynchronously
        }
    }
private:
    int work_id_;  // work id
    ThreadPool* pool_;  // The ThreadPool to which it belongs
};

inline ThreadPool::ThreadPool(const int& thread_num)
    : shutdown_(false), threads_(std::vector<std::thread>(thread_num)) {
        this->threads_.reserve(thread_num);
    }

inline void ThreadPool::init() {
    for (int i = 0; i < this->threads_.size(); ++i) {
        this->threads_[i] = std::thread(ThreadWorker(this, i));
    }
}

inline void ThreadPool::shutdown() {
    this->shutdown_ = true;
    this->condition_.notify_all();  // Wake up all threads
    for (int i = 0; i < this->threads_.size(); ++i) {
        if (this->threads_[i].joinable()) {  // Are there any threads not running yet?
            this->threads_[i].join();  // main-thread waits for the child-threads finish
        }
    }
}
/* --------------- Definition --------------- */
}
