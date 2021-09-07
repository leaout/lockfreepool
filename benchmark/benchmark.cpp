//
// Created by chenly on 2021/9/5.
//

#include <iostream>
#include <atomic>
#include <iomanip>

#include "../src/ThreadPool.h"
#include "../src/ThreadPool.h"

class Stat {
public:
    void finished_single_op() {
        m_done++;
        if (m_done >= m_next_report) {

            m_next_report += 500000;

            std::fprintf(stderr, "... finished %d ops%30s\r", m_done.load(), "");
            std::fflush(stderr);
        }
    }

public:

    std::atomic<int> m_done;
    int m_next_report = 500000;
};

void print_result(const char* func_name, int time_cost,Stat& stat){
    cout.setf(ios::left);
    std::cout << std::setw(32) <<func_name  << " : " << stat.m_done / time_cost * 1000 << " ops/s"<< std::endl;
}

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

struct ThreadMeta{
    void* th_pool = nullptr;
    Stat stat;
};

void addmyfunc(ThreadMeta *args) {
    lockfreepool::CThreadPool *th_pool = (lockfreepool::CThreadPool *) args->th_pool;

    auto start = std::chrono::high_resolution_clock::now();
    int total_count = 1000 * 10000;
    for (int i = 0; i < total_count; ++i) {
        TestTask* task = new TestTask;
        if (!th_pool->add_work(task)) {
            --i;
            delete task;
            continue;
        }
        args->stat.finished_single_op();
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    int time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    print_result(__FUNCTION__,time_cost, args->stat);
}

void lft_pool_test() {
    lockfreepool::CThreadPool th_pool;
    th_pool.init(6, lockfreepool::ScheduleType::LEAST_LOAD, 10);
    ThreadMeta thread_meta;
    thread_meta.th_pool = (void*)&th_pool;

    std::thread th(addmyfunc, &thread_meta);
    th.join();

    th_pool.stop();
    th_pool.join();
}


void multi_add(ThreadMeta *args) {
    lockfreepool::MultiToOne *th_pool = (lockfreepool::MultiToOne *) args->th_pool;

    int total_count = 100 * 10000;
    for (int i = 0; i < total_count; ++i) {
        TestTask* task = new TestTask;
        if (!th_pool->add_work(task)) {
            --i;
            delete task;
            continue;
        }
        args->stat.finished_single_op();
    }
}

void multi_to_one_test(){
    lockfreepool::MultiToOne reduce_pool(14);
    reduce_pool.start();
    vector<std::thread> vec_ths;
    vec_ths.resize(std::thread::hardware_concurrency());
    ThreadMeta thread_meta;
    thread_meta.th_pool = (void*)&reduce_pool;

    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < vec_ths.size(); ++i){
        std::thread th(multi_add, &thread_meta);

        vec_ths[i].swap(th);
    }
    for(auto&th : vec_ths){
        th.join();
    }

    reduce_pool.stop();
    reduce_pool.join();
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    int time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    print_result(__FUNCTION__,time_cost, thread_meta.stat);
}

int main(int argc, char *argv[]) {
    lft_pool_test();
    multi_to_one_test();
    return 0;
}