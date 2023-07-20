
#ifndef SERIALISTLOOPER_FACET_H
#define SERIALISTLOOPER_FACET_H

#include <vector>
#include <optional>
#include <magic_enum.hpp>

class Facet {
public:
    static const inline double enum_epsilon = 1e-6;


    explicit Facet(double v) : m_value(v) {}


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    explicit Facet(const T& v) : m_value(static_cast<double>(v)) {}

    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    explicit Facet(const T& v) : m_value(enum_to_double(v)) {}


//    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
//    static Facet from_enum(const T& v) {
//        return Facet(enum_to_double<T>(v));
//    }


    explicit operator int() const {
        return static_cast<int>(m_value);
    }


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    explicit operator T() const {
        return static_cast<T>(m_value);
    }

    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    explicit operator T() const {
        return double_to_enum<T>(m_value);
    }


    explicit operator bool() const {
        return m_value >= 0.5;
    }


    explicit operator double() const {
        return m_value;
    }


    bool operator==(const bool b) const {
        return static_cast<bool>(*this) == b;
    }


    template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    bool operator==(const T& t) const { return static_cast<T>(*this) >= t; }


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    bool operator!=(const T& t) const { return static_cast<T>(*this) != t; }


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    bool operator>=(const T& t) const { return static_cast<T>(*this) >= t; }


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    bool operator<=(const T& t) const { return static_cast<T>(*this) <= t; }


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    bool operator>(const T t) const { return static_cast<T>(*this) > t; }


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    bool operator<(const T& t) const { return static_cast<T>(*this) < t; }


//    template<typename OutputType, std::enable_if_t<std::is_arithmetic_v<OutputType>, int> = 0>
//    Facet operator+(const OutputType& t) {
//        return Facet(m_value + t);
//    }
//
//
//    template<typename OutputType, std::enable_if_t<std::is_arithmetic_v<OutputType>, int> = 0>
//    Facet operator-(const OutputType& t) {
//        return Facet(m_value - t);
//    }
//
//
//    template<typename OutputType, std::enable_if_t<std::is_arithmetic_v<OutputType>, int> = 0>
//    Facet operator*(const OutputType& t) {
//        return Facet(m_value * t);
//    }
//
//
//    template<typename OutputType, std::enable_if_t<std::is_arithmetic_v<OutputType>, int> = 0>
//    Facet operator/(const OutputType& t) {
//        return Facet(m_value / t);
//    }


    friend std::ostream& operator<<(std::ostream& os, const Facet& obj) {
        os << obj.m_value;
        return os;
    }


//    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
//    T as_enum(const T& min_enum_value, const T& max_enum_value) const {
//        return double_to_enum(m_value, min_enum_value, max_enum_value);
//    }


    double get() const {
        return m_value;
    }


    // TODO
//    template<typename OutputType>
//    std::optional<OutputType> as_enum(int min_enum_value, int max_enum_value) {
//        if (!m_value.empty())
//            return static_cast<OutputType>(m_value.at(0));
//        return std::nullopt;
//    }




//    std::optional<int> as_int() {
//        if (!m_value.empty())
//            return static_cast<int>(m_value.at(0));
//        return std::nullopt;
//    }
//
//
//    std::optional<double> as_double() {
//        if (!m_value.empty())
//            return static_cast<double>(m_value.at(0));
//        return std::nullopt;
//    }
//
//
//    std::optional<bool> as_bool() {
//        if (!m_value.empty())
//            return static_cast<bool>(m_value.at(0));
//        return std::nullopt;
//    }


//    std::vector<double> get_value() {
//        return m_value;
//    }


private:

    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    static T double_to_enum(double d) {
        d = std::min(1.0, std::max(0.0, d));
        constexpr auto n_values = magic_enum::enum_count<T>();

        auto index = std::floor((d + enum_epsilon) * n_values);

        return static_cast<T>(index);
    }


    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    static double enum_to_double(const T& v) {
        auto enum_v = static_cast<int>(v);
        constexpr auto n_values = magic_enum::enum_count<T>();

        return static_cast<double>(enum_v) / static_cast<double>(n_values);
    }


    double m_value;

};


#endif //SERIALISTLOOPER_FACET_H
