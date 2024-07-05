
#ifndef SERIALIST_VT_SERIALIZER_H
#define SERIALIST_VT_SERIALIZER_H

#include <juce_data_structures/juce_data_structures.h>

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



template<typename T, typename = void>
struct has_vt_serializer : std::false_type {
};

template<typename T>
struct has_vt_serializer<T, std::void_t<decltype(VTSerializer<T>::from_var(std::declval<juce::var>()))
                                        , decltype(VTSerializer<T>::to_var(std::declval<T>()))>>
        : std::true_type {
};

template<typename T>
inline constexpr bool has_vt_serializer_v = has_vt_serializer<T>::value;


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
// Indirectly serializable objects (implements to_string and from_string)
// ==============================================================================================

template<typename T>
struct VTSerializer<T, std::enable_if_t<!utils::is_variant_convertible_v<T> &&utils::is_serializable_v<T>>> {
    static T from_var(const juce::var& v) {
        return T::deserialize(v.toString().toStdString());
    }

    static juce::var to_var(const T& t) {
        return {t.serialize()};
    }

};

} // namespace serialist


#endif //SERIALIST_VT_SERIALIZER_H
