

#ifndef SERIALISTLOOPER_GENERATOR_RM_H
#define SERIALISTLOOPER_GENERATOR_RM_H

#include "core/generative.h"
#include "core/generatives/sequence.h"
#include "core/param/parameter_policy.h"
#include "core/param/socket_policy.h"
#include "core/param/socket_handler.h"
#include "core/utility/time_gate.h"

template<typename T>
class Generator : public Node<T> {
public:

    class GeneratorKeys {
    public:
        static const inline std::string TRIGGER = "trigger";
        static const inline std::string CURSOR = "cursor";
        static const inline std::string INTERP = "interpolator";
        static const inline std::string SEQUENCE = "sequence";
        static const inline std::string ENABLED = "enabled";

        static const inline std::string CLASS_NAME = "generator";
    };


    Generator(const std::string& id
              , ParameterHandler& parent
//              , Node<TriggerEvent>* trigger = nullptr
              , Node<Facet>* cursor = nullptr
              , Node<InterpolationStrategy>* interp = nullptr
              , Node<T>* sequence = nullptr
              , Node<Facet>* enabled = nullptr
              , Node<Facet>* num_voices = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler)
//              , trigger(m_socket_handler.create_socket(GeneratorKeys::TRIGGER, trigger))
              , m_cursor(m_socket_handler.create_socket(GeneratorKeys::CURSOR, cursor))
              , m_interpolation_strategy(m_socket_handler.create_socket(GeneratorKeys::INTERP, interp))
              , m_sequence(m_socket_handler.create_socket(GeneratorKeys::SEQUENCE, sequence))
              , m_enabled(m_socket_handler.create_socket(GeneratorKeys::ENABLED, enabled))
              , m_num_voices(m_socket_handler.create_socket(ParameterKeys::NUM_VOICES, num_voices)) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, GeneratorKeys::CLASS_NAME);
    }


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    Voices<T> process() override {
        auto t = m_time_gate.pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        if (!is_enabled() /* || !trigger.is_connected() */) {
            m_current_value = Voices<Facet>::create_empty_like();
            return m_current_value;
        }

        if (!m_interpolation_strategy.is_connected() || !m_sequence.is_connected()) {
            m_current_value = process_without_sequence();
            return m_current_value;
        }

        m_current_value = process_with_sequence();
        return m_current_value;
    }


    std::vector<Generative*> get_connected() override {
        return m_socket_handler.get_connected();
    }


    void disconnect_if(Generative& connected_to) override {
        m_socket_handler.disconnect_if(connected_to);
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    void set_cursor(Node<Facet>* cursor) { m_cursor = cursor; }


    void set_interpolation_strategy(Node<InterpolationStrategy>* is) { m_interpolation_strategy = is; }


    void set_sequence(Sequence<T>* sequence) { m_sequence = sequence; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<Facet>& get_cursor() { return m_cursor; }


    Socket<InterpolationStrategy>& get_interpolation_strategy() { return m_interpolation_strategy; }


    Socket<T>& get_sequence() { return m_sequence; }


    Socket<Facet>& get_enabled() { return m_enabled; }


    Socket<Facet>& get_num_voices() { return m_num_voices; }


private:

    bool is_enabled() { return m_enabled.process(1).first_or(true); }


    Voices<T> process_without_sequence() {
        auto voices = m_num_voices.process();
        auto y = m_cursor.process();

        auto num_voices = Generative::compute_voice_count(voices, y.size());

        std::vector<double> ys = m_cursor.process(num_voices).values_or(0.0);

        if constexpr (std::is_same_v<T, Facet>) {
            m_current_value = Voices<T>(Facet::vector_cast(ys));
        } else {
            m_current_value = Voices<T>(num_voices);
        }
        return m_current_value;
    }


    Voices<T> process_with_sequence() {
        auto voices = m_num_voices.process();
        auto cursor = m_cursor.process();
        auto strategy = m_interpolation_strategy.process();

        auto num_voices = Node<T>::compute_voice_count(voices, cursor.size(), strategy.size());
        std::vector<double> ys = cursor.adapted_to(num_voices).values_or(0.0);
        auto strategies = strategy.adapted_to(num_voices).firsts();

        std::vector<Voice<T>> output;
        output.reserve(num_voices);

        for (std::size_t i = 0; i < num_voices; ++i) {
            if (strategies.at(i)) {
                output.emplace_back(m_sequence.process(ys.at(i), *strategies.at(i)));
            } else {
                output.emplace_back(Voice<T>::create_empty());
            }
        }

        m_current_value = Voices<T>(output);
        return m_current_value;

    }


    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;


//    Socket<TriggerEvent>& trigger;
    Socket<Facet>& m_cursor;
    Socket<InterpolationStrategy>& m_interpolation_strategy;
    Socket<T>& m_sequence;

    Socket<Facet>& m_enabled;
    Socket<Facet>& m_num_voices;

    Voices<T> m_current_value = Voices<T>(1);
    TimeGate m_time_gate;

};


#endif //SERIALISTLOOPER_GENERATOR_RM_H
