#include <iostream>
#include <atomic>
#include "ThreadPool.h"

using namespace std;
#define WORK_QUEUE_POWER 10
#define WORK_QUEUE_SIZE (1 << WORK_QUEUE_POWER)

std::atomic<int> g_ntest(0);
//int g_ntest = 0;

void testfun(int args) {
    int i = *((int*)(&args));
    for (int j = 0; j < 1; j++) {
        ++g_ntest;
    }
    std::cout << "g_ntest:" << g_ntest << std::endl;

    //Sleep(1);
}
void addmyfunc(void *args) {
    CThreadPool* lfttest = (CThreadPool*)args;

//    while (true) {
    for (int i = 0; i <28495; ++i) {
        auto func = std::bind(testfun, i);
        if (!lfttest->add_work(func)) {
            std::this_thread::sleep_for(chrono::milliseconds(10));
            --i;
            continue;
            //return -1;
        }
    }
    //break;
    std::this_thread::sleep_for(chrono::microseconds(1));
    std::cout << "addmyfunc exit!" << std::endl;
//    }
}

void lft_pool_test();

int main(int argc, char**argv) {


    auto start = std::chrono::high_resolution_clock::now();
    //tpool_destroy(tpool, 1);
    lft_pool_test();

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "waited for "
              << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()
              << " microseconds\n";
}


void lft_pool_test() {
    CThreadPool lfttest;
    lfttest.init(0, LEAST_LOAD, 1<<3);
    std::thread th(addmyfunc, (void*)&lfttest);
    th.detach();

    while (1) {
        lfttest.show_status();
        std::cout << "g_ntest:" << g_ntest << std::endl;
        std::this_thread::sleep_for(chrono::seconds(1));
    }
}