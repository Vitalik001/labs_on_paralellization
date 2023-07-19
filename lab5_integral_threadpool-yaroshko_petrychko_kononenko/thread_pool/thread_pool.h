//
// Created by Nazar Kononenko on 15.03.2023.
//

#include <iostream>
#include <atomic>
#include <vector>
#include "threadsafe_queue.h"
#include <functional>
#include <thread>
#include "join_threads.h"
#include <future>

#ifndef INTEGRATE_PARALLEL_THREAD_POOL_THREAD_POOL_H
#define INTEGRATE_PARALLEL_THREAD_POOL_THREAD_POOL_H

class thread_pool {
    std::atomic_bool done;
    threadsafe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;
    size_t thread_amount;

    void worker_thread();

public:
    explicit thread_pool(size_t thread_amount);
    ~thread_pool();

    template<typename T>
    std::future<typename std::result_of<T()>::type> submit(T function);
};

template<typename T>
std::future<typename std::result_of<T()>::type> thread_pool::submit(T function) {
    typedef typename std::result_of<T()>::type result_type;
    auto task = std::make_shared<std::packaged_task<result_type()>>([function]() {
        return function();
    });
    std::future<result_type> res = task->get_future();
    work_queue.enqueue([task]() {
        (*task)();
    });
    return res;
}

#endif //INTEGRATE_PARALLEL_THREAD_POOL_THREAD_POOL_H