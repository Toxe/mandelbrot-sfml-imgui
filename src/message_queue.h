#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class MessageQueue {
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<T> queue_;

public:
    void send(T&& msg);

    [[nodiscard]] T wait_for_message();

    void notify_one() { cv_.notify_one(); };
    void notify_all() { cv_.notify_all(); };

    int clear();
    [[nodiscard]] bool empty();
    [[nodiscard]] auto size();
};

template <typename T>
void MessageQueue<T>::send(T&& msg)
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.emplace(std::move(msg));
    }

    cv_.notify_one();
}

template <typename T>
[[nodiscard]] T MessageQueue<T>::wait_for_message()
{
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [&] { return !queue_.empty(); });

    T msg = std::move(queue_.front());
    queue_.pop();

    return msg;
}

template <typename T>
int MessageQueue<T>::clear()
{
    std::lock_guard<std::mutex> lock(mtx_);
    int messages_removed = 0;

    while (!queue_.empty()) {
        queue_.pop();
        --messages_removed;
    }

    return messages_removed;
}

template <typename T>
[[nodiscard]] bool MessageQueue<T>::empty()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.empty();
}

template <typename T>
[[nodiscard]] auto MessageQueue<T>::size()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.size();
}
