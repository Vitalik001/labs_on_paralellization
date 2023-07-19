#ifndef COUNTWORDS_PAR_THREADSAFE_QUEUE_H
#define COUNTWORDS_PAR_THREADSAFE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex m;
    std::queue<T> data_queue;
    std::condition_variable cond_v;
    std::condition_variable cond_v_full;
    size_t max_size;

public:
    explicit threadsafe_queue(size_t max_size) : max_size(max_size) {}
    ~threadsafe_queue() = default;

    threadsafe_queue(threadsafe_queue& other) = delete;
    threadsafe_queue operator=(const threadsafe_queue& other) = delete;

    void enqueue(const T& value);
    void enqueue(T&& value);

    T dequeue();
    std::pair<T, T> dequeue_duo();
    bool empty() const;
    size_t get_size() const;
};

template<typename T>
void threadsafe_queue<T>::enqueue(const T& value) {
    {
        std::unique_lock<std::mutex> lock(m);
        cond_v_full.wait(lock, [this] {return data_queue.size() < max_size;});
        data_queue.push(value);
    }
    cond_v.notify_one();
}

template<typename T>
void threadsafe_queue<T>::enqueue(T&& value) {
    {
        std::unique_lock<std::mutex> lock(m);
        cond_v_full.wait(lock, [this] {return data_queue.size() < max_size;});
        data_queue.emplace(std::move(value));
    }
    cond_v.notify_one();
}

template<typename T>
T threadsafe_queue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(m);
    cond_v.wait(lock, [this]{return !data_queue.empty();});
    T res = data_queue.front();
    data_queue.pop();
    lock.unlock();
    cond_v_full.notify_one();
    return res;
}

template<typename T>
std::pair<T, T> threadsafe_queue<T>::dequeue_duo() {
    std::unique_lock<std::mutex> lock(m);
    cond_v.wait(lock, [this]{return data_queue.size() >= 2;});
    T res1 = std::move(data_queue.front());
    data_queue.pop();
    T res2 = data_queue.front();
    data_queue.pop();
    lock.unlock();
    cond_v_full.notify_one();
    return { std::move(res1), std::move(res2) };
}

template<typename T>
bool threadsafe_queue<T>::empty() const{
    std::lock_guard<std::mutex> lock(m);
    return data_queue.empty();
}

template<typename T>
size_t threadsafe_queue<T>::get_size() const {
    std::lock_guard<std::mutex> lock(m);
    return data_queue.size();
}

#endif //COUNTWORDS_PAR_THREADSAFE_QUEUE_H