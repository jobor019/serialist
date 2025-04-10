
#ifndef SERIALISTLOOPER_OSCILLATOR_H
#define SERIALISTLOOPER_OSCILLATOR_H

#include "core/algo/random/random.h"
#include "core/temporal/filters.h"
#include "core/temporal/phase_accumulator.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/types/trigger.h"
#include "core/collections/multi_voiced.h"
#include "sequence.h"
#include "variable.h"

namespace serialist {

class Waveform {
public:
    enum class Mode {
        phasor
        , sin
        , square
        , tri
        , white_noise
        , random_walk
    };

    static constexpr double DEFAULT_DUTY = 0.5;
    static constexpr double DEFAULT_CURVE = 1.0;
    static const inline Mode DEFAULT_MODE = Mode::phasor;


    explicit Waveform(std::optional<int> seed = std::nullopt) : m_random(seed) {}

    double process(double x) {
        switch (m_type) {
            case Mode::phasor:
                return x;
            case Mode::sin:
                return sin(x);
            case Mode::square:
                return square(x, m_duty);
            case Mode::tri:
                return tri(x, m_duty, m_curve);
            case Mode::white_noise:
                return white_noise();
            case Mode::random_walk:
                return random_walk();
        }
    }

    static double sin(double phase) {
        return 0.5 * -std::cos(2 * M_PI * phase) + 0.5;
    }


    static double square(double phase, double duty) {
        return static_cast<double>(phase >= duty);
    }


    static double tri(double phase, double duty, double curve) {
        if (duty < 1e-8) {                  // duty = 0 => negative phase only (avoid div0)
            return std::pow(1 - phase, curve);
        } else if (phase <= duty) {             // positive phase
            return std::pow(phase / duty, curve);
        } else {                            // negative phase
            return std::pow(1 - (phase - duty) / (1 - duty), curve);
        }
    }


    double white_noise() {
        return m_random.next();
    }


    static double random_walk() {
        throw std::runtime_error("not implemented"); // TODO
//        double next = current_ + distribution_(rng_);
//        if (next > 1.0) {
//            current_ = 2.0 - next;
//        } else if (next < 0.0) {
//            current_ = -next;
//        } else {
//            current_ = next;
//        }
//        return current_;
    }

    void set_type(Mode type) { m_type = type; }

    void set_duty(double duty) { m_duty = duty; }

    void set_curve(double curve) { m_curve = curve; }

private:
    Random m_random;

    Mode m_type = DEFAULT_MODE;
    double m_duty = DEFAULT_DUTY;
    double m_curve = DEFAULT_CURVE;

};


// ==============================================================================================

class Oscillator {
public:
    explicit Oscillator(std::optional<int> seed = std::nullopt) : m_waveform(seed) {}

    double process(const TimePoint& t, bool has_trigger) {
        auto x = m_pa.process(t, has_trigger);
        auto y = m_waveform.process(x);
        return m_lpf.process(t, y);
    }

    void reset() {
        m_pa.reset();
        m_lpf.reset();
    }

    void set_waveform(Waveform::Mode type) { m_waveform.set_type(type); }

    void set_pa_mode(PaMode mode) {
        m_pa.set_mode(mode);
        m_lpf.set_unit_stepped(mode == PaMode::triggered);
    }

    void set_pa_period(const DomainDuration& d) { m_pa.set_period(d); }

    void set_step_size(double step_size) { m_pa.set_step_size(step_size); }

    void set_pa_offset(const DomainDuration& d) { m_pa.set_offset(d); }

    void set_duty(double duty) { m_waveform.set_duty(duty); }

    void set_curve(double curve) { m_waveform.set_curve(curve); }

    void set_tau(const DomainDuration& tau) { m_lpf.set_tau(tau); }

private:
    PhaseAccumulator m_pa;
    Waveform m_waveform;
    FilterSmoo m_lpf;
};


// ==============================================================================================

class OscillatorNode : public NodeBase<Facet> {
public:
    class OscillatorKeys {
    public:
        OscillatorKeys() = delete;

        static const inline std::string TRIGGER = "trigger";
        static const inline std::string MODE = "mode";
        static const inline std::string WAVEFORM = "WAVEFORM";
        static const inline std::string PERIOD = "period";
        static const inline std::string PERIOD_TYPE = "period_type";
        static const inline std::string OFFSET = "offset";
        static const inline std::string OFFSET_TYPE = "offset_type";
        static const inline std::string STEP_SIZE = "step_size";
        static const inline std::string DUTY = "duty";
        static const inline std::string CURVE = "curve";
        static const inline std::string PHASE = "phase";
        static const inline std::string TAU = "tau";
        static const inline std::string TAU_TYPE = "tau_type";
        static const inline std::string RESET = "reset";

