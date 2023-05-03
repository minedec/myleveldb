#include <iostream>
#include <queue>
#include <future>
#include <thread>
#include <unistd.h>
#include <queue>

static std::queue<std::promise<int>*> global_q;
static int cnt = 0;

void SetPromise(std::promise<int>* promise) {
    cnt++;
    std::cout << "start func" << cnt << std::endl;
    sleep(2);
    promise->set_value(cnt);
    std::cout << "end func" << std::endl;
}

void worker() {
    std::cout << "start loop" << std::endl;
    while(true) {
        sleep(1);
        std::cout << "queue size" << global_q.size() << std::endl;
        while(!global_q.empty()) {
            std::promise<int>* promise =  global_q.front();
            std::cout << "get promise" << std::endl;
            global_q.pop();
            SetPromise(promise);
        }
    }
}

int main() {
    std::queue<std::promise<int>*> q1;
    std::queue<std::future<int>*> q2;
    
    std::promise<int> p1;
    std::future<int> f1 = p1.get_future();
    std::promise<int> p2;
    std::future<int> f2 = p2.get_future();

    q1.push(&p1);
    q1.push(&p2);
    q2.push(&f1);
    q2.push(&f2);

    // global_q.push(&p1);
    // global_q.push(&p2);

    // std::thread t(&worker);
    // t.detach();

    // std::cout << "before future" << std::endl;
    // std::cout << f1.get() << std::endl;
    // std::cout << f2.get() << std::endl;
    // std::cout << "after future" << std::endl;


    // for(int i = 0; i < 3; i++) {
    //     std::promise<int> *tmp = new std::promise<int>();
    //     std::future<int> tmpf = tmp->get_future();
    //     q1.push(tmp);
    //     q2.push(&tmpf);
    // }

    // std::promise<int> p1;
    // std::future<int> f1 = p1.get_future();

    for(int i = 0; i < 2; i++) {
        std::thread t(&SetPromise, q1.front());
        q1.pop();
        t.detach();
        sleep(5);
    }

    for(int i = 0; i < 2; i++) {
        std::cout << "future value: " << q2.front()->get() << std::endl;
        q2.pop();
    }

    std::cout << "finish main" << std::endl;
}