/* Thread safe implementation of a Queue using a std::queue */

#pragma once

#include <mutex>
#include <queue>

namespace TSQ {

/* --------------- Declaration --------------- */
template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() {}  // constructors can not be cv-qualified(no const)
    ThreadSafeQueue(ThreadSafeQueue&& other) {}
    ~ThreadSafeQueue() {}
    bool empty();
    int size();
    void push(const T& val);
    bool pop(T& val);

private:
    std::queue<T> queue_;
    std::mutex mutex_;
};

/* --------------- Definition --------------- */
template <typename T>
inline bool ThreadSafeQueue<T>::empty() {
    std::lock_guard<std::mutex> myGuard(this->mutex_);
    return this->queue_.empty();
}

template <typename T>
inline int ThreadSafeQueue<T>::size() {
    std::lock_guard<std::mutex> myGuard(this->mutex_);
    return this->queue_.size();
}

template <typename T>
inline void ThreadSafeQueue<T>::push(const T& val) {
    std::lock_guard<std::mutex> myGuard(this->mutex_);
    this->queue_.emplace(val);
}

template <typename T>
inline bool ThreadSafeQueue<T>::pop(T& val) {
    std::lock_guard<std::mutex> myGuard(this->mutex_);
    if (this->queue_.empty())
        return false;
    val = std::move(this->queue_.front());  // Rvalue
    this->queue_.pop();  // delete front element
    return true;
}

}  // namespace TSQ