        static const inline std::string CLASS_NAME = "oscillator";
    };

    OscillatorNode(const std::string& id
                   , ParameterHandler& parent
                   , Node<Trigger>* trigger = nullptr
                   , Node<Facet>* mode = nullptr
                   , Node<Facet>* waveform = nullptr
                   , Node<Facet>* period = nullptr
                   , Node<Facet>* period_type = nullptr
                   , Node<Facet>* offset = nullptr
                   , Node<Facet>* offset_type = nullptr
                   , Node<Facet>* step_size = nullptr
                   , Node<Facet>* duty = nullptr
                   , Node<Facet>* curve = nullptr
                   , Node<Facet>* tau = nullptr
                   , Node<Facet>* tau_type = nullptr
                   , Node<Trigger>* reset = nullptr
                   , Node<Facet>* enabled = nullptr
                   , Node<Facet>* num_voices = nullptr)
            : NodeBase<Facet>(id, parent, enabled, num_voices, OscillatorKeys::CLASS_NAME)
              , m_trigger(add_socket(OscillatorKeys::TRIGGER, trigger))
              , m_mode(add_socket(OscillatorKeys::MODE, mode))
              , m_waveform(add_socket(OscillatorKeys::WAVEFORM, waveform))
              , m_period(add_socket(OscillatorKeys::PERIOD, period))
              , m_period_type(add_socket(OscillatorKeys::PERIOD_TYPE, period_type))
              , m_offset(add_socket(OscillatorKeys::OFFSET, offset))
              , m_offset_type(add_socket(OscillatorKeys::OFFSET_TYPE, offset_type))
              , m_step_size(add_socket(OscillatorKeys::STEP_SIZE, step_size))
              , m_duty(add_socket(OscillatorKeys::DUTY, duty))
              , m_curve(add_socket(OscillatorKeys::CURVE, curve))
              , m_tau(add_socket(OscillatorKeys::TAU, tau))
              , m_tau_type(add_socket(OscillatorKeys::TAU_TYPE, tau_type))
              , m_reset(add_socket(OscillatorKeys::RESET, reset)) {}

    Voices<Facet> process() override {
        auto t = pop_time();
        if (!t)
            return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        auto reset_triggers = m_reset.process();
        if (Trigger::contains_pulse_on(reset_triggers)) {
            reset();
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like())
            return m_current_value;

        auto num_voices = get_voice_count();

        bool resized = num_voices != m_oscillators.size();
        if (resized) {
            m_oscillators.resize(num_voices);
        }

        update_parameters(num_voices, resized);

        trigger.adapted_to(num_voices);

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            bool has_trigger = Trigger::contains_pulse_on(trigger[i]);
            output[i].append(static_cast<Facet>(m_oscillators[i].process(*t, has_trigger)));
        }

        m_current_value = std::move(output);
        return m_current_value;
    }

private:
    std::size_t get_voice_count() {
        return voice_count(m_trigger.voice_count()
                           , m_period.voice_count()
                           , m_offset.voice_count()
                           , m_step_size.voice_count()
                           , m_duty.voice_count()
                           , m_curve.voice_count()
                           , m_tau.voice_count());
    }

