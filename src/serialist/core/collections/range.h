
#ifndef SERIALISTLOOPER_RANGE_H
#define SERIALISTLOOPER_RANGE_H

#include <type_traits>
#include <cassert>
#include "core/utility/math.h"
#include "vec.h"

namespace serialist {

class RangeUtils {
public:
    RangeUtils() = delete;

    template<typename T>
    static bool is_valid(T start, T end, bool include_start, bool include_end) {
        if constexpr (std::is_floating_point_v<T>) {
            if (include_start && include_end) {
                return start <= end;

            } else if (include_start || include_end) {
                return start < end;

            } else {
                return std::nextafter(start, std::numeric_limits<T>::infinity()) < end;
            }
        } else {
            return start + static_cast<T>(include_start) <= end - static_cast<T>(!include_end);
        }
    }

};


// ==============================================================================================

/**
 *
 * @note: This implementation yields incorrect results for ranges > mantissa(double) (typically 2^53)
 *        a separate implementation will be needed for ranges above this value
 */
template<typename T>
class DiscreteRange {
public:
    static constexpr double DEFAULT_EPSILON = 1e-8;



    static DiscreteRange from_step(T start, T end, T step_size
                                , bool include_end = false, double epsilon = DEFAULT_EPSILON) {
        return DiscreteRange(start
                             , end
                             , static_cast<double>(step_size)
                             , num_steps_of(start, end, static_cast<double>(step_size), include_end)
                             , include_end
                             , epsilon);
    }


    template<typename U = T, std::enable_if_t<!std::is_floating_point_v<U>, int> = 0>
    static DiscreteRange from_step(T start, T end, double step_size
                                , bool include_end = false, double epsilon = DEFAULT_EPSILON) {
        return DiscreteRange(start
                             , end
                             , step_size
                             , num_steps_of(start, end, step_size, include_end)
                             , include_end
                             , epsilon);
    }


    static DiscreteRange from_size(T start, T end, std::size_t num_steps
                                  , bool include_end = false, double epsilon = DEFAULT_EPSILON) {
        return DiscreteRange(start
                        , end
                        , step_size_of(start, end, num_steps, include_end)
                        , num_steps
                        , include_end
                        , epsilon);
    }

    static DiscreteRange<T> deserialize(std::string) {
        throw std::runtime_error("not implemented: deserialize");
    }

    std::string serialize() const {
        throw std::runtime_error("not implemented: serialize");
    }

    DiscreteRange<T> new_adjusted(std::optional<T> new_start, std::optional<T> new_end) const {
        if (!new_start && !new_end) return cloned();

        if (!new_start) new_start = m_start;
        if (!new_end) new_end = m_end;

        return from_step(*new_start, *new_end, m_step_size, m_include_end, m_epsilon);
    }

    DiscreteRange<T> cloned() const {
        return *this;
    }


    bool contains(T value) const {
        (void) value;
        throw std::runtime_error("not implemented: contains"); // TODO


        if constexpr (std::is_floating_point_v<T>) {
            // TODO
        } else {
            // TODO
        }
    }

    T clip(T value) const {
        return utils::clip(value, get_min(), get_max());
    }

    std::size_t map_index(double unit_value) const {
        unit_value = utils::clip(unit_value, 0.0, 1.0);

        return static_cast<std::size_t>(std::floor((m_range * unit_value + m_epsilon) / m_step_size));
    }

    T map(double unit_value) const {
        return at(map_index(unit_value));
    }

    double inverse(T value) const {
        return static_cast<double>(value - m_start) / m_range;
    }

    T at(std::size_t index) const {
        index = std::min(index, m_num_steps - 1);

        auto value =  static_cast<double>(m_start) + m_step_size * static_cast<double>(index);

        if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(std::floor(value + m_epsilon));
        } else {
            return static_cast<T>(value);
        }

    }

    std::size_t size() const {
        return m_num_steps;
    }

    Vec<T> to_vec() const {
        auto v = Vec<T>::allocated(m_num_steps);

        for (std::size_t i = 0; i < m_num_steps; ++i) {
            v.append(at(i));
        }
        return v;
    }

