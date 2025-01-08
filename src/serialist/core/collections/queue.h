
#ifndef SERIALISTLOOPER_QUEUE_H
#define SERIALISTLOOPER_QUEUE_H

#include <deque>
#include "vec.h"

namespace serialist {

template<typename T>
class Queue {
public:
    explicit Queue(std::size_t size) : m_size(size) {}

    void push(T value) {
        m_queue.emplace_back(value);
        if (m_queue.size() > m_size) {
            m_queue.pop_front();
        }
    }


    std::optional<T> pop() {
        if (m_queue.empty())
            return std::nullopt;

        auto elem = m_queue.front();
        m_queue.pop_front();
        return elem;
    }


    Vec<T> pop_all() {
        Vec<T> result;
        while (!m_queue.empty()) {
            result.append(std::move(m_queue.front()));
            m_queue.pop_front();
        }
        return result;
    }


    Vec<T> get_snapshot() { return {m_queue.cbegin(), m_queue.cend()}; }
    const T& back() { return m_queue.back(); }
    bool empty() { return m_queue.empty(); }
    std::size_t size() { return m_queue.size(); }


private:
    std::size_t m_size;
    std::deque<T> m_queue;
    std::mutex m_mutex;
};

} // namespace serialist

#endif //SERIALISTLOOPER_QUEUE_H
