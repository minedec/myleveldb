
#include <iostream>
#include <unistd.h>
#include <future>
#include <numa.h>
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
    int node_id = numa_node_of_cpu(sched_getcpu());
    std::cout << "here is varfunc1, running on node: " << node_id << std::endl;
    sleep(2);
    promise->set_value(a + b + c);
}

void varfunc2(std::promise<int>* promise, const int a, int b) {
    int node_id = numa_node_of_cpu(sched_getcpu());
    std::cout << "here is varfunc2, running on node: " << node_id << std::endl;
    sleep(2);
    promise->set_value(a + b);
}

int main() {
    if(numa_available() < 0) {
   	    std::cout << "Your system does not support NUMA API\n";
    }
    int node_num = numa_max_node();
    std::cout << "max node num " << node_num << std::endl;
    nodemask_t mask;
    nodemask_zero(&mask);
    ThreadPool* tp1 = new ThreadPool(1, 0);
    ThreadPool* tp2 = new ThreadPool(1, 1);
    tp1->start();
    tp2->start();
    std::promise<int> p1;
    std::future<int> res1 = p1.get_future();
    auto bindfun1 = std::bind(varfunc1, &p1, 1, 2, 3);
    // tp->addTask(func1, &p1);
    tp1->addVarArgTask(bindfun1);
    std::cout << "func1: " << res1.get() << std::endl;
    std::promise<int> p2;
    std::future<int> res2 = p2.get_future();
    auto bindfun2 = std::bind(varfunc2, &p2, 1, 2);
    // tp->addTask(func2, &p2);
    
    tp2->addVarArgTask(bindfun2);
    std::cout << "func2: " << res2.get() << std::endl;
    tp1->stop();
    tp2->stop();
}