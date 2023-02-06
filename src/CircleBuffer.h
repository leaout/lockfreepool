//
// Created by chenly on 8/30/21.
//

#ifndef LOCKFREEPOOL_CIRCLEBUFFER_H
#define LOCKFREEPOOL_CIRCLEBUFFER_H

#include <memory>
#include <atomic>
#include <cstring>
#include <array>
#include <assert.h>

namespace lockfreepool{
    template <class T>
    class CircleBuffer{
    public:
        explicit CircleBuffer(size_t buffer_size) : m_max_size(buffer_size),
                                                    m_current_size(0) {
            m_buffer = std::unique_ptr<T[]>(new T[buffer_size]);
            m_head = m_buffer.get();
            m_tail = m_buffer.get();
            m_buffer_start = m_buffer.get();
            m_buffer_end = m_buffer.get() + buffer_size;
        }

        bool read(T *out_buffer, size_t count) {
            if (size() < count) {
                return false;
            }
            assert(m_tail <= m_buffer_end);
            if (m_head > m_tail) {
                std::memcpy(out_buffer, (T*)m_tail, count);
                m_tail += count;
                assert(m_tail <= m_buffer_end);
            } else {

                auto buffer_end = m_buffer.get() + m_max_size;
                int available_size = buffer_end - m_tail;
                if (available_size > count) {
                    std::memcpy(out_buffer, (T*)m_tail, count);
                    m_tail += count;
                    assert(m_tail <= m_buffer_end);
                } else {
                    std::memcpy(out_buffer, (T*)m_tail, available_size);
                    auto buffer_start = m_buffer.get();
                    m_tail = buffer_start;
                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        std::memcpy(out_buffer+available_size, (T*)m_tail, todo_size);
                        m_tail += todo_size;
                        assert(m_tail <= m_buffer_end);
                        assert(m_tail <= m_head);
                    }
                }
            }
            m_current_size -= count;
            return true;
        }
        size_t read_all(T *out_buffer, size_t buffer_size) {
            auto now_available_size = size();
            size_t read_size = now_available_size < buffer_size? now_available_size:buffer_size;
            if (read_size <= 0) {
                return 0;
            }

            assert(m_tail <= m_buffer_end);
            if (m_head > m_tail) {
                std::memcpy(out_buffer, (T*)m_tail, read_size);
                m_tail += read_size;
                assert(m_tail <= m_buffer_end);
            } else {

                auto buffer_end = m_buffer.get() + m_max_size;
                int available_size = buffer_end - m_tail;
                if (available_size > read_size) {
                    std::memcpy(out_buffer, (T*)m_tail, read_size);
                    m_tail += read_size;
                    assert(m_tail <= m_buffer_end);
                } else {
                    std::memcpy(out_buffer, (T*)m_tail, available_size);
                    auto buffer_start = m_buffer.get();
                    m_tail = buffer_start;
                    int todo_size = read_size - available_size;
                    if (todo_size > 0) {
                        std::memcpy(out_buffer+available_size, (T*)m_tail, todo_size);
                        m_tail += todo_size;
                        assert(m_tail <= m_buffer_end);
                        assert(m_tail <= m_head);
                    }
                }
            }
            m_current_size -= read_size;
            return read_size;
        }

