#include "ThreadPool.h"

CThreadPool::CThreadPool() : m_nthread_num(0)
    , m_schedule_type(ScheduleType::ROUND_ROBIN),m_cur_thread_Index(0) {
    m_thread_queues.clear();
}

CThreadPool::~CThreadPool() {

    for(auto&queue : m_thread_queues){
        queue->stop();
    }
}

bool CThreadPool::init(int ntreads, ScheduleType schedule_type,int power) {
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

bool CThreadPool::add_work(ITask *task) {
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

void CThreadPool::stop_and_join(){

    for(auto&queue : m_thread_queues){
        queue->stop();
    }
}

bool CThreadPool::stop(){
    for(auto&queue : m_thread_queues){
        if(queue->queue_empty()){
            queue->stop();
        } else{
            return false;
        }
    }
    return true;
}

#include <iostream>
void CThreadPool::show_status() {
    std::cout <<"------------------------------------------" << std::endl;
    for(auto&queue : m_thread_queues){
        std::cout <<"thread id :"<< queue->get_thread_id() << std::endl;
        std::cout <<"task size :"<< queue->get_task_size() << std::endl;
        std::cout <<"queue_full :" << queue->queue_full()<< std::endl;
        std::cout <<"queue_empty :" << queue->queue_empty() << std::endl;
    }
    std::cout <<"------------------------------------------" << std::endl;
}



ThreadWithQueue* CThreadPool::round_robin_schedule() {

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

ThreadWithQueue* CThreadPool::least_load_schedule() {

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
