
#include <iostream>
#include <unistd.h>
#include <future>
#include "threadpool.h"


void func1(std::promise<int>* promise) {
    std::cout << "here is func1" << std::endl;
    sleep(5);
    promise->set_value(1);
}

void func2(std::promise<int>* promise) {
     std::cout << "here is func2" << std::endl;
     sleep(5);
     promise->set_value(2);
}

void varfunc1(std::promise<int>* promise, const int a, const int b, const int c) {
    std::cout << "here is varfunc1" << std::endl;
    sleep(2);
    promise->set_value(a + b + c);
}

void varfunc2(std::promise<int>* promise, const int a, int b) {
    std::cout << "here is varfunc2" << std::endl;
    sleep(2);
    promise->set_value(a + b);
}

int main() {
    ThreadPool* tp = new ThreadPool(1);
    tp->start();
    std::promise<int> p1;
    std::future<int> res1 = p1.get_future();
    auto bindfun1 = std::bind(varfunc1, &p1, 1, 2, 3);
    // tp->addTask(func1, &p1);
    tp->addVarArgTask(bindfun1);
    std::cout << "func1: " << res1.get() << std::endl;
    std::promise<int> p2;
    std::future<int> res2 = p2.get_future();
    auto bindfun2 = std::bind(varfunc2, &p2, 1, 2);
    // tp->addTask(func2, &p2);
    
    tp->addVarArgTask(bindfun2);
    std::cout << "func2: " << res2.get() << std::endl;
    tp->stop();
}