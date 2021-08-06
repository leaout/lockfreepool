//
// Created by chenly on 8/6/21.
//

#ifndef LOCKFREEPOOL_THREADWITHQUEUE_H
#define LOCKFREEPOOL_THREADWITHQUEUE_H

#include <vector>
#include <thread>
#include <atomic>

#include "ITask.h"
#include "CircleQueue.h"

class ThreadWithQueue {
public:
    ThreadWithQueue() = delete;
    explicit ThreadWithQueue(int power):m_queue(power),m_running(false){
    }
    ~ThreadWithQueue(){
        stop();
    }

    void start(){
        m_running.store(true);
        thread th(&ThreadWithQueue::thread_func,this);
        m_th.swap(th);
    }
    void stop(){
        m_running.store(false);
        if(m_th.joinable()){
            m_th.join();
        }
    }

    bool add_task(ITask* task){
        m_queue.add_one(task);
    }

    uint32_t get_task_size() {
        return m_queue.get_task_size();
    }
    bool queue_empty() {
        return m_queue.queue_empty();
    }

    bool queue_full() {
        return m_queue.queue_full();
    }
    thread::id get_thread_id(){
        return m_th.get_id();
    }
private:
    void thread_func(){
        while (m_running || !m_queue.queue_empty()) {

            if (m_queue.queue_empty()) {
                std::this_thread::sleep_for(chrono::milliseconds(1));
                continue;
            }
//            std::this_thread::sleep_for(chrono::seconds (2));
            auto work = m_queue.get_one();
            if (work) {
                ++m_call_count;
                if(work->on_init()){
                    work->on_process();
                    work->on_end();
                }
            }
        }
    }
private:
    int m_call_count = 0;
    CircleQueue<ITask> m_queue;
    thread m_th;
    atomic<bool> m_running;
};


#endif //LOCKFREEPOOL_THREADWITHQUEUE_H
