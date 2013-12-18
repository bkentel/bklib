//TODO replace with a lock-free queue?

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace bklib {

template <typename T>
class concurrent_queue {
public:
    void push(T&& e) {
        { //lock
            std::unique_lock<std::mutex> lock(mutex_);
            elements_.push(std::move(e));
        } //unlock

        if (elements_.size() > 200) {
            BK_DEBUG_BREAK();
        }

        empty_condition_.notify_all();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (elements_.empty()) empty_condition_.wait(lock);

        T result = std::move(elements_.front());
        elements_.pop();

        return result;
    }

    bool is_empty() const {
        return elements_.empty();
    }
private:
    std::mutex              mutable mutex_;
    std::condition_variable         empty_condition_;
    std::queue<T>                   elements_;
};

} //namespace bklib
