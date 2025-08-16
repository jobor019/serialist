
#ifndef SERIALIST_STACK_H
#define SERIALIST_STACK_H


#include <deque>
#include <optional>
#include "vec.h"

namespace serialist {

template<typename T>
class Stack {
public:
    explicit Stack(std::size_t size) : m_size(size) {}

    void push(T value) {
        m_stack.push_back(value);  // Add to back (top of stack)
        if (m_stack.size() > m_size) {
            m_stack.pop_front();   // Remove from front (bottom of stack)
        }
    }

    std::optional<T> pop() {
        if (m_stack.empty())
            return std::nullopt;

        auto elem = m_stack.back();  // Take from back (top of stack)
        m_stack.pop_back();
        return elem;
    }

    Vec<T> pop_all() {
        Vec<T> result;
        while (!m_stack.empty()) {
            result.append(std::move(m_stack.back()));  // Take from back (LIFO order)
            m_stack.pop_back();
        }
        return result;
    }

    Vec<T> get_snapshot() const { return Vec<T>{std::vector<T>{m_stack.cbegin(), m_stack.cend()}}; }

    const T& top() const { return m_stack.back(); }      // Most recently added (top)
    const T& bottom() const { return m_stack.front(); } // Oldest element (bottom)

    bool empty() const { return m_stack.empty(); }
    std::size_t size() const { return m_stack.size(); }

private:
    std::size_t m_size;
    std::deque<T> m_stack;
};

} // namespace serialist

#endif //SERIALIST_STACK_H
