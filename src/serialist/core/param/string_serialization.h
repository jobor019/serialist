
#ifndef SERIALIST_STRING_SERIALIZATION_H
#define SERIALIST_STRING_SERIALIZATION_H

#include <sstream>
#include <string>
#include <tuple>
#include <optional>
#include <type_traits>
#include <experimental/type_traits>

namespace serialist {

// ==============================================================================================
// Template Declaration
// ==============================================================================================

/**
 * @brief Non-intrusive helper for serializing and deserializing primitive types.
 *        This is equivalent to implementing `serialize` and `deserialize` methods in an intrusive manner,
 *        although the latter will always be preferred if it exists
 */
template<typename T, typename = void>
struct StringSerializer;


// ==============================================================================================
// Type Trait Helpers
// ==============================================================================================

namespace utils {
    template<typename T>
    using directly_string_serializable_signature_t = decltype(std::declval<T>().serialize());

    template<typename T>
    using directly_string_deserializable_signature_t = decltype(T::deserialize(std::declval<std::string>()));


    template<typename T>
    static constexpr bool directly_string_serializable_v = std::conjunction_v<
            std::experimental::is_detected_exact<std::string, directly_string_serializable_signature_t, T>
            , std::experimental::is_detected_exact<T, directly_string_deserializable_signature_t, T>
    >;

} // namespace utils


// ==============================================================================================
// Primitive Types
// ==============================================================================================

template<typename T>
struct StringSerializer<T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
    static std::string to_string(const T& value) { return std::to_string(value); }

    static T from_string(const std::string& token) { return static_cast<T>(std::stoll(token)); }
};


template<typename T>
struct StringSerializer<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    static std::string to_string(const T& value) { return std::to_string(value); }

    static T from_string(const std::string& token) { return std::stod(token); }
};


template<typename T>
struct StringSerializer<T, std::enable_if_t<std::is_same_v<T, bool>>> {
    static std::string to_string(const T& value) { return std::to_string(value); }

    static T from_string(const std::string& token) { return token == "1"; }
};


// ==============================================================================================
// std::string
// ==============================================================================================

template<typename T>
struct StringSerializer<T, std::enable_if_t<std::is_same_v<T, std::string>>> {
    static std::string to_string(const T& value) { return value; }

    static T from_string(const std::string& token) { return token; }
};


// ==============================================================================================
// std::optional
// ==============================================================================================

template<typename T>
struct StringSerializer<T, std::enable_if_t<std::is_same_v<T, std::optional<typename T::value_type>>>> {
    static std::string to_string(const T& value) {
        if (value) {
            return StringSerializer<typename T::value_type>::to_string(*value);
        } else {
            return "-nullopt";
        }
    }

    static T from_string(const std::string& token) {
        if (token == "-nullopt") {
            return std::nullopt;
        } else {
            return StringSerializer<typename T::value_type>::from_string(token);
        }
    }
};



// ==============================================================================================
// StringSerializationHelper
// ==============================================================================================

/**
 * Helper class for serializing a compound object into a single string
 */
class StringSerializationHelper {
private:
    template<typename T>
    using has_to_string_signature_t = decltype(StringSerializer<T>::to_string(std::declval<T>()));

    template<typename T>
    using has_from_string_signature_t = decltype(StringSerializer<T>::from_string(std::declval<std::string>()));


public:
    static const char SEPARATOR = ';';

    template<typename T>
    static constexpr bool has_string_serializer_v = std::conjunction_v<
            std::experimental::is_detected_exact<std::string, has_to_string_signature_t, T>
            , std::experimental::is_detected_exact<T, has_from_string_signature_t, T>
    >;



    template<typename... Args>
    static std::string serialize(const Args& ... args) {
        // TODO: Handle exceptions: throw as the correct type of serialist::ParameterError
        (validate_arg_type<Args>(), ...);
        std::ostringstream oss;
        ((oss << StringSerializer<Args>::to_string(args) << SEPARATOR), ...);
        std::string result = oss.str();
        result.pop_back(); // Remove the trailing semicolon
        return result;
    }

    template<typename... Args>
    static std::tuple<Args...> deserialize(const std::string& data) {
        // TODO: Handle exceptions: throw as the correct type of serialist::ParameterError
        (validate_arg_type<Args>(), ...);
        std::istringstream iss(data);
        return deserialize_impl<Args...>(iss);
    }


private:
    template<typename... Args, std::size_t... Is>
    static std::tuple<Args...> deserialize_impl(std::istringstream& iss, std::index_sequence<Is...>) {
        std::tuple<Args...> result;
        (deserialize_next(iss, std::get<Is>(result)), ...);
        return result;
    }

    template<typename... Args>
    static std::tuple<Args...> deserialize_impl(std::istringstream& iss) {
        return deserialize_impl<Args...>(iss, std::index_sequence_for<Args...>{});
    }

    template<typename T>
    static constexpr void validate_arg_type() {
        static_assert(has_string_serializer_v<T>, "No serializer available for this type");
    }

    template<typename T>
    static void deserialize_next(std::istringstream& iss, T& value) {
        // TODO[IMPORTANT]: This implementation extremely poor, and will crash when deserializing compound objects.
        //                  We need to implement a recursive solution or explicitly forbid compound objects.
        //     Example: MyObj = {int, int, Point}
        //              Point = {int, int}
        //              Serializing MyObj => "int;int;int;int"
        //              Deserializing "int;int;int;int" => with this implementation, will only pass "int" to Point.
        //     We want to avoid a full parser/lexer solution for this case, but a more
        //     flexible implementation than the current is definitely required
        std::string token;
        std::getline(iss, token, SEPARATOR);
        value = StringSerializer<T>::from_string(token);
    }
};

} // namespace serialist

#endif //SERIALIST_STRING_SERIALIZATION_H
