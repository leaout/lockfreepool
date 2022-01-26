/************************************************************************
  *Copyright(C),CU
  *FileName: ThreadPool.h
  *Author:leaout@163.com
  *Version:
  *Description: a thread pool without lock
  *Others:
  *History:
*************************************************************************/

#pragma once
#include <iostream>
#include <list>
#include <atomic>
#include <thread>
#include <functional>
#include <vector>
#include <map>
#include <sstream>
#include <mutex>
#include "ThreadWithQueue.h"

namespace lockfreepool{
    using namespace std;
    enum class ScheduleType {
        ROUND_ROBIN,
        LEAST_LOAD
    };

    class CThreadPool {
    public:
        CThreadPool(): m_nthread_num(0)
                , m_schedule_type(ScheduleType::ROUND_ROBIN),m_cur_thread_Index(0) {
            m_thread_queues.clear();
        }
        ~CThreadPool() {
            for(auto&queue : m_thread_queues){
                queue->stop();
            }
        }

        //ntreads,scheduletype
        bool init(int ntreads, ScheduleType schedule_type, int power) {
            int cpu_cores = std::thread::hardware_concurrency();
            if (ntreads > cpu_cores) {
                ntreads = cpu_cores;
            }

            m_thread_queues.reserve(ntreads);
            for (int i = 0; i < ntreads; ++i) {
                ThreadWithQueue* p = new ThreadWithQueue(power);
                if (p != nullptr) {
                    m_thread_queues.emplace_back(p);
                }
            }
            m_nthread_num = ntreads;
            m_schedule_type = schedule_type;

            //start all workers
            for(auto&queue : m_thread_queues){
                queue->start();
            }

            return true;
        }
        bool add_work(ITask *task){
            ThreadWithQueue *queue = nullptr;
            if (ScheduleType::LEAST_LOAD == m_schedule_type) {
                queue = least_load_schedule();
            } else {
                queue = round_robin_schedule();
            }
            if (queue)
                return queue->add_task(task);

            return false;
        }

        void stop(){
            for(auto&queue : m_thread_queues){
                queue->stop();
            }
        }
        void join(){
            for(auto&queue : m_thread_queues){
                queue->join();
            }
        }
        void show_status() {
            std::cout <<"------------------------------------------" << std::endl;
            for(auto&queue : m_thread_queues){
                std::cout <<"thread id :"<< queue->get_thread_id() << std::endl;
                std::cout <<"task size :"<< queue->get_task_size() << std::endl;
                std::cout <<"queue_full :" << queue->queue_full()<< std::endl;
                std::cout <<"queue_empty :" << queue->queue_empty() << std::endl;
            }
            std::cout <<"------------------------------------------" << std::endl;
        }


    private:

        ThreadWithQueue* round_robin_schedule() {
            int counter = 0;
            for(;counter < m_thread_queues.size(); ++counter){
                auto queue = m_thread_queues[m_cur_thread_Index];
                if(!queue->queue_full()){
                    return queue;
                }
                m_cur_thread_Index = (m_cur_thread_Index + 1) % m_nthread_num;
            }
            return nullptr;
        }

        ThreadWithQueue* least_load_schedule() {
            ThreadWithQueue* least_load_queue = nullptr;
            for(auto&queue : m_thread_queues){
                if(queue->queue_empty()){
                    return queue;
                }
                if (least_load_queue == nullptr) least_load_queue = queue;

                if (queue->get_task_size() < least_load_queue->get_task_size()) {
                    least_load_queue = queue;
                }
            }
            if(least_load_queue->queue_full()){
                return nullptr;
            }
            return least_load_queue;
        }

    private:
        vector<ThreadWithQueue*> m_thread_queues;		   //
        int m_nthread_num;

        ScheduleType m_schedule_type;
        int m_cur_thread_Index;
    };


    class MultiToOne{
    public:
        friend class ThreadWithQueue;
        MultiToOne() = delete;
        explicit MultiToOne(int power):m_power(power){

        }
        bool start() {
            m_running = true;
            //start worker
            thread th(&MultiToOne::thread_func,this);
            m_th.swap(th);

            return true;
        }
        bool add_work(ITask *task){

            auto *queue = get_queue();

            if (queue)
                return queue->add_one(task);

            return false;
        }

        void stop(){
            m_running = false;
        }
        void join(){
            if(m_th.joinable()){
                m_th.join();
            }
        }
        void thread_func(){
            int sleep_counter = 0;
            int thread_count = std::thread::hardware_concurrency();
            while (m_running) {
                for(auto&kv : m_thread_queues){
                    if (kv.second->queue_empty()) {
                        if (++sleep_counter >= thread_count) {
                            std::this_thread::yield();
                            sleep_counter = 0;
                        }

                        continue;
                    }
                    TaskStatus status = TaskStatus::Skip;
                    auto work = kv.second->get_one(status);
                    if (work) {
                        if(status != TaskStatus::Skip && work->on_init()){
                            work->on_process();
                        }
                        work->on_end();
                    }
                }
            }
        }
    private:
        CircleQueue<ITask> *get_queue(){

            auto thread_id = get_current_thread_id();
            auto find_it = m_thread_queues.find(thread_id);
            if(find_it != m_thread_queues.end()){
                return find_it->second;
            }
            //lock for new thread
            std::lock_guard<std::mutex> lock(m_mtx);
            auto thread_queue = new CircleQueue<ITask>(m_power);
            m_thread_queues.insert(std::make_pair(thread_id, thread_queue));

            return thread_queue;
        }
    private:
        long long get_current_thread_id() {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            std::string stid = oss.str();
            return std::stoll(stid);
        }
    private:
        std::map<long long,CircleQueue<ITask> *> m_thread_queues;
        std::mutex m_mtx;
        int m_power;
        std::thread m_th;
        atomic<bool> m_running;
    };
}
