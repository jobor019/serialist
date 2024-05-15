
#ifndef SERIALISTLOOPER_BEAT_GRID_PULSATOR_H
#define SERIALISTLOOPER_BEAT_GRID_PULSATOR_H

#include "core/algo/time/pulse.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/time/time_specification.h"
#include "core/generatives/stereotypes/pulsator_stereotypes.h"

// TODO: Unify Pulsator class and make this class inherit from it (main issue: start would currently take
//       a TimePoint& as first_pulse_time, which won't work for this class

class BeatPulsator : public Pulsator {
public:
    explicit BeatPulsator(std::unique_ptr<TimeSpecification> ts = std::make_unique<SingleDomainDuration>()
                          , double legato_amount = 1.0
             , bool sample_and_hold = false
    )
            : m_ts(std::move(ts))
              , m_legato_amount(legato_amount)
     , m_sample_and_hold(sample_and_hold)
    {
        assert(ts);
    }

    void start(const TimePoint& time, const std::optional<DomainTimePoint>& first_pulse_time) override {
        (void) first_pulse_time; // TODO: Remove first_pulse_time entirely

        m_last_callback_time = time;
        m_next_trigger_time = m_ts->next(time);
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

    Voice<Trigger> poll(const TimePoint &time) override {
        if (!m_running) {
            return {};
        }

        if (m_configuration_changed && !m_sample_and_hold) {

        }
    }

    Voice<Trigger> handle_external_triggers(const TimePoint &time, const Voice<Trigger> &triggers) override {}

    Voice<Trigger> handle_time_skip(const TimePoint &new_time) override {}

    Vec<Pulse> export_pulses() override {}

    void import_pulses(const Vec<Pulse> &pulses) override {}

    std::optional<DomainTimePoint> next_scheduled_pulse_on() override {}


private:
    void update_next_trigger(const TimePoint& time) {
        m_last_callback_time = time;

//        m_next_trigger_time = m_ts->next()


    }


    Pulses m_pulses;

    std::unique_ptr<TimeSpecification> m_ts;

    double m_legato_amount;
    bool m_sample_and_hold;

    bool m_configuration_changed = false;

    DomainTimePoint m_next_trigger_time = DomainTimePoint::zero();
    TimePoint m_last_callback_time = TimePoint::zero();

    bool m_running = false;
};


// ==============================================================================================

class BeatGridPulsatorNode : public NodeBase<Trigger> {
public:
};

#endif //SERIALISTLOOPER_BEAT_GRID_PULSATOR_H
