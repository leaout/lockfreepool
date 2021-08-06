//
// Created by chenly on 8/5/21.
//

#ifndef LOCKFREEPOOL_CIRCLEQUEUE_H
#define LOCKFREEPOOL_CIRCLEQUEUE_H

#include <cstdint>
#include <atomic>
#include "ITask.h"

using namespace std;

enum class TaskStatus{
    Pending,
    Processing,
    Skip,
    Free
};

#define MSG_LEN = 32;

template <class T>
class Slot{
public:
    T* m_task;
    char msg[32] = {};
    atomic<TaskStatus> m_status;
};

template <class T>
class CircleQueue {
public:
    CircleQueue() = delete;
    explicit CircleQueue(uint32_t queue_size): m_head(0), m_tail(0),
                                               m_ntask_size(0) {
        m_nqueue_size = queue_size;
        m_nqueue_mask = m_nqueue_size - 1;
        m_queue = new T[queue_size];
    }

    ~CircleQueue(){
        if (m_queue)
            delete[] m_queue;
    }

    uint32_t get_task_size(){
        return m_ntask_size;
    }
    bool queue_empty(){
        return (get_task_size() == 0);
    }

    bool queue_full(){
        return (get_task_size() == m_nqueue_size);
    }

    T* get_one(){
        if (get_task_size() <= 0)
            return nullptr;

        //prefetch work
        ITask* ret = nullptr;
        auto&slot = m_queue[m_tail];

        slot.m_status = TaskStatus::Free;
        slot.msg = {};
        ret = slot.m_task;

        --m_ntask_size;
        m_tail = val_offset(++m_tail);
    }

    bool add_one(T* task){
        if(queue_full()){
            return false;
        }
        auto& slot = m_queue[m_head];

        slot.m_task = task;
        slot.m_status = TaskStatus::Pending;

        ++m_ntask_size;
        m_head = val_offset(++m_head);
        return true;
    }

private:
    uint32_t val_offset(uint32_t val);
public:
    volatile uint32_t m_head;
    volatile uint32_t m_tail;
    atomic_int m_ntask_size;
    atomic_int m_nqueue_size;
    uint32_t m_nqueue_mask;

    Slot<T>* m_queue = nullptr;
};


#endif //LOCKFREEPOOL_CIRCLEQUEUE_H
