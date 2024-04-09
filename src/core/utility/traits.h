
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
inline constexpr bool is_printable_v = is_printable<T>::value;


/**
 * Struct for checking whether an object is an output stream (useful for replacing std::cout with other streams)
 */
template<typename OutputStream>
struct is_output_stream {
private:
    template<typename U>
    static auto test(int) -> decltype(std::cout << std::__1::declval<U>(), std::true_type{});

    template<typename U>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<OutputStream>(0))::value;
};


template<typename T>
inline constexpr bool is_output_stream_v = is_output_stream<T>::value;




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

template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;


// ==============================================================================================

template<typename T, typename = void>
struct is_serializable : std::false_type {
};

template<typename T>
struct is_serializable<T, std::void_t<decltype(std::declval<T>().from_string(std::declval<const std::string&>())
        , std::declval<T>().to_string())>> : std::true_type {
};

template<typename T>
inline constexpr bool is_serializable_v = is_serializable<T>::value;


// ==============================================================================================

/**
 * Checks whether `T` is implicitly convertible into an OSC-protocol compatible type
 */
template<typename T>
using is_osc_convertible = std::disjunction<std::is_arithmetic<T>, std::is_convertible<T, std::string>>;

template<typename T>
inline constexpr bool is_osc_convertible_v = is_osc_convertible<T>::value;


// ==============================================================================================

template<typename T>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template<typename T>
constexpr bool is_optional_v = is_optional<std::decay_t<T>>::value;


// ==============================================================================================

/**
 * Workaround to handle std::atomic<T>::is_always_lock_free for non-trivially copyable types
 */
template<typename T>
struct is_always_lock_free_internal
        : std::integral_constant<bool, std::atomic<T>::is_always_lock_free> {
};

template<typename T>
using is_always_lock_free = std::conjunction<std::is_trivially_copyable<T>, is_always_lock_free_internal<T>>;

template<typename T>
inline constexpr bool is_always_lock_free_v = is_always_lock_free<T>::value;


} // namespace utils

#endif //SERIALISTLOOPER_TRAITS_H
