#include <pthread.h>
#include <queue>
#include <functional>
#include <iostream>
#include <future>
#include "threadpool.h"

void* ThreadPool::MainFunction(void* ptr) {
    ThreadPool* pool = reinterpret_cast<ThreadPool*>(ptr);
    pthread_cond_init(&pool->m_condition, NULL);
    pthread_mutex_init(&pool->m_mutex, 0);
    
    while(!pool->m_is_stop) {
        pthread_mutex_lock(&pool->m_mutex);
        while (pool->m_tasks.empty()) {
            pthread_cond_wait(&(pool->m_condition), &pool->m_mutex);
        }
        std::function<void(std::promise<int>&)> cb = pool->m_tasks.front();
        std::promise<int>& promise = pool->m_promises.front();
        pool->m_tasks.pop();
        pool->m_promises.pop();
        pthread_mutex_unlock(&pool->m_mutex);

        cb(promise);
    }
    return nullptr;
}

ThreadPool::ThreadPool(int size) : m_size(size) {
    for(int i = 0; i < m_size; ++i) {
        pthread_t thread;
        m_threads.emplace_back(thread);
    }
    pthread_cond_init(&m_condition, nullptr);
}



void ThreadPool::start() {
    std::cout << "start threadpool" << std::endl;
    for (int i = 0; i < m_size; ++i) {
        pthread_create(&m_threads[i], nullptr, &ThreadPool::MainFunction, this);
    }
}

void ThreadPool::stop() {
  m_is_stop = true;
}

void ThreadPool::addTask(std::function<void(std::promise<int>&)> cb, std::promise<int>& promise) {
    pthread_mutex_lock(&m_mutex);
    m_tasks.push(cb);
    m_promises.push(&promise);
    pthread_mutex_unlock(&m_mutex);
    pthread_cond_signal(&m_condition);
}

void ThreadPool::addVarArgTask(std::function<void(...)> cb, ...) {
    
}

ThreadPool::~ThreadPool() {}