//
// Created by chenly on 8/30/21.
//

#ifndef LOCKFREEPOOL_CIRCLEBUFFER_H
#define LOCKFREEPOOL_CIRCLEBUFFER_H

#include <memory>
#include <atomic>
#include <cstring>
#include <array>

namespace lockfreepool{
    template <class T>
    class CircleBuffer{
    public:
        explicit CircleBuffer(size_t buffer_size) : m_max_size(buffer_size * sizeof(T)),
                                                    m_current_size(0) {
            m_buffer = std::unique_ptr<T[]>(new T[buffer_size]);
            m_head = m_buffer.get();
            m_tail = m_buffer.get();
        }

        bool read(T *out_buffer, size_t count) {
            if (size() < count) {
                return false;
            }
            if (m_head > m_tail) {
                std::memcpy(out_buffer, m_tail, count * sizeof(T));
                m_tail += count;
            } else {
                auto buffer_start = m_buffer.get();
                auto buffer_end = m_buffer.get() + m_max_size + 1;
                int available_size = buffer_end - m_tail;
                if (available_size > count) {
                    std::memcpy(out_buffer, m_tail, count * sizeof(T));
                    m_tail += count;
                } else {
                    std::memcpy(out_buffer, m_tail, available_size * sizeof(T));
                    m_tail = buffer_start;
                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        std::memcpy(out_buffer, m_tail, todo_size * sizeof(T));
                        m_tail += todo_size;
                    }
                }
            }
            m_current_size -= count;
            return true;
        }

        bool write(const T *in_buffer, size_t count){
            if(capacity() < count){
                return false;
            }
            if(m_head > m_tail){
                auto buffer_start = m_buffer.get();
                auto buffer_end = m_buffer.get() + m_max_size + 1;
                int available_size = buffer_end - m_head;
                if (available_size > count) {
                    std::memcpy(m_head, in_buffer, count * sizeof(T));
                    m_head += count;
                } else {
                    std::memcpy(m_head, in_buffer, available_size * sizeof(T));
                    m_head = buffer_start;
                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        std::memcpy(m_head, in_buffer + available_size, todo_size * sizeof(T));
                        m_head += todo_size;
                    }
                }
            } else {
                std::memcpy(m_head, in_buffer, count * sizeof(T));
                m_head += count;
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
                auto buffer_end = m_buffer.get() + m_max_size + 1;
                int available_size = buffer_end - m_tail;
                if (available_size > count) {
                    return std::memcmp(in_buffer, m_tail, count * sizeof(T)) == 0;
                } else {
                    std::array<T> temp(count);
                    std::memcpy(temp.data(), m_tail, available_size * sizeof(T));

                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        auto temp_tail = buffer_start;
                        std::memcpy(temp.data() + available_size, temp_tail, todo_size * sizeof(T));
                    }
                    return std::memcmp(in_buffer, temp.data(), count * sizeof(T)) == 0;
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
                m_tail += count;
            } else {
                auto buffer_start = m_buffer.get();
                auto buffer_end = m_buffer.get() + m_max_size + 1;
                int available_size = buffer_end - m_tail;
                if (available_size > count) {
                    std::memcpy(out_buffer, m_tail, count * sizeof(T));
                    m_tail += count;
                } else {
                    std::memcpy(out_buffer, m_tail, available_size * sizeof(T));
                    m_tail = buffer_start;
                    int todo_size = count - available_size;
                    if (todo_size > 0) {
                        std::memcpy(out_buffer, m_tail, todo_size * sizeof(T));
                        m_tail += todo_size;
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
        bool empty() const{
            return m_current_size == 0;
        }
        bool full() const{
            return m_current_size == m_max_size;
        }
        size_t capacity() const{
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
        size_t m_max_size;
        std::atomic<size_t> m_current_size;
    };

    class Msg{
    public:
        
    private:
    };
}
#endif //LOCKFREEPOOL_CIRCLEBUFFER_H
