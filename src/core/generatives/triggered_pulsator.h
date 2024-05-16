
#ifndef SERIALISTLOOPER_TRIGGERED_PULSATOR_H
#define SERIALISTLOOPER_TRIGGERED_PULSATOR_H

#include "core/algo/time/pulse.h"
#include "core/generatives/stereotypes/pulsator_stereotypes.h"


class TriggeredPulsator : public Pulsator {
public:
    explicit TriggeredPulsator(const DomainDuration& duration = DomainDuration(1.0, DomainType::ticks)
                               , bool sample_and_hold = true)
            : m_duration(duration)
              , m_sample_and_hold(sample_and_hold) {}

    void start(const TimePoint& time, const std::optional<DomainTimePoint>&) override {
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

    Voice<Trigger> poll(const TimePoint& time) override {
        if (!is_running())
            return {};

        if (m_configuration_changed && !m_sample_and_hold) {
            reschedule();
        }
        m_configuration_changed = false;

        return m_pulses.drain_elapsed_as_triggers(time);
    }

    Voice<Trigger> handle_external_triggers(const TimePoint& time, const Voice<Trigger>& triggers) override {
        if (!is_running())
            return {};

        if (auto index = triggers.index([](const Trigger& t) { return t.is(Trigger::Type::pulse_on); })) {
            return schedule_pulse(time, triggers[*index].get_id());
        }

        return {};
    }

    Voice<Trigger> handle_time_skip(const TimePoint& new_time) override {
        // TODO: Ideal strategy would be to reschedule based on elapsed time
        //  (i.e. Pulse.trigger_time - m_last_callback_time), but it also needs to take quantization into consideration
        (void) new_time;
        throw std::runtime_error("AutoPulsator::handle_time_skip not implemented");
    }

    Voice<Trigger> flush() override {
        return m_pulses.flush();
    }

    void set_duration(const Period& duration) {
        m_duration = duration;
        m_configuration_changed = true;
    }

    void import_pulses(const Vec<Pulse>& pulses) override {
        for (auto& pulse: pulses) {
            if (!pulse.has_pulse_off()) {
                m_pulses.vec_mut().append(Pulse(pulse.get_id(), pulse.get_trigger_time(), DomainTimePoint::zero()));
            } else {
                m_pulses.vec_mut().append(pulse);
            }
        }
    }

    Vec<Pulse> export_pulses() override {
        return m_pulses.vec_mut().drain();
    }

    std::optional<DomainTimePoint> next_scheduled_pulse_on() override {
        return std::nullopt;
    }

private:
    Voice<Trigger> schedule_pulse(const TimePoint& time, std::optional<std::size_t> id) {
        auto pulse_on_time = DomainTimePoint::from_time_point(time, m_duration.get_type());
        auto pulse_off_time = m_duration.next(time, false);
        return Voice<Trigger>::singular(m_pulses.new_pulse(pulse_on_time, pulse_off_time, id));
    }

    void reschedule() {
        // TODO
        throw std::runtime_error("TriggeredPulsator::reschedule() not implemented");
    }

    Pulses m_pulses;

    Period m_duration; // offset determined by input from user, so this pulsator is always free
    bool m_sample_and_hold;

    bool m_configuration_changed = false;
    TimePoint m_last_callback_time = TimePoint::zero();
    bool m_running = false;
};

#endif //SERIALISTLOOPER_TRIGGERED_PULSATOR_H
