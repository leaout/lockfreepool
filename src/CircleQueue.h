//
// Created by chenly on 8/5/21.
//

#ifndef LOCKFREEPOOL_CIRCLEQUEUE_H
#define LOCKFREEPOOL_CIRCLEQUEUE_H

#include <cstdint>
#include <atomic>
#include "ITask.h"

using namespace std;

class CircleQueue {
public:
    CircleQueue();
    explicit CircleQueue(uint32_t queue_size);
    ~CircleQueue();
    uint32_t get_task_size();
    bool queue_empty();
    bool queue_full();
    uint32_t val_offset(uint32_t val);
public:
//    CThreadPool* m_pthread_pool;
public:
    volatile uint32_t m_nin;
    volatile uint32_t m_nout;
    atomic_int m_ntask_size;
    atomic_int m_nqueue_size;
    uint32_t m_nqueue_mask;
    volatile bool m_brunning;
    ITask** m_ptask_queue = nullptr;
};


#endif //LOCKFREEPOOL_CIRCLEQUEUE_H
