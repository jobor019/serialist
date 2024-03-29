
#ifndef SERIALISTLOOPER_AUTO_PULSATOR_H
#define SERIALISTLOOPER_AUTO_PULSATOR_H

#include "core/collections/scheduler.h"
#include "core/algo/time/pulse.h"
#include "core/algo/time/trigger.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/generatives/stereotypes/pulsator_stereotypes.h"



// ==============================================================================================

class AutoPulsator : public Pulsator {
public:
    explicit AutoPulsator(double duration = 1.0, double legato_amount = 1.0, bool sample_and_hold = true)
            : m_duration(duration)
              , m_legato_amount(legato_amount)
              , m_sample_and_hold(sample_and_hold) {}


    void start(double time) override {
        schedule_pulse(time);
        m_last_callback_time = time;
        m_running = true;
        m_configuration_changed = false;
    }

    Voice<Trigger> stop() override {
        m_running = false;
        return flush();
    }

    Voice<Trigger> flush() override {
        return m_pulses.flush();
    }

    bool is_running() const override {
        return m_running;
    }

    Voice<Trigger> poll(double time) override {
        if (!is_running())
            return {};

        if (m_configuration_changed && !m_sample_and_hold) {
            reschedule();
        }
        m_configuration_changed = false;

        auto output = m_pulses.drain_elapsed_as_triggers(time, true);

        if (time >= m_next_trigger_time) {
            output.append(schedule_pulse(m_next_trigger_time));
        }

        m_last_callback_time = time;
        return output;
    }

    Voice<Trigger> handle_external_triggers(double, const Voice<Trigger>&) override {
        // Not relevant for AutoPulsator
        return {};
    }

    Voice<Trigger> handle_time_skip(double new_time) override {
        // TODO: Ideal strategy would be to reschedule based on elapsed time
        //  (i.e. Pulse.trigger_time - m_last_callback_time), but it also needs to take quantization into consideration
        throw std::runtime_error("AutoPulsator::handle_time_skip not implemented");
    }

    Vec<Pulse<>> export_pulses() override {
        return m_pulses.vec_mut().drain();
    }

    void import_pulses(const Vec<Pulse<>>& pulses) override {
        // adding pulses without defined pulse_off is ok here since we're considering those as elapsed in process()
        m_pulses.vec_mut().extend(pulses);
    }

    void set_duration(double duration) {
        m_duration = utils::clip(duration, {0.0});
        m_configuration_changed = true;
    }

    void set_legato_amount(double legato_amount) {
        m_legato_amount = utils::clip(legato_amount, {0.0});
        m_configuration_changed = true;
    }

    void set_sample_and_hold(bool sample_and_hold) {
        m_sample_and_hold = sample_and_hold;
    }

    double get_duration() const { return m_duration; }

    double get_legato_amount() const { return m_legato_amount; }


private:
    void reschedule() {
        // TODO
        throw std::runtime_error("AutoPulsator::reschedule not implemented");
    }

    Trigger schedule_pulse(double trigger_time) {
        auto trigger = m_pulses.new_pulse(trigger_time, trigger_time + m_duration * m_legato_amount);
        m_next_trigger_time = trigger_time + m_duration;

        return trigger;
    }


    Pulses<> m_pulses;

    double m_duration;
    double m_legato_amount;
    bool m_sample_and_hold;

    bool m_configuration_changed = false;

    double m_next_trigger_time = 0.0;
    double m_last_callback_time = 0.0;
    bool m_running = false;
};


// ==============================================================================================

class AutoPulsatorNode : public PulsatorBase<AutoPulsator> {
public:

    static const inline std::string DURATION = "duration";
    static const inline std::string LEGATO = "legato";

    static const inline std::string CLASS_NAME = "pulsator";

    AutoPulsatorNode(const std::string& id
                     , ParameterHandler& parent
//                 , Node<Trigger>* trigger = nullptr
                     , Node<Facet>* duration = nullptr
                     , Node<Facet>* legato_amount = nullptr
                     , Node<Facet>* enabled = nullptr
                     , Node<Facet>* num_voices = nullptr)
            : PulsatorBase<AutoPulsator>(id, parent, enabled, num_voices, CLASS_NAME)
//              , m_trigger(add_socket(ParameterKeys::TRIGGER, trigger))
              , m_duration(add_socket(DURATION, duration))
              , m_legato_amount(add_socket(LEGATO, legato_amount)) {}

    std::size_t get_voice_count() override {
        return voice_count(
//                m_trigger.voice_count()
                m_duration.voice_count()
                , m_legato_amount.voice_count());

    }

    void update_parameters(std::size_t num_voices, bool size_has_changed) override {
        if (size_has_changed || m_duration.has_changed()) {
            auto duration = m_duration.process().adapted_to(num_voices).firsts_or(1.0);
            get_pulsators_mut().set(&AutoPulsator::set_duration, duration.as_type<double>());
        }

        if (size_has_changed || m_legato_amount.has_changed()) {
            auto legato = m_legato_amount.process().adapted_to(num_voices).firsts_or(1.0);
            get_pulsators_mut().set(&AutoPulsator::set_legato_amount, legato.as_type<double>());
        }
    }

    Voices<Trigger> get_incoming_triggers(const TimePoint&, std::size_t) override {
        return {};
    }

private:
//    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_duration;
    Socket<Facet>& m_legato_amount;
};

#endif //SERIALISTLOOPER_AUTO_PULSATOR_H