        bool write(const T *in_buffer, size_t count){
            if(capacity() < count){
                return false;
            }
            assert(m_head <= m_buffer_end);
            if(m_head > m_tail){

                int available_size = m_buffer_end - m_head;
                if (available_size > count) {
                    std::memcpy((T*)m_head, in_buffer, count);
                    m_head += count;
                    assert(m_head <= m_buffer_end);
                } else {
                    std::memcpy((T*)m_head, in_buffer, available_size);
                    m_head = m_buffer_start;
                    int todo_size = count - available_size;
                    int new_available_size = m_tail - m_head;
                    if (todo_size <= new_available_size) {
                        std::memcpy((T*)m_head, in_buffer + available_size, todo_size);
                        m_head += todo_size;
                        assert(m_head <= m_buffer_end);
                    } else{
                        std::cout << "buffer error" << std::endl;
                    }

                }
            } else {
                int available_size = m_buffer_end - m_head;
                if(available_size > count){
                    std::memcpy((T *) m_head, in_buffer, count);
                    m_head += count;
                    assert(m_head <= m_buffer_end);
                } else{
                    std::memcpy((T*)m_head, in_buffer, available_size);

                    m_head = m_buffer_start;
                    int todo_size = count - available_size;
                    int new_available_size = m_tail - m_head;
                    if (todo_size <= new_available_size) {
                        std::memcpy((T*)m_head, in_buffer + available_size, todo_size);
                        m_head += todo_size;
                        assert(m_head <= m_buffer_end);
                    } else{
                        std::cout << "buffer error" << std::endl;
                    }
                }
            }

            m_current_size += count;
            return true;
        }

        bool equal(const T *in_buffer, size_t count){
            if (size() < count) {
                return false;
            }
            if (m_head > m_tail) {
                return std::memcmp(in_buffer, m_tail, count * sizeof(T)) == 0;
            } else {
                auto buffer_start = m_buffer.get();
                auto buffer_end = m_buffer.get() + m_max_size;
                int available_size = buffer_end - m_tail;
                if (available_size > count) {
                    return std::memcmp(in_buffer, m_tail, count * sizeof(T)) == 0;
                } else {
                    T temp[count];
                    std::memcpy(temp, m_tail, available_size * sizeof(T));

                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        auto temp_tail = buffer_start;
                        std::memcpy(temp + available_size, temp_tail, todo_size * sizeof(T));
                    }
                    return std::memcmp(in_buffer, temp, count * sizeof(T)) == 0;
                }
            }
            return false;
        }

        bool copy(T *out_buffer, size_t count){
            if (size() < count) {
                return false;
            }
            if (m_head > m_tail) {
                std::memcpy(out_buffer, m_tail, count * sizeof(T));
            } else {
                auto buffer_start = m_buffer.get();
                auto buffer_end = m_buffer.get() + m_max_size;
                int available_size = buffer_end - m_tail;
                if (available_size > count) {
                    std::memcpy(out_buffer, m_tail, count * sizeof(T));
                } else {
                    std::memcpy(out_buffer, m_tail, available_size * sizeof(T));
                    T* tmp_tail = buffer_start;
                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        std::memcpy(out_buffer + available_size, tmp_tail, todo_size * sizeof(T));
                    }
                }
            }
            return true;
        }

        void reset() {
            m_head = m_buffer.get();
            m_tail = m_buffer.get();
            m_current_size = 0;
        }
        bool erase(int count) {
            if (size() < count) {
                return false;
            }
            if (m_head > m_tail) {
                m_tail += count;
            } else {
                auto buffer_start = m_buffer.get();
                auto buffer_end = m_buffer.get() + m_max_size;
                int available_size = buffer_end - m_tail;
                if (available_size > count) {
                    m_tail += count;
                } else {
                    m_tail = buffer_start;
                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        m_tail += todo_size;
                    }
                }
            }
            return true;
        }
        bool empty() const{
            return m_current_size == 0;
        }
        bool full() const{
            return m_current_size == m_max_size;
        }
        size_t capacity() const{
            assert(m_max_size >= m_current_size);
            return m_max_size - m_current_size;
        }

        size_t size() const{
            return m_current_size;
        }
    private:

    private:
        std::unique_ptr<T[]> m_buffer;
        volatile T *m_head;
        volatile T *m_tail;
        T *m_buffer_start;
        T *m_buffer_end;
        size_t m_max_size;
        std::atomic<size_t> m_current_size;
    };

    class Msg{
    public:

    private:
        CircleBuffer<char> m_buffer;
    };
}
#endif //LOCKFREEPOOL_CIRCLEBUFFER_H
