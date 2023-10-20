
#ifndef SERIALISTLOOPER_PULSATOR_H
#define SERIALISTLOOPER_PULSATOR_H

#include "core/generative.h"
#include "core/algo/time/events.h"
#include "core/algo/time/scheduler.h"
#include "core/param/socket_policy.h"
#include "core/param/socket_handler.h"
#include "core/algo/time/time_gate.h"
#include "core/generatives/variable.h"

class Pulsator : public Node<TriggerEvent> {
public:

    static inline constexpr double JUMP_THRESHOLD_TICKS = 1.0;

    class PulsatorKeys {
    public:
        static const inline std::string TRIGGER = "trigger";
        static const inline std::string DUTY = "duty";
        static const inline std::string QUANTIZATION = "quantization";

        static const inline std::string CLASS_NAME = "pulsator";
    };


    Pulsator(const std::string& id
             , ParameterHandler& parent
             , Node<Facet>* trigger_interval = nullptr
             , Node<Facet>* duty_cycle = nullptr
             , Node<Facet>* enabled = nullptr
             , Node<Facet>* num_voices = nullptr)
//             , Node<Facet>* quantization_level = nullptr
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler)
              , m_trigger_interval(m_socket_handler.create_socket(PulsatorKeys::TRIGGER, trigger_interval))
              , m_duty_cycle(m_socket_handler.create_socket(PulsatorKeys::DUTY, duty_cycle))
