//
// Created by chenly on 8/30/21.
//

#ifndef LOCKFREEPOOL_CIRCLEBUFFER_H
#define LOCKFREEPOOL_CIRCLEBUFFER_H

#include <memory>

template <class T>
class CircleBuffer{
public:
    explicit CircleBuffer(size_t buffer_size): m_max_size(buffer_size* sizeof(T)){
        m_buffer = std::unique_ptr<T[]>(new T[buffer_size]);
    }
    size_t read(T *out_buffer, size_t count){
        return 0;
    }

    bool write(const T *in_buffer, size_t count){
        return true;
    }

    int compare(const T *in_buffer, size_t count, size_t offset = 0){
        return 0;
    }

    int copy(T *out_buffer, size_t count, size_t offset = 0){
        return 0;
    }

    void reset(){
        m_head = m_buffer.get()+1;
        m_tail = m_buffer.get();
    }
    bool empty() const{

    }
    bool full() const{

    }
    size_t capacity() const{
        return m_max_size;
    }

    size_t size() const{

    }
private:

private:
    std::unique_ptr<T[]> m_buffer;
    volatile T *m_head;
    volatile T *m_tail;
    size_t m_max_size;
};
#endif //LOCKFREEPOOL_CIRCLEBUFFER_H
