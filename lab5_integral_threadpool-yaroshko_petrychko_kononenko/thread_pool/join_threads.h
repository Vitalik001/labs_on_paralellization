//
// Created by Nazar Kononenko on 15.03.2023.
//

#ifndef INTEGRATE_PARALLEL_THREAD_POOL_JOIN_THREADS_H
#define INTEGRATE_PARALLEL_THREAD_POOL_JOIN_THREADS_H

#include <iostream>
#include <vector>
#include <thread>

class join_threads {
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& threads_): threads(threads_) {}
    ~join_threads() {
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
};

#endif //INTEGRATE_PARALLEL_THREAD_POOL_JOIN_THREADS_H
