
#ifndef SERIALISTLOOPER_FACET_H
#define SERIALISTLOOPER_FACET_H

#include <vector>
#include <optional>

class Facet {
public:
    explicit Facet(int v) : m_value(static_cast<double>(v)) {}


    explicit Facet(double v) : m_value(v) {}


    explicit Facet(bool v) : m_value(static_cast<double>(v)) {
    }


    // TODO
//    template<typename EnumType>
//    explicit Facet(EnumType v, int min_enum_value, int max_enum_value)
//            : m_value((static_cast<int>(v) - min_enum_value) / (max_enum_value - min_enum_value)) {
//        static_assert(std::is_enum_v<EnumType>, "ctor only supports enum types");
//    }


    explicit operator int() const {
        return static_cast<int>(m_value);
    }


    explicit operator bool() const {
        return m_value > 0.5;
    }


    explicit operator double() const {
        return m_value;
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
    double m_value;

};


#endif //SERIALISTLOOPER_FACET_H
