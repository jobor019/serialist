

#ifndef SERIALIST_LOOPER_PHASOR_H
#define SERIALIST_LOOPER_PHASOR_H

#include <cmath>
#include <iostream>
#include <iomanip>
#include "core/utility/math.h"
#include "time_point.h"
#include "core/utility/stateful.h"
#include "core/temporal/phase.h"
#include "core/temporal/time_point_generators.h"
#include <map>

namespace serialist {

enum class PaMode {
    transport_locked
    , free_periodic
    , triggered
};

struct PaParameters {
    static constexpr double DEFAULT_PERIOD = 1.0;
    static const inline DomainType DEFAULT_PERIOD_TYPE = DomainType::ticks;
    static constexpr double DEFAULT_OFFSET = 0.0;
    static const inline DomainType DEFAULT_OFFSET_TYPE = DomainType::ticks;
    static constexpr double DEFAULT_STEP_SIZE = 0.1;

    WithChangeFlag<DomainDuration> period{DomainDuration{DEFAULT_PERIOD, DEFAULT_PERIOD_TYPE}};
    WithChangeFlag<DomainDuration> offset{DomainDuration{DEFAULT_OFFSET, DEFAULT_OFFSET_TYPE}};
    WithChangeFlag<double> step_size{DEFAULT_STEP_SIZE};

    static void clear_flags(PaParameters& p) {
        p.period.clear_flag();
        p.offset.clear_flag();
        p.step_size.clear_flag();
    }
};


// ==============================================================================================

struct PaState {
    std::optional<double> x = std::nullopt;
    bool has_trigger = false;
    std::optional<TimePoint> previous_callback = std::nullopt;
};


// ==============================================================================================

class PaStrategy {
public:
    PaStrategy() = default;

    virtual ~PaStrategy() = default;

    PaStrategy(const PaStrategy&) = delete;

    PaStrategy& operator=(const PaStrategy&) = delete;

    PaStrategy(PaStrategy&&) noexcept = default;

    PaStrategy& operator=(PaStrategy&&) noexcept = default;

    virtual double process(const TimePoint& t, PaState& s, PaParameters& p) = 0;

    virtual void clear(PaState& s) {
        s.previous_callback = std::nullopt;
        s.x = std::nullopt;
    }

    static double offset_as_phase(const TimePoint& t, const PaParameters& p) {
        auto type = p.period->get_type();
        auto period = p.period->get_value();
        auto offset = p.offset->as_type(type, t.get_meter()).get_value();

        if (utils::equals(period, 0.0)) {
            return Phase::phase_mod(offset);
        }

        auto q = offset / std::abs(period);
        return Phase::phase_mod(q);
    }

    static double phasor_value_offset(const TimePoint&, const PaParameters& p) {
        auto offset = p.offset->get_value();
        return Phase::phase_mod(offset);
    }
};


// ==============================================================================================

class TriggeredPa : public PaStrategy {
public:
    double process(const TimePoint& t, PaState& s, PaParameters& p) override {
        if (!s.x) {
            // first callback => set to initial phase
            s.x = offset_as_phase(t, p);
        } else if (s.has_trigger) {
            s.x = Phase::phase_mod(*s.x + *p.step_size);
        }

        return *s.x;
    }
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class FreePeriodPa : public PaStrategy {
public:
    double process(const TimePoint& t, PaState& s, PaParameters& p) override {
        if (!s.x) {
            // first callback => set to initial phase
            s.x = phasor_value_offset(t, p);
        } else {
            assert(s.previous_callback.has_value()); // invariant: s.x.has_value() <-> s.previous_callback.has_value()

            auto type = p.period->get_type();
            auto dt = t.get(type) - s.previous_callback->get(type);

            // time skip occurred
            if (dt < 0.0) {
                s.x = phasor_value_offset(t, p);;
            } else if (!utils::equals(p.period->get_value(), 0.0)) {
                auto dx = dt / p.period->get_value();
                s.x = Phase::phase_mod(*s.x + dx);
            } // else: no change, output previous value
        }

        return *s.x;
    }

};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class TransportLockedPa : public PaStrategy {
public:
    double process(const TimePoint& t, PaState& s, PaParameters& p) override {
        s.x = TransportLocked::phase_of(t, *p.period, *p.offset);
        return *s.x;
    }
};


// ==============================================================================================


class PhaseAccumulator {
public:
    static const inline PaMode DEFAULT_MODE = PaMode::transport_locked;

    PhaseAccumulator() = default;

    ~PhaseAccumulator() = default;

    PhaseAccumulator(const PhaseAccumulator& other)
            : m_params(other.m_params)
              , m_state(other.m_state)
              , m_mode(other.m_mode) {}

    PhaseAccumulator& operator=(const PhaseAccumulator& other) {
        if (this == &other) return *this;
        m_params = other.m_params;
        m_state = other.m_state;
        m_mode = other.m_mode;
        return *this;
    }

    PhaseAccumulator(PhaseAccumulator&&) noexcept = default;

    PhaseAccumulator& operator=(PhaseAccumulator&&) noexcept = default;

    double process(const TimePoint& t, bool has_trigger) {
        m_state.has_trigger = has_trigger;
        auto phase = active_strategy().process(t, m_state, m_params);

        m_state.previous_callback = t;
        m_state.has_trigger = false;
        PaParameters::clear_flags(m_params);

        return phase;
    }

    void reset() {
        m_state.previous_callback = std::nullopt;
        m_state.x = std::nullopt;
    }

    void set_period(const DomainDuration& period) { m_params.period = period; }

    void set_offset(const DomainDuration& offset) { m_params.offset = offset; }

    void set_step_size(double step_size) { m_params.step_size = step_size; }

    void set_mode(PaMode mode) { m_mode = mode; }

private:

    PaStrategy& active_strategy() {
        return *m_strategies[m_mode];
    }

    static std::map<PaMode, std::unique_ptr<PaStrategy>> make_strategies() {
        std::map<PaMode, std::unique_ptr<PaStrategy>> strategies;
        strategies[PaMode::transport_locked] = std::make_unique<TransportLockedPa>();
        strategies[PaMode::free_periodic] = std::make_unique<FreePeriodPa>();
        strategies[PaMode::triggered] = std::make_unique<TriggeredPa>();
        return strategies;
    }

    std::map<PaMode, std::unique_ptr<PaStrategy>> m_strategies = make_strategies();
    PaParameters m_params;
    PaState m_state;
    PaMode m_mode = DEFAULT_MODE;
};

} // namespace serialist

#endif //SERIALIST_LOOPER_PHASOR_H
