
#ifndef SERIALISTLOOPER_FACET_H
#define SERIALISTLOOPER_FACET_H

#include <vector>
#include <optional>

class Facet {
public:
    static const inline double enum_epsilon = 1e-6;


    template<typename T, std::enable_if_t<std::is_same_v<T, double>, int> = 0>
    explicit Facet(double v) : m_value(v) {}


    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    explicit Facet(T v) : m_value(static_cast<double>(v)) {}


    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    static Facet from_enum(T v, T min_enum_value, T max_enum_value) {
        return Facet(enum_to_double(v, min_enum_value, max_enum_value));
    }


    explicit operator int() const {
        return static_cast<int>(m_value);
    }


    template<typename T>
    explicit operator T() const {
        return static_cast<T>(m_value);
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


    friend std::ostream& operator<<(std::ostream& os, const Facet& obj) {
        os << obj.m_value;
        return os;
    }


    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    T as_enum(const T& min_enum_value, const T& max_enum_value) const {
        return double_to_enum(m_value, min_enum_value, max_enum_value);
    }


    // TODO
//    template<typename T>
//    std::optional<T> as_enum(int min_enum_value, int max_enum_value) {
//        if (!m_value.empty())
//            return static_cast<T>(m_value.at(0));
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
    static T double_to_enum(double d, const T& min_enum, const T& max_enum) {
        auto min = static_cast<int>(min_enum);
        auto max = static_cast<int>(max_enum);
        int n_values = max - min + 1;

        auto index = std::floor((d + enum_epsilon) * n_values) - min;

        return static_cast<T>(index);
    }


    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    static double enum_to_double(const T& v, const T& min_enum, const T& max_enum) {
        auto min = static_cast<int>(min_enum);
        auto max = static_cast<int>(max_enum);
        auto enum_v = static_cast<int>(v);
        int n_values = max - min + 1;

        return static_cast<double>(enum_v - min) / static_cast<double>(n_values);
    }


    double m_value;

};


#endif //SERIALISTLOOPER_FACET_H
