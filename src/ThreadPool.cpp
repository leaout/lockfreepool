#include "ThreadPool.h"

CthreadCircleQueue::CthreadCircleQueue() :
    m_pthread_pool(NULL), m_nin(0), m_nout(0),
    m_brunning(false), m_ptask_queue(NULL),m_ntask_size(0) {

}

CthreadCircleQueue::CthreadCircleQueue(unsigned int queue_size):
    m_pthread_pool(NULL), m_nin(0), m_nout(0),
    m_brunning(false),m_ntask_size(0)  {
    m_nqueue_size = queue_size;
    m_nqueue_mask = m_nqueue_size - 1;
    m_ptask_queue = new Task[queue_size];
}

CthreadCircleQueue::~CthreadCircleQueue() {
    if (m_ptask_queue)
        delete[] m_ptask_queue;
}

uint32_t CthreadCircleQueue::get_task_size() {
    return m_ntask_size;
}
bool CthreadCircleQueue::queue_empty() {
    return (get_task_size() == 0);
}

bool CthreadCircleQueue::queue_full() {
    return (get_task_size() == m_nqueue_size);
}

unsigned int CthreadCircleQueue::val_offset(unsigned int val) {
    return (m_nqueue_mask & val);
}

Task CThreadPool::get_work(CthreadCircleQueue *pthread) {
//    std::function<void()> work = NULL;

    if (pthread->get_task_size() <= 0)
        return Task(false);

    //prefetch work
    auto work = pthread->m_ptask_queue[pthread->m_nout];
    --pthread->m_ntask_size;
    pthread->m_nout = pthread->val_offset(++pthread->m_nout);
//    ++pthread->m_nout;

    return work;
}

void tpool_thread(void *arg) {
    CthreadCircleQueue *pthread = (CthreadCircleQueue *)arg;

    while (pthread->m_brunning || !pthread->queue_empty()) {
        //thread exit
        if (pthread->queue_empty()) {
            std::this_thread::sleep_for(chrono::milliseconds(1));
            continue;
        }

        auto work = pthread->m_pthread_pool->get_work(pthread);
        if (work.valid) {
            work.task_func();
        }
    }
}

CThreadPool::CThreadPool() : m_nthread_num(0)
    , m_schedule_type(ScheduleType::ROUND_ROBIN),m_cur_thread_Index(0) {
    m_list_threads.clear();
}

CThreadPool::~CThreadPool() {

    if (m_list_threads.size() == 0) {
        return;
    }
    for (list<CthreadCircleQueue*>::iterator it = m_list_threads.begin(); it != m_list_threads.end(); ++it) {
        (*it)->m_brunning = true;
    }
}

bool CThreadPool::init(int ntreads, ScheduleType schedule_type,int nqsize) {

    if (ntreads <= 0) {
        ntreads = std::thread::hardware_concurrency();
    } else if (ntreads > kMaxThreadNum) {
        return false;
    }
    m_threads.resize(ntreads);
    for (int i = 0; i < ntreads; ++i) {
        CthreadCircleQueue* p = new CthreadCircleQueue(nqsize);
        if (p != NULL) {
            m_list_threads.push_back(p);
        }
    }
    m_nthread_num = ntreads;
    m_schedule_type = schedule_type;
    int index = 0;
    for (list<CthreadCircleQueue*>::iterator it = m_list_threads.begin(); it != m_list_threads.end(); ++it) {
        (*it)->m_pthread_pool = this;
        (*it)->m_brunning = true;

        std::thread th(tpool_thread, (void *)(*it));
        m_threads[index++].swap(th);
    }

    return true;
}

bool CThreadPool::add_work(Task &task) {
    CthreadCircleQueue *pthread = NULL;
    if (ScheduleType::LEAST_LOAD == m_schedule_type) {
        pthread = least_load_schedule();
    } else {
        pthread = round_robin_schedule();
    }
    if (pthread == NULL)
        return false;

    return dispatch_work2thread(pthread, task);
}

void CThreadPool::stop_and_join(){
    for(;;){
        if(stop()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds (10));
    }
    for(auto&thread: m_threads){
        thread.join();
    }
}

bool CThreadPool::stop(){
    for(auto&val : m_list_threads){
        if(val->queue_empty()){
            val->m_brunning = false;
        } else{
            return false;
        }
    }
    return true;
}

#include <iostream>
void CThreadPool::show_status() {
    std::cout <<"------------------------------------------" << std::endl;
    for (list<CthreadCircleQueue*>::iterator it = m_list_threads.begin(); it != m_list_threads.end(); ++it) {
        std::cout <<"thread_queue_task :"<< (*it)->get_task_size() << std::endl;
        std::cout <<"is runing :" << (*it)->m_brunning<< std::endl;
        std::cout <<"m_nin :" << (*it)->m_nin << std::endl;
        std::cout <<"m_nout :" << (*it)->m_nout << std::endl;
    }
    std::cout <<"------------------------------------------" << std::endl;
}

bool CThreadPool::dispatch_work2thread(CthreadCircleQueue *pthread, Task &task) {

    if (pthread != NULL && pthread->queue_full()) {
        return false;
    }

    auto work = &pthread->m_ptask_queue[pthread->m_nin];
    *work = task;

    ++pthread->m_ntask_size;
    pthread->m_nin = pthread->val_offset(++pthread->m_nin);

    return true;
}

CthreadCircleQueue* CThreadPool::round_robin_schedule() {
    m_cur_thread_Index = (m_cur_thread_Index + 1) % m_nthread_num;
    int i = 0;
    for (list<CthreadCircleQueue*>::iterator it = m_list_threads.begin(); it != m_list_threads.end(); ++it) {
        if (i++ == m_cur_thread_Index)
            return (*it);
    }
    return NULL;
}
CthreadCircleQueue* CThreadPool::least_load_schedule() {

    CthreadCircleQueue* pret = NULL;
    for (list<CthreadCircleQueue*>::iterator it = m_list_threads.begin(); it != m_list_threads.end(); ++it) {
        if (pret == NULL)
            pret = (*it);

        if ((*it)->get_task_size() < pret->get_task_size()) {
            pret = (*it);
        }
    }
    return pret;
}
