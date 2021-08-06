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

#include "ThreadWithQueue.h"

using namespace std;
enum class ScheduleType {
    ROUND_ROBIN, 
    LEAST_LOAD	  
};

class CThreadPool {
  public:
    CThreadPool();
    ~CThreadPool();

    //ntreads,scheduletype
    bool init(int thread_size, ScheduleType schedule_type, int power);
    bool add_work(ITask *task);

    void stop_and_join();
    void show_status();

private:

    ThreadWithQueue* round_robin_schedule();
    ThreadWithQueue* least_load_schedule();
    bool stop();
  private:
    vector<ThreadWithQueue*> m_thread_queues;		   //
    int m_nthread_num;

    ScheduleType m_schedule_type;
    int m_cur_thread_Index;
};

void tpool_thread(void *arg);
