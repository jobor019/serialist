#ifndef SERIALIST_INDEX_H
#define SERIALIST_INDEX_H
#include "temporal/phase.h"


namespace serialist {
// ==============================================================================================

class Index {
public:
    static constexpr double EPSILON = 1e-8;

    enum class Strategy { cont, mod, clip, pass };

    explicit Index(std::size_t value = 0) : m_value(value) {}

    static Index zero() { return Index{0}; }


    static Index from_index_facet(double index) {
        if (index < 0.0) {
            return zero();
        }
        return Index(static_cast<std::size_t>(std::round(index)));
    }


    static Index from_index_facet(const Facet& value) { return from_index_facet(value.get()); }


    static Index from_phase(const Phase& phase, std::size_t map_size) {
        return from_phase_like(phase.get(), map_size);
    }


    static Index from_phase_like(double phase_like, std::size_t map_size) {
        return Index(index_op(phase_like, map_size));
    }


    static std::size_t index_op(double position, std::size_t map_size, double epsilon = EPSILON) {
        if (map_size == 0) {
            return 0; // technically invalid, but inconvenient to use std::nullopt only for this particular case
        }

        auto size =  static_cast<double>(map_size);
        auto [q, r] = utils::divmod<double>(position, 1.0);

        auto q_part = static_cast<std::size_t>(std::round(q) * size);
        return q_part + static_cast<std::size_t>(std::floor(r * size + epsilon));
    }


    explicit operator std::string() const { return "Index(" + std::to_string(m_value) + ")"; }

    friend std::ostream& operator<<(std::ostream& os, const Index& index) {
        os << static_cast<std::string>(index) << ")";
        return os;
    }


    std::size_t get_raw() const { return m_value; }
    std::size_t get_cont() const { return get_raw(); }
    std::size_t get_mod(std::size_t size) const { return m_value % size; }
    std::size_t get_clip(std::size_t size) const { return std::min(m_value, size - 1); }


    std::optional<std::size_t> get_pass(std::size_t size) const {
        if (m_value < size)
            return m_value;
        return std::nullopt;
    }


    std::optional<std::size_t> get(std::size_t size, Strategy strategy = Strategy::cont) const {
        switch (strategy) {
            case Strategy::cont:
                return get_cont();
            case Strategy::mod:
                return get_mod(size);
            case Strategy::clip:
                return get_clip(size);
            case Strategy::pass:
                return get_pass(size);
        }
        throw std::runtime_error("Unknown strategy");
    }

private:
    std::size_t m_value;
};
}

#endif //SERIALIST_INDEX_H
