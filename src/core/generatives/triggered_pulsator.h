
#ifndef SERIALISTLOOPER_TRIGGERED_PULSATOR_H
#define SERIALISTLOOPER_TRIGGERED_PULSATOR_H

#include "core/algo/time/pulse.h"
#include "core/generatives/stereotypes/pulsator_stereotypes.h"


class TriggeredPulsator : public Pulsator {
public:
    explicit TriggeredPulsator(double duration = 1.0)
            : m_duration(duration) {}

    void start(double time) override {
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

    Voice<Trigger> schedule_pulse(double time) {
        return Voice<Trigger>::singular(m_pulses.new_pulse(time, std::nullopt));
    }

    Voice<Trigger> process(double time) override {
        if (!is_running())
            return {};

        if (m_configuration_changed && !m_sample_and_hold) {
            reschedule();
        }
        m_configuration_changed = false;

        auto output = m_pulses.drain_elapsed_as_triggers(time);
    }

    Voice<Trigger> handle_time_skip(double new_time) override {
        // TODO: Ideal strategy would be to reschedule based on elapsed time
        //  (i.e. Pulse.trigger_time - m_last_callback_time), but it also needs to take quantization into consideration
        throw std::runtime_error("AutoPulsator::handle_time_skip not implemented");
    }

    Voice<Trigger> flush() override {
        return m_pulses.flush();
    }

    void set_duration(double duration) {
        m_duration = duration;
        m_configuration_changed = true;
    }

private:
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
    // TODO: double m_sample_and_hold ?

};

#endif //SERIALISTLOOPER_TRIGGERED_PULSATOR_H
