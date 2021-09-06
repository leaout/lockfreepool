#include <iostream>
#include <atomic>
#include "ThreadPool.h"
#include "CircleBuffer.h"
using namespace std;

class TestTask :public ITask{
public:
    virtual ~TestTask(){}

    bool on_init() override {
        return true;
    }
    void on_process() override{
        int a = 1+2;
    }
    void on_end() override {
        delete this;
    }
};

void addmyfunc(void *args) {
    lockfreepool::CThreadPool *th_pool = (lockfreepool::CThreadPool *) args;

    auto start = std::chrono::high_resolution_clock::now();
    int total_count = 1000 * 10000;
    for (int i = 0; i < total_count; ++i) {
        TestTask* task = new TestTask;
        if (!th_pool->add_work(task)) {
            --i;
            delete task;
            continue;
        }
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "waited for "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
              << " ms\n"
              << "qps :" << (double)total_count/std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() << " /s"<<std::endl;
}

void lft_pool_test() {
    lockfreepool::CThreadPool th_pool;
    th_pool.init(6, lockfreepool::ScheduleType::LEAST_LOAD, 10);
    std::thread th(addmyfunc, (void*)&th_pool);
    th.join();

    th_pool.stop();
    th_pool.join();
}
void multi_add(void *args) {
    lockfreepool::MultiToOne *th_pool = (lockfreepool::MultiToOne *) args;

    auto start = std::chrono::high_resolution_clock::now();
    int total_count = 100 * 10000;
    for (int i = 0; i < total_count; ++i) {
        TestTask* task = new TestTask;
        if (!th_pool->add_work(task)) {
            --i;
            delete task;
            continue;
        }
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "waited for "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
              << " ms\n"
              << "qps :" << (double)total_count/std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() << " /s"<<std::endl;
}

void multi_to_one_test(){
    lockfreepool::MultiToOne reduce_pool(14);
    reduce_pool.start();
    vector<std::thread> vec_ths;
    vec_ths.resize(std::thread::hardware_concurrency());
    for(int i = 0; i < vec_ths.size(); ++i){
        std::thread th(multi_add, (void*)&reduce_pool);

        vec_ths[i].swap(th);
    }
    for(auto&th : vec_ths){
        th.join();
    }

    reduce_pool.stop();
    reduce_pool.join();
}

int main(int argc, char**argv) {

    lft_pool_test();
    multi_to_one_test();
}

