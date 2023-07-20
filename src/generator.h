

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

    class GeneratorKeys {
    public:
        static const inline std::string CURSOR = "cursor";
        static const inline std::string INTERP = "interpolator";
        static const inline std::string SEQUENCE = "sequence";
        static const inline std::string ENABLED = "enabled";

        static const inline std::string CLASS_NAME = "generator";
    };

    Generator(const std::string& id
              , ParameterHandler& parent
              , Node<Facet>* cursor = nullptr
              , Node<InterpolationStrategy>* interp = nullptr
              , DataNode<T>* sequence = nullptr
              , Node<Facet>* enabled = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler("", m_parameter_handler, ParameterKeys::GENERATIVE_SOCKETS_TREE)
              , m_cursor(GeneratorKeys::CURSOR, m_socket_handler, cursor)
              , m_interpolation_strategy(GeneratorKeys::INTERP, m_socket_handler, interp)
              , m_sequence(GeneratorKeys::SEQUENCE, m_socket_handler, sequence)
              , m_enabled(GeneratorKeys::ENABLED, m_socket_handler, enabled) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, GeneratorKeys::CLASS_NAME);
    }


    std::vector<T> process(const TimePoint& t) override {
        if (!is_enabled(t))
            return {};

        if (!m_cursor.is_connected())
            return {};

        auto y = m_cursor.process_or(t, Facet(0.0)).get();

        if (!m_interpolation_strategy.is_connected() || !m_sequence.is_connected()) {
            if constexpr (std::is_same_v<T, Facet>) {
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

    void disconnect_if(Generative& connected_to) override {
        m_cursor.disconnect_if(connected_to);
        m_interpolation_strategy.disconnect_if(connected_to);
        m_sequence.disconnect_if(connected_to);
    }

    ParameterHandler & get_parameter_handler() override {
        return m_parameter_handler;
    }


    void set_cursor(Node<Facet>* cursor) { m_cursor = cursor; }


    void set_interpolation_strategy(Node<InterpolationStrategy>* interpolation_strategy) {
        m_interpolation_strategy = interpolation_strategy;
    }


    void set_sequence(Sequence<T>* sequence) { m_sequence = sequence; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    Socket<Facet>& get_cursor() { return m_cursor; }


    Socket<InterpolationStrategy>& get_interpolation_strategy() { return m_interpolation_strategy; }


    DataSocket<T>& get_sequence() { return m_sequence; }


    Socket<Facet>& get_enabled() { return m_enabled; }


private:

    bool is_enabled(const TimePoint& t) {
        return static_cast<bool>(m_enabled.process_or(t, Facet(true)));
    }

    ParameterHandler m_parameter_handler;
    ParameterHandler m_socket_handler;


    Socket<Facet> m_cursor;
    Socket<InterpolationStrategy> m_interpolation_strategy;
    DataSocket<T> m_sequence;

    Socket<Facet> m_enabled;

};


#endif //SERIALISTLOOPER_GENERATOR_H
