
#ifndef SERIALISTLOOPER_TRAITS_H
#define SERIALISTLOOPER_TRAITS_H

#include "magic_enum.hpp"
#include <deque>
#include <cmath>
#include <type_traits>
#include <iostream>

namespace serialist::utils {

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
struct is_serializable<T, std::void_t<
        decltype(std::declval<T>().deserialize(std::declval<const std::string&>())
        , std::declval<T>().serialize())>>
        : std::true_type {
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

template<typename, typename = void>
struct is_equality_comparable : std::false_type {};

// Specialization that checks if T == T is valid
template<typename T>
struct is_equality_comparable<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};

template<typename T>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;


template<typename, typename = void>
struct is_inequality_comparable : std::false_type {};

// Specialization that checks if T != T is valid
template<typename T>
struct is_inequality_comparable<T, std::void_t<decltype(std::declval<T>() != std::declval<T>())>> : std::true_type {};

template<typename T>
inline constexpr bool is_inequality_comparable_v = is_inequality_comparable<T>::value;


template <typename T, typename = void>
struct is_less_than_comparable : std::false_type {};

template <typename T>
struct is_less_than_comparable<T, std::void_t<decltype(std::declval<T>() < std::declval<T>())>>
    : std::true_type {};

template<typename T>
inline constexpr bool is_less_than_comparable_v = is_less_than_comparable<T>::value;

// alias
template<typename T>
inline constexpr bool is_sortable_v = is_less_than_comparable<T>::value;


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


// ==============================================================================================

template <typename T, typename U, typename = void>
struct is_static_castable : std::false_type {};

template <typename T, typename U>
struct is_static_castable<T, U, std::void_t<decltype(static_cast<U>(std::declval<T>()))>>
    : std::true_type {};

template <typename T, typename U>
inline constexpr bool is_static_castable_v = is_static_castable<T, U>::value;

} // namespace serialist::utils

#endif //SERIALISTLOOPER_TRAITS_H
