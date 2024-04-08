
#ifndef SERIALISTLOOPER_TRIGGERED_PULSATOR_H
#define SERIALISTLOOPER_TRIGGERED_PULSATOR_H

#include "core/algo/time/pulse.h"
#include "core/generatives/stereotypes/pulsator_stereotypes.h"


class TriggeredPulsator : public Pulsator {
public:
    explicit TriggeredPulsator(double duration = 1.0, bool sample_and_hold = true)
            : m_duration(duration)
            , m_sample_and_hold(sample_and_hold) {}

    void start(double time, std::optional<double>) override {
        m_last_callback_time = time;
        m_running = true;
    }

    Voice<Trigger> stop() override {
        m_running = false;
        return flush();
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

        return m_pulses.drain_elapsed_as_triggers(time);
    }

    Voice<Trigger> handle_external_triggers(double time, const Voice<Trigger>& triggers) override {
        if (!is_running())
            return {};

        if (auto index = triggers.index([](const Trigger& t) { return t.is(Trigger::Type::pulse_on); })) {
            return schedule_pulse(time, triggers[*index].get_id());
        }

        return {};
    }

    Voice<Trigger> handle_time_skip(double new_time) override {
        // TODO: Ideal strategy would be to reschedule based on elapsed time
        //  (i.e. Pulse.trigger_time - m_last_callback_time), but it also needs to take quantization into consideration
        (void) new_time;
        throw std::runtime_error("AutoPulsator::handle_time_skip not implemented");
    }

    Voice<Trigger> flush() override {
        return m_pulses.flush();
    }

    void set_duration(double duration) {
        if (!utils::equals(duration, m_duration)) {
            m_duration = duration;
            m_configuration_changed = true;
        }
    }

    void import_pulses(const Vec<Pulse<>> &pulses) override {
        for (auto &pulse : pulses) {
            if (!pulse.has_pulse_off()) {
                m_pulses.vec_mut().append(Pulse<>(pulse.get_id(), pulse.get_trigger_time(), 0.0));
            } else {
                m_pulses.vec_mut().append(pulse);
            }
        }
    }

    Vec<Pulse<>> export_pulses() override {
        return m_pulses.vec_mut().drain();
    }

    std::optional<double> next_scheduled_pulse_on() override {
        return std::nullopt;
    }

private:
        Voice<Trigger> schedule_pulse(double time, std::optional<std::size_t> id) {
        return Voice<Trigger>::singular(m_pulses.new_pulse(time, time + m_duration, id));
    }

    void reschedule() {
        // TODO
        throw std::runtime_error("TriggeredPulsator::reschedule() not implemented");
    }

    Pulses<> m_pulses;

    double m_duration;
    bool m_sample_and_hold;

    bool m_configuration_changed = false;
    double m_last_callback_time = 0.0;
    bool m_running = false;
};

#endif //SERIALISTLOOPER_TRIGGERED_PULSATOR_H
