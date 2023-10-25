
#ifndef SERIALISTLOOPER_QUEUE_H
#define SERIALISTLOOPER_QUEUE_H

#include <deque>
#include <thread>

template<typename T = double>
class LockingQueue {
public:
    explicit LockingQueue(std::size_t size) : m_size(size) {}


    void push(T value) {
        std::lock_guard lock(m_mutex);
        m_queue.emplace_back(value);
        if (m_queue.size() > m_size) {
            m_queue.pop_front();
        }
    }


    std::optional<T> pop() {
        std::lock_guard lock(m_mutex);

        if (m_queue.empty())
            return std::nullopt;

        auto elem = m_queue.front();
        m_queue.pop_front();
        return elem;
    }


    Vec<T> pop_all() {
        std::lock_guard lock(m_mutex);

        Vec<T> result;
        while (!m_queue.empty()) {
            result.append(std::move(m_queue.front()));
            m_queue.pop_front();
        }
        return result;
    }


    Vec<T> get_snapshot() {
        std::lock_guard lock(m_mutex);
        return {m_queue.cbegin(), m_queue.cend()};
    }


    [[nodiscard]]
    const T& back() {
        std::lock_guard lock(m_mutex);
        return m_queue.back();
    }


    bool empty() {
        return m_queue.empty();
    }


    std::size_t size() {
        return m_queue.size();
    }


private:
    std::size_t m_size;
    std::deque<T> m_queue;
    std::mutex m_mutex;
};

#endif //SERIALISTLOOPER_QUEUE_H
