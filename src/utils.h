

#ifndef SERIALIST_LOOPER_UTILS_H
#define SERIALIST_LOOPER_UTILS_H

#include <type_traits>
#include <cmath>

namespace utils {

    template<typename T, typename _ = void>
    struct is_container : std::false_type {
    };

    template<typename T>
    struct is_container<
            T,
            std::conditional_t<
                    false,
                    std::void_t<
                            typename T::value_type,
                            typename T::size_type,
                            typename T::allocator_type,
                            typename T::iterator,
                            typename T::const_iterator,
                            decltype(std::declval<T>().size()),
                            decltype(std::declval<T>().begin()),
                            decltype(std::declval<T>().end()),
                            decltype(std::declval<T>().cbegin()),
                            decltype(std::declval<T>().cend())
                    >,
                    void>
    > : public std::true_type {
    };


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

}

#endif //SERIALIST_LOOPER_UTILS_H
