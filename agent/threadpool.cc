#include <pthread.h>
#include <queue>
#include <functional>
#include <iostream>
#include "threadpool.h"
#include "port/port.h"
#include "util/mutexlock.h"

// use as thread pool for leveldb which transfer request to real db instance
// client ---> thread pool ---> worker thread ---> db instance ---> Get/Put
namespace leveldb {

// thread pool main function, do loop work to process function void()
void* ThreadPool::MainFunction(void* ptr) {
    ThreadPool* pool = reinterpret_cast<ThreadPool*>(ptr);
    pthread_cond_init(&pool->m_condition, NULL);
    // pthread_mutex_init(&pool->m_mutex, 0);
    
    while(!pool->m_is_stop) {
        pthread_mutex_lock(&pool->m_mutex);
        while (pool->m_tasks.empty()) {
            pthread_cond_wait(&(pool->m_condition), &pool->m_mutex);
        }
        std::function<void()> cb = pool->m_tasks.front();
        pool->m_tasks.pop();
        pthread_mutex_unlock(&pool->m_mutex);

        cb();
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
    for (int i = 0; i < m_size; ++i) {
        pthread_create(&m_threads[i], nullptr, &ThreadPool::MainFunction, this);
    }
}

void ThreadPool::stop() {
  m_is_stop = true;
}

void ThreadPool::addTask(std::function<void()> cb) {
    pthread_mutex_lock(&m_mutex);
    m_tasks.push(cb);
    pthread_mutex_unlock(&m_mutex);
    pthread_cond_signal(&m_condition);
}

ThreadPool::~ThreadPool() {}

}