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
#include <list>
#include <atomic>
#include <thread>
#include <functional>
#include <vector>

using namespace std;
enum class ScheduleType {
    ROUND_ROBIN, 
    LEAST_LOAD	  
};
const int kMaxThreadNum  = 512;

struct Task{
    Task() = default;
    Task(bool val):valid(val){
    }
    function<void()> task_func;
    bool valid = true;
    string msg;
};

class CThreadPool;

class CthreadCircleQueue {
  public:
    CthreadCircleQueue();
    CthreadCircleQueue(uint32_t queue_size);
    ~CthreadCircleQueue();
    uint32_t get_task_size();
    bool queue_empty();
    bool queue_full();
    uint32_t val_offset(uint32_t val);
  public:
    CThreadPool* m_pthread_pool;
  public:
    volatile uint32_t m_nin;
    volatile uint32_t m_nout;
    atomic_int m_ntask_size;
    atomic_int m_nqueue_size;
    uint32_t m_nqueue_mask;
    volatile bool m_brunning;
    Task* m_ptask_queue = nullptr;
};

class CThreadPool {
  public:
    CThreadPool();
    ~CThreadPool();

    //ntreads,scheduletype
    bool init(int thread_size, ScheduleType schedule_type, int queue_size);
    bool add_work(Task &task);
    Task get_work(CthreadCircleQueue *pthread);
    void stop_and_join();
    void show_status();

private:
    bool dispatch_work2thread(CthreadCircleQueue *pthread, Task &task);

    CthreadCircleQueue* round_robin_schedule();
    CthreadCircleQueue* least_load_schedule();
    bool stop();
  private:
    vector<CthreadCircleQueue*> m_list_threads;		   //
    int m_nthread_num;
    vector<thread> m_threads;
    ScheduleType m_schedule_type;
    int m_cur_thread_Index;
};

void tpool_thread(void *arg);
