//
// Created by chenly on 8/5/21.
//

#ifndef LOCKFREEPOOL_ITASK_H
#define LOCKFREEPOOL_ITASK_H

#include <string>
#include <atomic>
using namespace std;

class ITask {
public:
    ITask() = default;

    virtual ~ITask(){}

    virtual bool on_init() = 0;
    virtual void on_process() = 0;
    virtual void on_end() = 0;
protected:

};


#endif //LOCKFREEPOOL_ITASK_H
