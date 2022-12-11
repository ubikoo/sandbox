/*
 * threadpool.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "threadpool.hpp"
#include "model.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * ThreadPool::create
 * @brief Create a pool with a specified number of work_threads.
 */
void ThreadPool::create(const uint64_t n_threads)
{
    /* Initialize queue */
    while (!work_queue.empty()) {
        work_queue.pop();
    }
    queue_is_finished = false;

    /* Initialize the queue mutex and condition variable */
    pthread_mutex_init(&queue_lock, NULL);
    pthread_cond_init(&queue_cond_empty, NULL);
    pthread_cond_init(&queue_cond_not_empty, NULL);

    /* Create a pool of threads */
    work_threads.resize(n_threads);
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
    for (auto &thread : work_threads) {
        pthread_create(
            &thread,
            &thread_attr,
            execute,
            this);
    }
}

/**
 * ThreadPool::destroy
 * @brief Destroy the thread pool and join all threads.
 */
void ThreadPool::destroy(void)
{
    /* Set queue finished flag */
    pthread_mutex_lock(&queue_lock);
    queue_is_finished = true;
    pthread_mutex_unlock(&queue_lock);

    /* Wake up any workers so they can recheck the finished flag and exit */
    pthread_cond_broadcast(&queue_cond_not_empty);

    /* Wait for all threads to terminate.  */
    for (auto &thread : work_threads) {
        pthread_join(thread, NULL);
    }
}

/**
 * ThreadPool::enqueue
 * @brief Insert a new work item into the work queue and signal the condition
 * queue_cond to awake thread and execute the work.
 */
void ThreadPool::enqueue(const void *arg)
{
    const ThreadWork *work = static_cast<const ThreadWork *>(arg);
    pthread_mutex_lock(&queue_lock);
    work_queue.push(*work);
    pthread_cond_broadcast(&queue_cond_not_empty);
    pthread_mutex_unlock(&queue_lock);
}

/**
 * ThreadPool::wait
 * @brief Wait blocks until all work items in have been processed, ie while
 * the work queue is not empty.
 */
void ThreadPool::wait(void)
{
    /*
     * If queue is not empty, sleep and wait for a signal on the condition that
     * queue is empty (sent by a worker thread).
     */
	pthread_mutex_lock(&queue_lock);
    while (!work_queue.empty()) {
        pthread_cond_wait(&queue_cond_empty, &queue_lock);
    }
    pthread_mutex_unlock(&queue_lock);
}

/**
 * ThreadPool::execute
 * @brief Execute a work item from the queue.
 */
void *ThreadPool::execute(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);

    /** Run while the queue is not empty */
    while (true) {
        /*
         * If the queue is empty, sleep and wait for a signal on the condition
         * queue is not empty (sent by the job enqueue function).
         */
        pthread_mutex_lock(&(pool->queue_lock));
        while (!pool->queue_is_finished && pool->work_queue.empty()) {
            pthread_cond_wait(&(pool->queue_cond_not_empty), &(pool->queue_lock));
        }

        if (pool->queue_is_finished) {
            pthread_mutex_unlock(&(pool->queue_lock));
            pthread_exit(NULL);
        }

        /*
         * Pop a work item from the queue. If the queue is empty, wake up the
         * wait thread on the condition queue is empty.
         */
        ThreadWork work = pool->work_queue.front();
        pool->work_queue.pop();
        if (pool->work_queue.empty()) {
            pthread_cond_signal(&(pool->queue_cond_empty));
        }
        pthread_mutex_unlock(&(pool->queue_lock));

        /* Execute work item and store the result. */
        work.model->integrate(work.point_ix);
        // pthread_mutex_lock(&(pool->queue_lock));
        // work.model->sample(*(work.data));
        // pthread_mutex_unlock(&(pool->queue_lock));
    }
}