    void update_parameters(std::size_t num_voices, bool resized) {
        m_oscillators.set(&Oscillator::set_pa_mode
                          , NodeBase<Facet>::adapted(m_mode.process(), num_voices, PhaseAccumulator::DEFAULT_MODE));

        m_oscillators.set(&Oscillator::set_waveform
                          , NodeBase<Facet>::adapted(m_waveform.process(), num_voices, Waveform::DEFAULT_MODE));

        // TODO: This has_changed is completely redundant: we've already called process() in get_voice_count,
        //       so has_changed is always false here
        if (resized || m_period.has_changed() || m_period_type.has_changed()) {
            auto period = m_period.process().adapted_to(num_voices).firsts_or(PaParameters::DEFAULT_PERIOD);
            auto period_type = m_period_type.process().first_or(PaParameters::DEFAULT_PERIOD_TYPE);

            for (std::size_t i = 0; i < num_voices; ++i) {
                m_oscillators[i].set_pa_period(DomainDuration{period[i], period_type});
            }
        }

        if (resized || m_offset.has_changed() || m_offset_type.has_changed()) {
            auto offset = m_offset.process().adapted_to(num_voices).firsts_or(PaParameters::DEFAULT_OFFSET);
            auto offset_type = m_offset_type.process().first_or(PaParameters::DEFAULT_OFFSET_TYPE);

            for (std::size_t i = 0; i < num_voices; ++i) {
                m_oscillators[i].set_pa_offset(DomainDuration{offset[i], offset_type});
            }
        }

        m_oscillators.set(&Oscillator::set_step_size
                          , NodeBase<Facet>::adapted(m_step_size.process(), num_voices
                                                     , PaParameters::DEFAULT_STEP_SIZE));

        m_oscillators.set(&Oscillator::set_duty
                          , NodeBase<Facet>::adapted(m_duty.process(), num_voices, Waveform::DEFAULT_DUTY));

        m_oscillators.set(&Oscillator::set_curve
                          , NodeBase<Facet>::adapted(m_curve.process(), num_voices, Waveform::DEFAULT_CURVE));

        if (resized || m_tau.has_changed() || m_tau_type.has_changed()) {
            auto tau = m_tau.process().adapted_to(num_voices).firsts_or(FilterSmoo::DEFAULT_TAU);
            auto tau_type = m_tau_type.process().first_or(FilterSmoo::DEFAULT_TAU_TYPE);

            for (std::size_t i = 0; i < num_voices; ++i) {
                m_oscillators[i].set_tau(DomainDuration{tau[i], tau_type});
            }
        }
    }

    void reset() {
        for (auto& oscillator : m_oscillators) {
            oscillator.reset();
        }
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_mode;
    Socket<Facet>& m_waveform;
    Socket<Facet>& m_period;
    Socket<Facet>& m_period_type;
    Socket<Facet>& m_offset;
    Socket<Facet>& m_offset_type;
    Socket<Facet>& m_step_size;
    Socket<Facet>& m_duty;
    Socket<Facet>& m_curve;
    Socket<Facet>& m_tau;
    Socket<Facet>& m_tau_type;
    Socket<Trigger>& m_reset;

    MultiVoiced<Oscillator, double> m_oscillators;

//    LockingQueue<Voices<Facet>> m_previous_values{HISTORY_LENGTH};

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================

template<typename FloatType = double>
struct OscillatorWrapper {
    using Keys = OscillatorNode::OscillatorKeys;

    ParameterHandler ph;

    Sequence<Trigger> trigger{Keys::TRIGGER, ph, Trigger::pulse_on()};

    Variable<Facet, PaMode> mode{Keys::MODE, ph, PhaseAccumulator::DEFAULT_MODE};
    Variable<Facet, Waveform::Mode> waveform{Keys::WAVEFORM, ph, Waveform::DEFAULT_MODE};

    Sequence<Facet, FloatType> period{Keys::PERIOD, ph, Voices<FloatType>::singular(PaParameters::DEFAULT_PERIOD)};
    Variable<Facet, DomainType> period_type{Keys::PERIOD_TYPE, ph, PaParameters::DEFAULT_PERIOD_TYPE};
    Sequence<Facet, FloatType> offset{Keys::OFFSET, ph, Voices<FloatType>::singular(PaParameters::DEFAULT_OFFSET)};
    Variable<Facet, DomainType> offset_type{Keys::OFFSET_TYPE, ph, PaParameters::DEFAULT_OFFSET_TYPE};
    Sequence<Facet, FloatType> step_size{Keys::STEP_SIZE, ph
            , Voices<FloatType>::singular(static_cast<FloatType>(PaParameters::DEFAULT_STEP_SIZE))
    };

    Sequence<Facet, FloatType> duty{Keys::DUTY, ph, Voices<FloatType>::singular(Waveform::DEFAULT_DUTY)};
    Sequence<Facet, FloatType> curve{Keys::CURVE, ph, Voices<FloatType>::singular(Waveform::DEFAULT_CURVE)};

    Sequence<Facet, FloatType> tau{Keys::TAU, ph, Voices<FloatType>::singular(FilterSmoo::DEFAULT_TAU)};
    Variable<Facet, DomainType> tau_type{Keys::TAU_TYPE, ph, FilterSmoo::DEFAULT_TAU_TYPE};

    Sequence<Trigger> reset_trigger{Keys::RESET, ph};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    OscillatorNode oscillator{Keys::CLASS_NAME
                              , ph
                              , &trigger
                              , &mode
                              , &waveform
                              , &period
                              , &period_type
                              , &offset
                              , &offset_type
                              , &step_size
                              , &duty
                              , &curve
                              , &tau
                              , &tau_type
                              , &reset_trigger
                              , &enabled
                              , &num_voices};
};

} // namespace serialist

#endif //SERIALISTLOOPER_OSCILLATOR_H
