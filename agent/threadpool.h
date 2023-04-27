#include <pthread.h>
#include <queue>
#include <functional>
#include "leveldb/db.h"

namespace leveldb {

class ThreadPool {
public:
    ThreadPool(int size);

    ~ThreadPool();
    
    void start();

    void stop();

    void addTask(std::function<void()> cb);

private:
    static void* MainFunction(void* ptr);

public:
    int m_size {0};
    std::vector<pthread_t> m_threads;
    std::queue<std::function<void()>> m_tasks;

    pthread_mutex_t m_mutex;
    pthread_cond_t m_condition;
    bool m_is_stop {false};

    DB** m_db;
    enum OperationCode {
        Write = 0,
        Get = 1,
        Put = 2,
        Delete = 3
    };
};

}
