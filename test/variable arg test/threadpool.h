#ifndef TINYRPC_THREAD_THRFEADPOOL_H
#define TINYRPC_THREAD_THRFEADPOOL_H

#include <pthread.h>
#include <queue>
#include <functional>
#include <future>

class ThreadPool {
public:
    ThreadPool(int size);

    ~ThreadPool();
    
    void start();

    void stop();

    void addTask(std::function<void(std::promise<int>*)> cb, std::promise<int>* promise);

    void addVarArgTask(std::function<void()> cb);

private:
    static void* MainFunction(void* ptr);

public:
    int m_size {0};
    std::vector<pthread_t> m_threads;
    std::queue<std::promise<int>*> m_promises;
    std::queue<std::function<void()>> m_tasks;

    pthread_mutex_t m_mutex;
    pthread_cond_t m_condition;
    bool m_is_stop {false};

public:
    enum Code {
        Write = 0,
        Get = 1,
        Put = 2,
        Delete = 3
    };
};

#endif