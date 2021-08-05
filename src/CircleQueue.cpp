//
// Created by root on 8/5/21.
//

#include "CircleQueue.h"

CircleQueue::CircleQueue() :
        /*m_pthread_pool(NULL),*/ m_nin(0), m_nout(0),
        m_brunning(false), m_ptask_queue(NULL),m_ntask_size(0) {

}

CircleQueue::CircleQueue(unsigned int queue_size):
        /*m_pthread_pool(NULL),*/ m_nin(0), m_nout(0),
        m_brunning(false),m_ntask_size(0)  {
    m_nqueue_size = queue_size;
    m_nqueue_mask = m_nqueue_size - 1;
    m_ptask_queue = new ITask*[queue_size];
}

CircleQueue::~CircleQueue() {
    if (m_ptask_queue)
        delete[] m_ptask_queue;
}

uint32_t CircleQueue::get_task_size() {
    return m_ntask_size;
}
bool CircleQueue::queue_empty() {
    return (get_task_size() == 0);
}

bool CircleQueue::queue_full() {
    return (get_task_size() == m_nqueue_size);
}

unsigned int CircleQueue::val_offset(unsigned int val) {
    return (m_nqueue_mask & val);
}