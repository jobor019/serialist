
#ifndef SERIALISTLOOPER_FACET_H
#define SERIALISTLOOPER_FACET_H

#include <iomanip>
#include <vector>
#include <optional>
#include <magic_enum.hpp>

namespace serialist {

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

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    static std::vector<Facet> vector_cast(const std::vector<T>& v) {
        std::vector<Facet> output;
        output.reserve(v.size());

        std::transform(v.begin(), v.end(), std::back_inserter(output)
                       , [](const T& t) { return static_cast<Facet>(t); });

        return output;
    }


    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    static std::vector<Facet> vector_cast(const std::vector<T>& v) {
        std::vector<Facet> output;
        output.reserve(v.size());

        std::transform(v.begin(), v.end(), std::back_inserter(output)
                       , [](const T& t) { enum_to_double(t); });

        return output;
    }


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


    bool operator==(const Facet& other) const {
        return std::abs(m_value - other.m_value) < enum_epsilon;
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


    friend std::ostream& operator<<(std::ostream& os, const Facet& facet) {
        auto original_precision = os.precision();
        os << std::fixed << std::setprecision(3);
        os << facet.m_value;
        os << std::setprecision(static_cast<int>(original_precision));

        return os;
    }


    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    Facet operator+(U other) const {
        return Facet(m_value + static_cast<double>(other));
    }


    template<typename U
             , typename = std::enable_if_t<std::conjunction_v<std::is_arithmetic<U>
                                                              , std::negation<std::is_same<Facet, U>>>>>
    friend Facet operator+(U lhs, const Facet& rhs) {
        return Facet(static_cast<double>(lhs) + rhs.m_value);
    }


    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    Facet operator-(U other) const {
        return Facet(m_value - static_cast<double>(other));
    }


    template<typename U
             , typename = std::enable_if_t<std::conjunction_v<std::is_arithmetic<U>
                                                              , std::negation<std::is_same<Facet, U>>>>>
    friend Facet operator-(U lhs, const Facet& rhs) {
        return Facet(static_cast<double>(lhs) - rhs.m_value);
    }


    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    Facet operator*(U other) const {
        return Facet(m_value * static_cast<double>(other));
    }


    template<typename U
             , typename = std::enable_if_t<std::conjunction_v<std::is_arithmetic<U>
                                                              , std::negation<std::is_same<Facet, U>>>>>
    friend Facet operator*(U lhs, const Facet& rhs) {
        return Facet(static_cast<double>(lhs) * rhs.m_value);
    }


    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    Facet operator/(U other) const {
        return Facet(m_value / static_cast<double>(other));
    }


    template<typename U
             , typename = std::enable_if_t<std::conjunction_v<std::is_arithmetic<U>
                                                              , std::negation<std::is_same<Facet, U>>>>>
    friend Facet operator/(U lhs, const Facet& rhs) {
        return Facet(static_cast<double>(lhs) / rhs.m_value);
    }


    double operator*() const {
        return m_value;
    }



//    template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
//    T as_enum(const T& min_enum_value, const T& max_enum_value) const {
//        return double_to_enum(m_value, min_enum_value, max_enum_value);
//    }


    double get() const {
        return m_value;
    }


    // TODO
//    template<typename PivotType>
//    std::optional<PivotType> as_enum(int min_enum_value, int max_enum_value) {
//        if (!m_value.empty())
//            return static_cast<PivotType>(m_value.at(0));
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

} // namespace serialist


// ==============================================================================================

template<>
struct std::is_floating_point<serialist::Facet> : std::true_type {
};

template<>
struct std::is_arithmetic<serialist::Facet> : std::true_type {
};




#endif //SERIALISTLOOPER_FACET_H
