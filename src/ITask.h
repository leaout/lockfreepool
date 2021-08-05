//
// Created by chenly on 8/5/21.
//

#ifndef LOCKFREEPOOL_ITASK_H
#define LOCKFREEPOOL_ITASK_H

#include <string>
#include <atomic>
using namespace std;

enum class TaskStatus{
    Pending,
    Processing,
    Skip
};

class ITask {
public:
    ITask() = default;
    explicit ITask(bool val):valid(val){
    }

    virtual ~ITask(){}

    virtual void on_process() = 0;
    virtual void on_end() = 0;
protected:
    bool valid = true;
    string msg;
    atomic<TaskStatus> m_status;
};


#endif //LOCKFREEPOOL_ITASK_H
