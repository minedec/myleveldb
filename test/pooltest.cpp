
#include <iostream>
#include <unistd.h>
#include "threadpool.h"


void func1() {
    std::cout << "here is func1" << std::endl;
}

void func2() {
     std::cout << "here is func2" << std::endl;
}


int main() {
    std::cout << "aaa" << std::endl;
    ThreadPool* tp = new ThreadPool(1);
    tp->start();
    sleep(5);
    tp->addTask(func1);
    sleep(5);
    tp->addTask(func2);
    sleep(5);
    tp->stop();
}