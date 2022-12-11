/*
 * threadpool.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <vector>
#include "base.hpp"

/**
 * ThreadWork
 */
struct Model;
struct ThreadWork {
    Model *model;
    size_t point_ix;
};

/**
 * ThreadPool
 * The thread pool key data structure is a queue. When a worker finds the queue
 * empty, the worker sleeps on the queue_cond condition.
 * When a new work item is added to the queue, a worker is awaken with a signal
 * on the queue_cond condition.
 * After the worker finishes the job, broadcast a signal to all threads. If it
 * is a worker thread, pull the next job. If it is the waiter thread, check if
 * the queue is empty.
 *
 * https://stackoverflow.com/questions/6954489/how-to-utilize-a-thread-pool-with-pthreads
 */
struct ThreadPool {
    /* ThreadPool state */
    std::queue<ThreadWork> work_queue;
    bool queue_is_finished;

    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond_empty;
    pthread_cond_t queue_cond_not_empty;

    std::vector<pthread_t> work_threads;
    pthread_attr_t thread_attr;

    /* ThreadPool api */
    void create(const uint64_t n_threads);
    void destroy(void);

    void wait(void);
    void enqueue(const void *arg);

    static void *execute(void *arg);
};

#endif /* THREADPOOL_H_ */
