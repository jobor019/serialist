

#ifndef SERIALISTLOOPER_GENERATOR_H
#define SERIALISTLOOPER_GENERATOR_H

#include <iomanip>

#include "generative.h"
#include "parameter_policy.h"
#include "sequence.h"
#include "socket_policy.h"

template<typename T>
class Generator : public Node<T> {
public:
    Generator(const std::string& id
              , ParameterHandler& parent
              , Node<double>* cursor = nullptr
              , Node<InterpolationStrategy<T>>* interp = nullptr
              , Sequence<T>* sequence = nullptr
                      , Node<bool>* enabled = nullptr)
            : Node<T>(id, parent)
              , m_cursor("cursor", *this, cursor)
              , m_interpolation_strategy("interp", *this, interp)
              , m_sequence("sequence", *this, sequence)
              , m_enabled("enabled", *this, enabled) {}


    std::vector<T> process(const TimePoint& t) override {
        if (!is_enabled(t))
            return {};

        if (!m_cursor.is_connected())
            return {};

        auto y = m_cursor.process_or(t, 0.0);

        if (!m_interpolation_strategy.is_connected() || !m_sequence.is_connected()) {
            if constexpr (std::is_arithmetic_v<T>) {
                return {static_cast<T>(y)};
            } else {
                return {};
            }
        }

        auto strategy = m_interpolation_strategy.process(t);

        if (strategy.empty())
            return {};


        return m_sequence.process(t, y, strategy.at(0));
    }


    std::vector<Generative*> get_connected() override {
        return Generative::collect_connected(m_cursor.get_connected()
                                             , m_interpolation_strategy.get_connected()
                                             , m_sequence.get_connected());
    }


    void set_cursor(Node<double>* cursor) { m_cursor = cursor; }


    void set_interpolation_strategy(Node<InterpolationStrategy<T>>* interpolation_strategy) {
        m_interpolation_strategy = interpolation_strategy;
    }


    void set_sequence(Sequence<T>* sequence) { m_sequence = sequence; }

    Socket<double>& get_cursor() {return m_cursor; }

    Socket<InterpolationStrategy<T>>& get_interpolation_strategy() { return m_interpolation_strategy; }

    DataSocket<T> get_sequence() { return m_sequence; }

    Socket<bool>& get_enabled() {return m_enabled; }


private:

    bool is_enabled(const TimePoint& t) {
        return m_enabled.process_or(t, true);
    }

    Socket<double> m_cursor;
    Socket<InterpolationStrategy<T>> m_interpolation_strategy;
    DataSocket<T> m_sequence;

    Socket<bool> m_enabled;

};


#endif //SERIALISTLOOPER_GENERATOR_H
