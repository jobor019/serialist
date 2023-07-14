

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
            : m_parameter_handler(id, parent)
            , m_socket_handler(ParameterKeys::GENERATIVE_SOCKETS, m_parameter_handler)
              , m_cursor("cursor", m_socket_handler, cursor)
              , m_interpolation_strategy("interp", m_socket_handler, interp)
              , m_sequence("sequence", m_socket_handler, sequence)
              , m_enabled("enabled", m_socket_handler, enabled) {}


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

    ParameterHandler & get_parameter_handler() override {
        return m_parameter_handler;
    }


    void set_cursor(Node<double>* cursor) { m_cursor = cursor; }


    void set_interpolation_strategy(Node<InterpolationStrategy<T>>* interpolation_strategy) {
        m_interpolation_strategy = interpolation_strategy;
    }


    void set_sequence(Sequence<T>* sequence) { m_sequence = sequence; }


    void set_enabled(Node<bool>* enabled) { m_enabled = enabled; }


    Socket<double>& get_cursor() { return m_cursor; }


    Socket<InterpolationStrategy<T>>& get_interpolation_strategy() { return m_interpolation_strategy; }


    DataSocket<T> get_sequence() { return m_sequence; }


    Socket<bool>& get_enabled() { return m_enabled; }


private:

    bool is_enabled(const TimePoint& t) {
        return m_enabled.process_or(t, true);
    }

    ParameterHandler m_parameter_handler;
    ParameterHandler m_socket_handler;


    Socket<double> m_cursor;
    Socket<InterpolationStrategy<T>> m_interpolation_strategy;
    DataSocket<T> m_sequence;

    Socket<bool> m_enabled;

};


#endif //SERIALISTLOOPER_GENERATOR_H
