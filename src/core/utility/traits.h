
#ifndef SERIALISTLOOPER_TRAITS_H
#define SERIALISTLOOPER_TRAITS_H

#include "magic_enum.hpp"
#include <deque>
#include <cmath>
#include <type_traits>
#include <iostream>

namespace utils {

template<typename T>
struct is_printable {
    template<typename U>
    static auto test(int) -> decltype(std::cout << std::__1::declval<U>(), std::true_type{});

    template<typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(0))::value;

};


template<typename T>
constexpr bool is_printable_v = is_printable<T>::value;


// ==============================================================================================

template<typename T, typename _ = void>
struct is_container : std::false_type {
};

template<typename T>
struct is_container<
        T, std::conditional_t<
                false, std::void_t<
                        typename T::value_type
                        , typename T::size_type
                        , typename T::allocator_type
                        , typename T::iterator
                        , typename T::const_iterator
                        , decltype(std::declval<T>().size())
                        , decltype(std::declval<T>().begin())
                        , decltype(std::declval<T>().end())
                        , decltype(std::declval<T>().cbegin())
                        , decltype(std::declval<T>().cend())
                >, void>
> : public std::true_type {
};


// ==============================================================================================

template<typename T, typename = void>
struct is_serializable : std::false_type {
};

template<typename T>
struct is_serializable<T, std::void_t<decltype(std::declval<T>().from_string(std::declval<const std::string&>())
        , std::declval<T>().to_string())>> : std::true_type {
};


} // namespace utils

#endif //SERIALISTLOOPER_TRAITS_H
