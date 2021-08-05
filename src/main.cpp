#include <iostream>
#include <atomic>
#include "ThreadPool.h"

using namespace std;
#define WORK_QUEUE_POWER 10


std::atomic<int> g_ntest(0);

auto kStart = std::chrono::high_resolution_clock::now();

void testfun() {

    ++g_ntest;
    if (g_ntest % 500000 == 0) {
        auto elapsed = std::chrono::high_resolution_clock::now() - kStart;
        int mirco_time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        std::cout << g_ntest <<";time:"<<mirco_time<<"; testfun speed "
                  << (double) g_ntest / mirco_time
                  << " ll/ms\n";
    }
}

void addmyfunc(void *args) {
    CThreadPool *lfttest = (CThreadPool *) args;

    auto func = std::bind(testfun);
    Task task;
    task.task_func = func;

    task.msg = "hello";
    for (int i = 0; i < 100 * 10000; ++i) {
        if (!lfttest->add_work(task)) {
            --i;
            continue;
        }
    }
    function<void()> temp;
    task.task_func.swap(temp);
    std::cout << "addmyfunc exit!" << std::endl;
}

void lft_pool_test();

int main(int argc, char**argv) {
    string tmp;
    tmp.resize(10,0);
    auto start = std::chrono::high_resolution_clock::now();
    //tpool_destroy(tpool, 1);
    lft_pool_test();

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "waited for "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
              << " ms\n";
}

void lft_pool_test() {
    CThreadPool lfttest;
    lfttest.init(0, ScheduleType::LEAST_LOAD, 1<<10);
    std::thread th(addmyfunc, (void*)&lfttest);
    th.detach();
//    std::this_thread::sleep_for(std::chrono::seconds(1));
    lfttest.stop_and_join();
}