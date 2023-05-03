
#include <iostream>
#include <unistd.h>
#include <future>
#include "threadpool.h"


void func1(std::promise<int>& promise) {
    std::cout << "here is func1" << std::endl;
    sleep(5);
    promise.set_value(1);
}

void func2(std::promise<int>& promise) {
     std::cout << "here is func2" << std::endl;
     sleep(5);
     promise.set_value(2);
}


int main() {
    ThreadPool* tp = new ThreadPool(1);
    tp->start();
    std::promise<int> p1;
    std::future<int> res1 = p1.get_future();
    tp->addTask(func1, &p1);
    std::cout << res1.get() << std::endl;
    std::promise<int> p2;
    std::future<int> res2 = p2.get_future();
    tp->addTask(func2, &p2);
    std::cout << res2.get() << std::endl;
    tp->stop();
}