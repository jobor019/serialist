

#ifndef SERIALIST_LOOPER_UTILS_H
#define SERIALIST_LOOPER_UTILS_H

#include <type_traits>

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

#endif //SERIALIST_LOOPER_UTILS_H
