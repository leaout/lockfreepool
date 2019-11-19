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

using namespace std;
enum schedule_type {
    ROUND_ROBIN, 
    LEAST_LOAD	  
};
const int MAX_THREAD_NUM  = 512;

class CThreadPool;

class CthreadCircleQueue {
  public:
    CthreadCircleQueue();
    CthreadCircleQueue(unsigned int queue_size);
    ~CthreadCircleQueue();
    int get_task_size();
    bool queue_empty();
    bool queue_full();
    unsigned int val_offset(unsigned int val);
  public:
    CThreadPool* m_pthread_pool;
  public:
    unsigned int m_nin;
    unsigned int  m_nout;
    std::atomic_uint m_ntask_size;
    std::atomic_uint m_nqueue_size;
    unsigned int m_nqueue_mask;
    bool m_brunning;
    std::function<void()>* m_ptask_queue;
};


class CThreadPool {
  public:
    CThreadPool();
    ~CThreadPool();
    //ntreads,scheduletype
    bool init(int ntreads, int scheduletype, int nqsize);
    bool add_work(std::function<void()>);
    std::function<void()> get_work(CthreadCircleQueue *pthread);

    void show_status();
  private:
    bool dispatch_work2thread(CthreadCircleQueue *pthread, std::function<void()> routine);

    CthreadCircleQueue* round_robin_schedule();
    CthreadCircleQueue* least_load_schedule();

  private:
    list<CthreadCircleQueue*> m_list_threads;		   //
    int m_nthread_num;

    int m_schedule_type;			 
    int m_cur_thread_Index;			
};

void tpool_thread(void *arg);
