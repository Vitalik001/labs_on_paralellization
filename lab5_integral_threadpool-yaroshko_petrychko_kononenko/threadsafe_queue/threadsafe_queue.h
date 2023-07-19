//
// Created by Nazar Kononenko on 07.03.2023.
//

#ifndef LAB4_THREADSAFE_QUEUE_H
#define LAB4_THREADSAFE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex m;
    std::queue<T> data_queue;
    std::condition_variable cond_v;

public:
    threadsafe_queue() = default;
    ~threadsafe_queue() = default;

    threadsafe_queue(threadsafe_queue& other) = delete;
    threadsafe_queue operator=(const threadsafe_queue& other) = delete;

    void enqueue(const T& value);
    T dequeue();
    bool empty() const;
    size_t get_size() const;
};

template<typename T>
void threadsafe_queue<T>::enqueue(const T& value) {
    {
        std::lock_guard<std::mutex> lock(m);
        data_queue.push(value);
    }
    cond_v.notify_one();
}

template<typename T>
T threadsafe_queue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(m);
    cond_v.wait(lock, [this]{return !data_queue.empty();});
    T res = data_queue.front();
    data_queue.pop();
    return res;
}

template<typename T>
bool threadsafe_queue<T>::empty() const{
    std::lock_guard<std::mutex> lock(m);
    return data_queue.empty();
}

template<typename T>
size_t threadsafe_queue<T>::get_size() const {
    std::unique_lock<std::mutex> lock(m);
    return data_queue.size();
}

#endif //LAB4_THREADSAFE_QUEUE_H
