
#include <iostream>
#include <unistd.h>
#include <future>
#include "threadpool.h"


int func1() {
    std::cout << "here is func1" << std::endl;
    sleep(5);
    return 1;
}

int func2() {
     std::cout << "here is func2" << std::endl;
     sleep(5);
     return 2;
}


int main() {
    ThreadPool* tp = new ThreadPool(1);
    tp->start();
    std::future<int> res1 = tp->addTask(func1);
    std::cout << res1.get() << std::endl;
    std::future<int> res2 = tp->addTask(func2);
    std::cout << res2.get() << std::endl;
    tp->stop();
}