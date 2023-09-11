

#ifndef SERIALIST_LOOPER_UTILS_H
#define SERIALIST_LOOPER_UTILS_H

#include <type_traits>
#include <cmath>
#include <deque>
#include <magic_enum.hpp>

namespace utils {

template <typename T>
struct is_printable {
    template <typename U>
    static auto test(int) -> decltype(std::cout << std::declval<U>(), std::true_type{});

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(0))::value;
};


template <typename T>
constexpr bool is_printable_v = is_printable<T>::value;

// ==============================================================================================

template<typename T, typename _ = void>
struct is_container : std::false_type {
};

template<typename T>
struct is_container<
        T, std::conditional_t<
                false, std::void_t<
                        typename T::value_type, typename T::size_type, typename T::allocator_type, typename T::iterator
                        , typename T::const_iterator, decltype(std::declval<T>().size()), decltype(std::declval<
                                T>().begin()), decltype(std::declval<T>().end()), decltype(std::declval<T>().cbegin())
                        , decltype(std::declval<T>().cend())
                >, void>
> : public std::true_type {
};


// ==============================================================================================

template<typename EnumType>
bool enum_is_consecutive_and_zero_indexed() {
    constexpr auto enum_entries = magic_enum::enum_entries<EnumType>();
    if (enum_entries.empty()) {
        return true;
    }

    int prev = static_cast<int>(enum_entries.at(0).first);

    if (prev != 0) {
        return false;
    }

    for (auto it = std::begin(enum_entries) + 1; it != std::end(enum_entries); ++it) {
        int current = static_cast<int>(it->first);
        if (current != prev + 1) {
            return false;
        }
        prev = current;
    }
    return true;
}


// ==============================================================================================

template<typename T, typename... Args>
std::vector<T*> collect_if(Args* ... args) {
    std::vector<T*> collected;

    ([&] {
        if (auto* elem = dynamic_cast<T*>(args)) {
            collected.emplace_back(elem);
        }
    }(), ...);
    return collected;
}


// ==============================================================================================

/**
 * @brief Computes the remainder of a division between two doubles, allowing negative nominators.
 *
 * This function computes the remainder of the division of a double value `n` by another double value `d`,
 * ensuring that the result is always positive.
 *
 * @param n The dividend.
 * @param d The divisor.
 * @return The remainder of the division `n` by `d`, always positive.
 */
inline double modulo(double n, double d) {
    return std::fmod(std::fmod(n, d) + d, d);
}


inline long modulo(long n, long d) {
    return ((n % d) + d) % d;
}


// ==============================================================================================


template<typename T = double>
class LockingQueue {
public:
    explicit LockingQueue(std::size_t size) : m_size(size) {}


    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace_back(value);
        if (m_queue.size() > m_size) {
            m_queue.pop_front();
        }
    }


    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_queue.empty())
            return std::nullopt;

        auto elem = m_queue.front();
        m_queue.pop_front();
        return elem;
    }


    std::vector<T> pop_all() {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::vector<T> result;
        while (!m_queue.empty()) {
            result.push_back(std::move(m_queue.front()));
            m_queue.pop_front();
        }
        return result;
    }


    std::vector<T> get_snapshot() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return {m_queue.cbegin(), m_queue.cend()};
    }


    [[nodiscard]]
    const T& back() {
        std::lock_guard<std::mutex> lock(m_mutex);
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

} // namespace utils

#endif //SERIALIST_LOOPER_UTILS_H