//              , m_quantization_level(m_socket_handler.create_socket(PulsatorKeys::QUANTIZATION, quantization_level))
              , m_enabled(m_socket_handler.create_socket(ParameterKeys::ENABLED, enabled))
              , m_num_voices(m_socket_handler.create_socket(ParameterKeys::NUM_VOICES, num_voices)) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, PulsatorKeys::CLASS_NAME);
    }


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    Voices<TriggerEvent> process() override {
        // TODO: Pausing is not implemented intelligently: triggers should probably be removed when disabled
        //    and requeued when enabled again. Also need to account for changes in voices that occurred during this time

        auto t = m_time_gate.pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;


        bool enabled = is_enabled();
        if (!enabled) {
            if (m_previous_enabled_state) {
                m_current_value = flush(m_current_value.size()); // flush based on old voice count
            } else {
                m_current_value = Voices<TriggerEvent>::create_empty_like();
            }
            m_previous_enabled_state = enabled;
            return m_current_value;
        }


        auto voices = m_num_voices.process();
        auto trigger_intervals = m_trigger_interval.process();
        auto duty_cycles = m_duty_cycle.process();

        auto num_voices = compute_voice_count(voices, trigger_intervals.size(), duty_cycles.size());

        auto intervals = trigger_intervals.adapted_to(num_voices).values_or(1.0);
        auto dutys = duty_cycles.adapted_to(num_voices).values_or(1.0, {0.0}, {1.0});

        if (!m_previous_enabled_state || m_current_value.size() != num_voices) {
            schedule_missing_triggers(*t, num_voices);
        }


        m_current_value = process_triggers(*t, num_voices, intervals, dutys);
        m_previous_enabled_state = enabled;

        return m_current_value;
    }


    void disconnect_if(Generative& connected_to) override { m_socket_handler.disconnect_if(connected_to); }


    std::vector<Generative*> get_connected() override { return m_socket_handler.get_connected(); }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


    void set_trigger_interval(Node<Facet>* trigger_interval) { m_trigger_interval = trigger_interval; }


    void set_duty_cycle(Node<Facet>* duty_cycle) { m_duty_cycle = duty_cycle; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


//    void set_quantization_level(Node<Facet>* quantization_level) { m_quantization_level = quantization_level; }


    Socket<Facet>& get_trigger_interval() { return m_trigger_interval; }


    Socket<Facet>& get_duty_cycle() { return m_duty_cycle; }


    Socket<Facet>& get_enabled() { return m_enabled; }


    Socket<Facet>& get_num_voices() { return m_num_voices; }


//    Socket<Facet>& get_quantization_level() { return m_quantization_level; }



private:
    bool is_enabled() { return m_enabled.process(1).first_or(true); }


    void schedule_missing_triggers(const TimePoint& t, std::size_t new_voice_count) {
        auto next_trigger_time = t.next_tick_of({1, 4}); // TODO: Should be a user parameter / Variable<Facet>
        auto voice_has_trigger = has_triggers(new_voice_count);
        for (std::size_t i = 0; i < new_voice_count; ++i) {
            if (!voice_has_trigger.at(i)) {
                m_scheduler.add_event(std::make_unique<TriggerEvent>(next_trigger_time
                                                                     , TriggerEvent::Type::pulse_on
                                                                     , static_cast<int>(i)));
            }
        }
    }


    Voices<TriggerEvent> flush(std::size_t num_voices) {
        auto triggers = m_scheduler.flush();

        auto output = Voices<TriggerEvent>(num_voices);

        for (auto& trigger: triggers) {
            if (trigger->get_type() == TriggerEvent::Type::pulse_off) {
                output.append_to(static_cast<std::size_t>(trigger->get_id()), *trigger);
            }
        }

        return output;
    }


    Voices<TriggerEvent> process_triggers(const TimePoint& t
                                          , std::size_t num_voices
                                          , std::vector<double> trigger_intervals
                                          , std::vector<double> duty_cycles) {
        auto triggers = m_scheduler.poll(t);

        if (triggers.empty()) {
            return Voices<TriggerEvent>::create_empty_like();
        }

        auto output = Voices<TriggerEvent>(num_voices);

        for (auto& trigger: triggers) {
            auto id = static_cast<std::size_t>(trigger->get_id());
            if (id < num_voices) { // discard any lingering triggers of old voices
                output.append_to(id, *trigger);

                if (trigger->get_type() == TriggerEvent::Type::pulse_on) {
                    if (t.get_tick() - trigger->get_time() > JUMP_THRESHOLD_TICKS) {
                        // Gap larger than reasonable drift occurred: do not apply drift compensation
                        throw std::runtime_error("scheduling jump/gap: not implemented"); // TODO
                    } else {
                        // TODO: This could be a bit problematic for 0 < values < JUMP_THRESHOLD_TICKS
                        schedule_pulse(trigger->get_time(), trigger_intervals.at(id), duty_cycles.at(id)
                                       , trigger->get_id());
                    }

                }
            }
        }
        return output;
    }


    std::vector<bool> has_triggers(std::size_t num_voices) const {
        std::vector<bool> has_pulse_on(num_voices, false);

        for (auto& trigger: m_scheduler.peek()) {
            auto id = static_cast<std::size_t>(trigger->get_id());
            if (trigger->get_type() == TriggerEvent::Type::pulse_on && id < num_voices) {
                has_pulse_on.at(id) = true;
            }
        }

        return has_pulse_on;
    }


    void schedule_pulse(double now, double interval, double duty, int id) {
        auto next_pulse_on = now + interval;
        auto next_pulse_off = now + interval * duty;

        // if simultaneous: place pulse_off before pulse_on in scheduler's internal vector
        m_scheduler.add_event(std::make_unique<TriggerEvent>(next_pulse_off, TriggerEvent::Type::pulse_off, id));
        m_scheduler.add_event(std::make_unique<TriggerEvent>(next_pulse_on, TriggerEvent::Type::pulse_on, id));
    }


    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

    Scheduler<TriggerEvent> m_scheduler;

    Socket<Facet>& m_trigger_interval;
    Socket<Facet>& m_duty_cycle;
//    Socket<Facet>& m_quantization_level; // TODO

    Socket<Facet>& m_enabled;
    Socket<Facet>& m_num_voices;

    Voices<TriggerEvent> m_current_value = Voices<TriggerEvent>::create_empty_like();

    bool m_previous_enabled_state = false; // enforce trigger queueing on first process call

    TimeGate m_time_gate;


};

#endif //SERIALISTLOOPER_PULSATOR_H
