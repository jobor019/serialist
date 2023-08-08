

#ifndef SERIALISTLOOPER_GENERATOR_H
#define SERIALISTLOOPER_GENERATOR_H

#include "generative.h"
#include "parameter_policy.h"
#include "sequence.h"
#include "socket_policy.h"
#include "socket_handler.h"

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
              , Node<Facet>* enabled = nullptr
              , Node<Facet>* num_voices = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler)
              , m_cursor(m_socket_handler.create_socket(GeneratorKeys::CURSOR, cursor))
              , m_interpolation_strategy(m_socket_handler.create_socket(GeneratorKeys::INTERP, interp))
              , m_sequence(m_socket_handler.create_data_socket(GeneratorKeys::SEQUENCE, sequence))
              , m_enabled(m_socket_handler.create_socket(GeneratorKeys::ENABLED, enabled))
              , m_num_voices(m_socket_handler.create_socket(ParameterKeys::NUM_VOICES, num_voices)) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, GeneratorKeys::CLASS_NAME);
    }


    Voices<T> process(const TimePoint& t) override {
        auto num_voices = static_cast<std::size_t>(std::max(1, m_num_voices.process(t, 1).front_or(1)));

        if (!is_enabled(t) || !m_cursor.is_connected()) {
            m_current_value.clear(num_voices);
            return m_current_value;
        }

        std::vector<double> ys = m_cursor.process(t, num_voices).values_or(0.0);

        if (!m_interpolation_strategy.is_connected() || !m_sequence.is_connected()) {
            if constexpr (std::is_same_v<T, Facet>) {
                m_current_value = Voices<T>(Facet::vector_cast(ys));
            } else {
                m_current_value =  Voices<T>(num_voices);
            }
            return m_current_value;
        }

        std::vector<Voice<T>> output;
        output.reserve(num_voices);
        auto strategies = m_interpolation_strategy.process(t, num_voices).fronts();

        for (std::size_t i = 0; i < num_voices; ++i) {
            if (strategies.at(i)) {
                output.emplace_back(m_sequence.process(t, ys.at(i), *strategies.at(i)));
            } else {
                output.emplace_back(Voice<T>::create_empty());
            }
        }

        m_current_value = Voices<T>(output);
        return m_current_value;
    }


    std::vector<Generative*> get_connected() override {
        return m_socket_handler.get_connected();
    }

    void disconnect_if(Generative& connected_to) override {
        m_socket_handler.disconnect_if(connected_to);
    }

    ParameterHandler & get_parameter_handler() override {
        return m_parameter_handler;
    }


    void set_cursor(Node<Facet>* cursor) { m_cursor = cursor; }


    void set_interpolation_strategy(Node<InterpolationStrategy>* is) { m_interpolation_strategy = is; }


    void set_sequence(Sequence<T>* sequence) { m_sequence = sequence; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }

    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<Facet>& get_cursor() { return m_cursor; }


    Socket<InterpolationStrategy>& get_interpolation_strategy() { return m_interpolation_strategy; }


    DataSocket<T>& get_sequence() { return m_sequence; }


    Socket<Facet>& get_enabled() { return m_enabled; }

    Socket<Facet>& get_num_voices() { return m_num_voices; }


private:

    bool is_enabled(const TimePoint& t) {
        return m_enabled.process(t, 1).front_or(true);
    }

    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;


    Socket<Facet>& m_cursor;
    Socket<InterpolationStrategy>& m_interpolation_strategy;
    DataSocket<T>& m_sequence;

    Socket<Facet>& m_enabled;
    Socket<Facet>& m_num_voices;

    Voices<T> m_current_value = Voices<T>(1);

};


#endif //SERIALISTLOOPER_GENERATOR_H