    T get_start() const { return m_start; }

    T get_end() const { return m_end; }

    T get_min() const { return at(0); }

    T get_max() const { return at(m_num_steps - 1); }

    double get_step_size() const {
        return m_step_size;
    }


private:
    DiscreteRange(T start, T end, double step_size, std::size_t num_steps, bool include_end, double epsilon)
            : m_start(start)
              , m_end(end)
              , m_include_end(include_end)
              , m_step_size(step_size)
              , m_num_steps(num_steps)
              , m_range(range_of(m_start, m_end, m_include_end, m_step_size))
              , m_epsilon(epsilon) {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");
        assert(RangeUtils::is_valid(m_start, m_end, false, m_include_end));

//        std::cout << "start: " << m_start << std::endl;
//        std::cout << "end: " << m_end << std::endl;
//        std::cout << "include end: " << m_include_end << std::endl;
//        std::cout << "STEP_SIZE: " << m_step_size << std::endl;
//        std::cout << "NUM_STEPS: " << m_num_steps << std::endl;
//        std::cout << "RANGE: " << m_range << std::endl;
//        std::cout << "epsilon: " << m_epsilon << std::endl;

    }

    static double step_size_of(T start
                               , T end
                               , std::size_t num_steps
                               , bool include_end = false) {
        assert(RangeUtils::is_valid(start, end, false, include_end));
        num_steps = std::max(std::size_t{1}, num_steps - static_cast<std::size_t>(include_end));
        return static_cast<double>(end - start) / static_cast<double>(num_steps);
    }

    static std::size_t num_steps_of(T start
                                     , T end
                                     , double step_size
                                     , bool include_end = false
                                     , double epsilon = DEFAULT_EPSILON) {
        assert(RangeUtils::is_valid(start, end, false, include_end));

        if (step_size <= 0.0) {
            return 1;
        }

        auto num_steps_floating = static_cast<double>(end - start) / step_size + static_cast<double>(include_end);

        return static_cast<std::size_t>(std::floor(num_steps_floating + epsilon));
    }

    static double range_of(T start
                      , T end
                      , bool include_end = false
                      , std::optional<double> step_size = std::nullopt) {
        assert(RangeUtils::is_valid(start, end, false, include_end));

        if (!step_size)
            step_size = step_size_of(start, end, false, include_end);
        else
            step_size = std::max(0.0, *step_size);

        return static_cast<double>(end - start) - (*step_size) * static_cast<double>(!include_end);
    }


    T m_start;
    T m_end;

    bool m_include_end;

    double m_step_size;
    std::size_t m_num_steps;
    double m_range;


    double m_epsilon;
};


// ==============================================================================================

template<typename T>
class ContinuousRange {
public:
    ContinuousRange(T start, T end, bool include_end = true, bool include_start = false)
            : m_start(start), m_end(end), m_include_end(include_end), m_include_start(include_start) {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

        if (!RangeUtils::is_valid(m_start, m_end, m_include_start, m_include_end)) {
            throw std::invalid_argument("Invalid range");
        }
    }

    T map(double unit_value) {
        unit_value = utils::clip(unit_value, 0.0, 1.0);
        return m_start + (m_end - m_start) * unit_value;
    }

    // TODO
//    double inverse(T value) {
//        if constexpr (std::is_integral_v<T>) {
//            if (m_end - m_start <= 0) {
//
//            }
//        }
//        return utils::clip((value - m_start) / (m_end - m_start), 0.0, 1.0);
//    }


    bool is_in(T value) const {
        bool above_start = m_include_start ? value > m_start : value >= m_start;
        bool below_end = m_include_end ? value < m_end : value <= m_end;
        return above_start && below_end;
    }

    // TODO: Vec<T> to_vec(unsigned int num_steps)
    // TODO: Vec<T> to_vec(double step_size)
    // TODO: discretize?


private:
    double m_start;
    double m_end;

    bool m_include_end;
    bool m_include_start;
};

} // namespace serialist

#endif //SERIALISTLOOPER_RANGE_H
