
#ifndef SERIALIST_VT_SERIALIZATION_H
#define SERIALIST_VT_SERIALIZATION_H

#include <juce_data_structures/juce_data_structures.h>
#include <type_traits>
#include <experimental/type_traits>
#include "core/param/string_serialization.h"

namespace serialist {


// ==============================================================================================
// Template Declaration
// ==============================================================================================

template<typename T, typename = void>
struct VTSerializer;


// ==============================================================================================
// Type Trait Helpers
// ==============================================================================================

namespace utils {

template<typename T>
using is_variant_convertible = std::conjunction<std::is_convertible<juce::var, T>, std::is_convertible<T, juce::var>>;

template<typename T>
inline constexpr bool is_variant_convertible_v = is_variant_convertible<T>::value;

} // namespace utils


// ==============================================================================================
// Directly convertible types
// ==============================================================================================

template<typename T>
struct VTSerializer<T, std::enable_if_t<utils::is_variant_convertible_v<T>>> {
    static T from_var(const juce::var& v) {
        return static_cast<T>(v);
    }

    static juce::var to_var(const T& t) {
        return t;
    }
};


// ==============================================================================================
// enums
// ==============================================================================================

template<typename T>
struct VTSerializer<T, std::enable_if_t<std::is_enum_v<T>>> {
    static T from_var(const juce::var& v) {
        return static_cast<T>(v);
    }

    static juce::var to_var(const T& t) {
        return static_cast<int>(t);
    }
};


// ==============================================================================================
// std::string
// ==============================================================================================

template<>
struct VTSerializer<std::string> {
    static std::string from_var(const juce::var& v) {
        return v.toString().toStdString();
    }

    static juce::var to_var(const std::string& v) {
        return juce::String(v);
    }
};


// ==============================================================================================
// Facet
// ==============================================================================================

// TODO Not sure if this should exist or not


// ==============================================================================================
// std::optional<T>
// ==============================================================================================

template<typename T>
struct VTSerializer<std::optional<T>> {
    static std::optional<T> from_var(const juce::var& v) {
        return v.isVoid() ? std::nullopt : std::make_optional<T>(VTSerializer<T>::from_var(v));
    }

    static juce::var to_var(const std::optional<T>& v) {
        return v ? VTSerializer<T>::to_var(*v) : juce::var();
    }
};


// ==============================================================================================
// GenericSerializer
// ==============================================================================================

class GenericSerializer {
private:
    template<typename T>
    using has_to_var_signature_t = decltype(VTSerializer<T>::to_var(std::declval<T>()));

    template<typename T>
    using has_from_var_signature_t = decltype(VTSerializer<T>::from_var(std::declval<juce::var>()));


public:
    template<typename T>
    static constexpr bool has_vt_serializer_v = std::conjunction_v<
            std::experimental::is_detected_exact<juce::var, has_to_var_signature_t, T>
            , std::experimental::is_detected_exact<T, has_from_var_signature_t, T>
    >;


    template<typename T>
    static constexpr bool is_serializable_v = GenericSerializer::has_vt_serializer_v<T> ||
                                              StringSerializationHelper::has_string_serializer_v<T>;


    template<typename T>
    static juce::var serialize(const T& t) {
        validate_type<T>();

        if constexpr (has_vt_serializer_v<T>) {
            return VTSerializer<T>::to_var(t);
        } else {
            return {StringSerializer<T>::to_string(t)};
        }
    }


    template<typename T>
    static T deserialize(const juce::var& v) {
        validate_type<T>();

        if constexpr (has_vt_serializer_v<T>) {
            return VTSerializer<T>::from_var(v);
        } else {
            return StringSerializer<T>::from_string(v.toString().toStdString());
        }
    }


private:
    template<typename T>
    static constexpr void validate_type() {
        static_assert(is_serializable_v<T>, "No serializer available for this type");
    }

};


} // namespace serialist


#endif //SERIALIST_VT_SERIALIZATION_H
