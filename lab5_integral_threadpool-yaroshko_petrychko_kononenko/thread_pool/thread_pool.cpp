//
// Created by Nazar Kononenko on 15.03.2023.
//
#include "thread_pool.h"

thread_pool::thread_pool(size_t thread_amount_): done(false), joiner(threads), thread_amount(thread_amount_) {
    try {
        for (size_t i = 0; i < thread_amount; ++i) {
            threads.emplace_back(&thread_pool::worker_thread, this);
        }
    } catch(...) {
        done = true;
        throw;
    }
}



void thread_pool::worker_thread() {
    while (!done) {
        std::function<void()> task;

        if (!work_queue.empty()) {
            task = work_queue.dequeue();
            task();
        } else {
            std::this_thread::yield();
        }
    }
}

thread_pool::~thread_pool() {
    done = true;
}
