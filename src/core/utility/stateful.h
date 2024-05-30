
#ifndef SERIALISTLOOPER_STATEFUL_H
#define SERIALISTLOOPER_STATEFUL_H

#include <type_traits>
#include "traits.h"
#include "math.h"

/**
 * @brief Wrapper class that tracks if the value has changed.

 */
template<typename T>
class WithChangeFlag {
public:
    template<typename E = T, typename = std::enable_if_t<std::is_default_constructible_v<E>>>
    WithChangeFlag() {}

    WithChangeFlag(const T& value) : m_value(value) {} // NOLINT(*-explicit-constructor)

    T* operator->() { return &m_value; }

    const T* operator->() const { return &m_value; }

    T& operator*() { return m_value; }

    const T& operator*() const { return m_value; }

    const T& get() const { return m_value; }

    void set(const T& value) {
        if constexpr (utils::is_inequality_comparable_v<T>) {
            m_changed = value != m_value;
        } else if constexpr (std::is_floating_point_v<T>) {
            m_changed = utils::equals(value, m_value);
        } else {
            m_changed = true;
        }
        m_value = value;
    }

    bool changed() const {
        return m_changed;
    }

    void clear_flag() {
        m_changed = false;
    }


private:
    T m_value;
    bool m_changed = false;
};


// ==============================================================================================

//template<typename T, std::size_t N>
//class WithHistory

#endif //SERIALISTLOOPER_STATEFUL_H
