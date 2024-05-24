
#ifndef SERIALISTLOOPER_THREE_STATE_PULSATOR_H
#define SERIALISTLOOPER_THREE_STATE_PULSATOR_H

#include "core/generatives/stereotypes/pulsator_stereotypes.h"
#include "auto_pulsator.h"
#include "triggered_pulsator.h"
#include "sequence.h"
#include "variable.h"
#include "core/algo/temporal/time_point.h"

class ThruPulsator : public Pulsator {
public:
    static const DomainType TYPE = DomainType::ticks;

    void start(const TimePoint&, const std::optional<DomainTimePoint>&) override { m_running = true; }

    Voice<Trigger> stop() override {
        m_running = false;
        return flush();
    }

    bool is_running() const override {
        return m_running;
    }

    Voice<Trigger> handle_external_triggers(const TimePoint& time, const Voice<Trigger>& triggers) override {
        if (!is_running()) {
            return {};
        }

        for (const auto& trigger: triggers) {
            if (trigger.is(Trigger::Type::pulse_on)) {
                m_pulses.append(Pulse(trigger.get_id(), as_dtp(time)));

            } else if (trigger.is(Trigger::Type::pulse_off)) {
                m_pulses.remove([trigger](const Pulse& pulse) {
                    return pulse.get_id() == trigger.get_id();
                });
            }
        }

        return triggers;
    }

    Voice<Trigger> poll(const TimePoint&) override {
        // Irrelevant for ThruPulsator
        return {};
    }


    Voice<Trigger> handle_time_skip(const TimePoint&) override {
        // Since no scheduling occurs, time skips are unproblematic
        return {};
    }

    Vec<Pulse> export_pulses() override {
        return m_pulses.drain();
    }


    void import_pulses(const Vec<Pulse>& pulses) override {
        // TODO: This situation seems quite problematic, as we cannot guarantee that
        //  associated pulse_offs ever will be received
        m_pulses.extend(pulses);
    }

    std::optional<DomainTimePoint> next_scheduled_pulse_on() override {
        return std::nullopt;
    }

    Voice<Trigger> flush() override {
        return export_pulses().as_type<Trigger>([](const Pulse& pulse) {
            return Trigger::pulse_off(pulse.get_id());
        });
    }


private:
    static DomainTimePoint as_dtp(const TimePoint& time) {
        return DomainTimePoint::from_time_point(time, DomainType::ticks);
    }

    Vec<Pulse> m_pulses;
    bool m_running = false;

};


// ==============================================================================================

class VariableStatePulsator : public Pulsator {
public:
    enum class Mode {
        auto_pulsator
        , triggered_pulsator
        , thru_pulsator
    };

    explicit VariableStatePulsator(const DomainDuration& duration = DomainDuration(1.0, DomainType::ticks)
                                   , const DomainDuration& offset = DomainDuration(1.0, DomainType::ticks)
                                   , double legato = 1.0
                                   , bool offset_enabled = false)
            : m_duration(duration)
              , m_offset(offset)
              , m_offset_enabled(offset_enabled)
              , m_legato(legato)
              , m_auto_pulsator(temporal::ts_from_duration_offset(duration, offset, offset_enabled), legato)
              , m_triggered_pulsator(duration * legato) {
    }

    void start(const TimePoint& time, const std::optional<DomainTimePoint>& first_pulse_time) override {
        if (m_new_mode) {
            update_mode(*m_new_mode, time);
            m_new_mode = std::nullopt;
        }

        get_active_pulsator().start(time, first_pulse_time);
    }


    Voice<Trigger> stop() override {
        return get_active_pulsator().stop();
    }

    bool is_running() const override {
        return get_active_pulsator().is_running();
    }

    Voice<Trigger> handle_external_triggers(const TimePoint& time, const Voice<Trigger>& triggers) override {
        if (m_new_mode) {
            update_mode(*m_new_mode, time);
            m_new_mode = std::nullopt;
        }

        return get_active_pulsator().handle_external_triggers(time, triggers);
    }

    Voice<Trigger> poll(const TimePoint& time) override {
        if (m_new_mode) {
            update_mode(*m_new_mode, time);
            m_new_mode = std::nullopt;
        }

        return get_active_pulsator().poll(time);
    }


    Voice<Trigger> handle_time_skip(const TimePoint& new_time) override {
        return get_active_pulsator().handle_time_skip(new_time);
    }

    Vec<Pulse> export_pulses() override {
        return get_active_pulsator().export_pulses();
    }

    void import_pulses(const Vec<Pulse>& pulses) override {
        get_active_pulsator().import_pulses(pulses);
    }

    std::optional<DomainTimePoint> next_scheduled_pulse_on() override {
        return get_active_pulsator().next_scheduled_pulse_on();
    }

    Voice<Trigger> flush() override {
        return get_active_pulsator().flush();
    }


    void set_duration(const DomainDuration& duration) {
        m_duration = duration;
        m_auto_pulsator.set_ts(temporal::ts_from_duration_offset(m_duration, m_offset, m_offset_enabled));
        m_triggered_pulsator.set_duration(FreePeriodicTimePoint(m_duration * m_legato));
    }

    void set_offset(const DomainDuration& offset) {
        m_offset = offset;
        m_auto_pulsator.set_ts(temporal::ts_from_duration_offset(m_duration, m_offset, m_offset_enabled));
    }

    void set_offset_enabled(bool enabled) {
        m_offset_enabled = enabled;
        m_auto_pulsator.set_ts(temporal::ts_from_duration_offset(m_duration, m_offset, m_offset_enabled));
    }



    void set_legato_amount(double legato_amount) {
        m_legato = legato_amount;
        m_auto_pulsator.set_legato_amount(legato_amount);
        m_triggered_pulsator.set_duration(FreePeriodicTimePoint(m_duration * m_legato));
    }

    void set_sample_and_hold(bool sample_and_hold) {
        m_auto_pulsator.set_sample_and_hold(sample_and_hold);
        // m_triggered_pulsator.set_sample_and_hold(sample_and_hold);
    }

    void set_mode(Mode mode) {
        if (m_mode == mode) {
            return;
        }

        m_new_mode = mode;
    }


private:
    void update_mode(Mode new_mode, const TimePoint& current_time) {
        auto& old_pulsator = get_pulsator(m_mode);
        auto& new_pulsator = get_pulsator(new_mode);

        bool is_running = old_pulsator.is_running();

        new_pulsator.import_pulses(old_pulsator.export_pulses());
        auto flushed = old_pulsator.stop();

        assert(flushed.empty());

        if (is_running) {
            new_pulsator.start(current_time, old_pulsator.next_scheduled_pulse_on());
        }

        m_mode = new_mode;
    }

    Pulsator& get_pulsator(Mode mode) {
        switch (mode) {
            case Mode::auto_pulsator:
                return m_auto_pulsator;
            case Mode::triggered_pulsator:
                return m_triggered_pulsator;
            case Mode::thru_pulsator:
                return m_thru_pulsator;
        }
    }

    const Pulsator& get_pulsator(Mode mode) const {
        switch (mode) {
            case Mode::auto_pulsator:
                return m_auto_pulsator;
            case Mode::triggered_pulsator:
                return m_triggered_pulsator;
            case Mode::thru_pulsator:
                return m_thru_pulsator;
        }
    }

    const Pulsator& get_active_pulsator() const {
        return get_pulsator(m_mode);
    }

    Pulsator& get_active_pulsator() {
        return get_pulsator(m_mode);
    }

    DomainDuration m_duration;
    DomainDuration m_offset;
    bool m_offset_enabled;
    double m_legato;

    AutoPulsator m_auto_pulsator;
    TriggeredPulsator m_triggered_pulsator;
    ThruPulsator m_thru_pulsator;

    Mode m_mode = Mode::auto_pulsator;
    std::optional<Mode> m_new_mode = std::nullopt;

};


// ==============================================================================================

class VariableStatePulsatorNode : public PulsatorNodeBase<VariableStatePulsator> {
public:
    struct Keys {
        static const inline std::string TRIGGER = "trigger";
        static const inline std::string DURATION = "duration";
        static const inline std::string DURATION_TYPE = "duration_type";
        static const inline std::string OFFSET = "offset_type";
        static const inline std::string OFFSET_TYPE = "offset_type";
        static const inline std::string OFFSET_ENABLED = "offset_type";
        static const inline std::string LEGATO = "legato";
        static const inline std::string SAMPLE_AND_HOLD = "sample_and_hold";

        static const inline std::string CLASS_NAME = "variable_state_pulsator";

    };

    VariableStatePulsatorNode(const std::string& id
                              , ParameterHandler& parent
                              , Node<Trigger>* trigger = nullptr
                              , Node<Facet>* duration = nullptr
                              , Node<Facet>* duration_type = nullptr
                              , Node<Facet>* offset = nullptr
                              , Node<Facet>* offset_type = nullptr
                              , Node<Facet>* offset_enabled = nullptr
                              , Node<Facet>* legato_amount = nullptr
                              , Node<Facet>* sample_and_hold = nullptr
                              , Node<Facet>* enabled = nullptr
                              , Node<Facet>* num_voices = nullptr)
            : PulsatorNodeBase<VariableStatePulsator>(id, parent, enabled, num_voices, Keys::CLASS_NAME)
              , m_trigger(add_socket(ParameterKeys::TRIGGER, trigger))
              , m_duration(add_socket(Keys::DURATION, duration))
              , m_duration_type(add_socket(Keys::DURATION_TYPE, duration_type))
              , m_offset(add_socket(Keys::OFFSET, offset))
              , m_offset_type(add_socket(Keys::OFFSET_TYPE, offset_type))
              , m_offset_enabled(add_socket(Keys::OFFSET_ENABLED, offset_enabled))
              , m_legato_amount(add_socket(Keys::LEGATO, legato_amount))
              , m_sample_and_hold(add_socket(Keys::SAMPLE_AND_HOLD, sample_and_hold)) {
        set_mode(get_mode());
    }


    VariableStatePulsator::Mode get_mode() const {
        if (!m_trigger.is_connected()) {
            return VariableStatePulsator::Mode::auto_pulsator;
        } else if (!m_duration.is_connected()) {
            return VariableStatePulsator::Mode::thru_pulsator;
        } else {
            return VariableStatePulsator::Mode::triggered_pulsator;
        }
    }

    std::size_t get_voice_count() override {
        return voice_count(m_trigger.voice_count()
                           , m_duration.voice_count()
                           , m_legato_amount.voice_count()
                           , m_sample_and_hold.voice_count());
    }

    void update_parameters(std::size_t num_voices, bool size_has_changed) override {
        auto mode = get_mode();
        if (size_has_changed || mode != m_previous_mode) {
            set_mode(mode);
        }

        if (size_has_changed || m_duration.has_changed() || m_duration_type.has_changed()) {
            auto duration = m_duration.process().adapted_to(num_voices).firsts_or(1.0);
            auto duration_type = m_duration_type.process().first_or(DomainType::ticks);
            pulsators().set(&VariableStatePulsator::set_duration
                            , duration.as_type<DomainDuration>([&duration_type](const double dur) {
                        return DomainDuration(dur, duration_type);
                    }));
        }

        if (size_has_changed || m_offset.has_changed() || m_offset_type.has_changed() || m_offset_enabled.has_changed()) {
            auto offset = m_offset.process().adapted_to(num_voices).firsts_or(0.0);
            auto offset_type = m_offset_type.process().first_or(DomainType::ticks);
            auto offset_enable = m_offset_enabled.process().first_or(false);

            pulsators().set(&VariableStatePulsator::set_offset
                            , offset.as_type<DomainDuration>([&offset_type](const double ot) {
                                return DomainDuration(ot, offset_type);
                            }));

            pulsators().set(&VariableStatePulsator::set_offset_enabled, Vec<bool>::repeated(num_voices, offset_enable));
        }

        if (size_has_changed || m_legato_amount.has_changed()) {
            auto legato_amount = m_legato_amount.process().adapted_to(num_voices).firsts_or(1.0);
            pulsators().set(&VariableStatePulsator::set_legato_amount, legato_amount.as_type<double>());
        }

        if (size_has_changed || m_sample_and_hold.has_changed()) {
            auto sample_and_hold = m_sample_and_hold.process().adapted_to(num_voices).firsts_or(true);
            pulsators().set(&VariableStatePulsator::set_sample_and_hold, sample_and_hold.as_type<bool>());
        }
    }

    Voices<Trigger> get_incoming_triggers(const TimePoint&, std::size_t num_voices) override {
        return m_trigger.process(num_voices);
    }

    void set_trigger(Node<Trigger>* trigger) { m_trigger = trigger; }

    void set_duration(Node<Facet>* duration) { m_duration = duration; }

    void set_duration_type(Node<Facet>* duration_type) { m_duration_type = duration_type; }

    void set_offset(Node<Facet>* offset) { m_offset = offset; }

    void set_offset_type(Node<Facet>* offset_type) { m_offset_type = offset_type; }

    void set_legato_amount(Node<Facet>* legato_amount) { m_legato_amount = legato_amount; }

    void set_sample_and_hold(Node<Facet>* sample_and_hold) { m_sample_and_hold = sample_and_hold; }

    Socket<Trigger>& get_trigger() { return m_trigger; }

    Socket<Facet>& get_duration() { return m_duration; }

    Socket<Facet>& get_duration_type() { return m_duration_type; }

    Socket<Facet>& get_offset() { return m_offset; }

    Socket<Facet>& get_offset_type() { return m_offset_type; }

    Socket<Facet>& get_legato_amount() { return m_legato_amount; }

    Socket<Facet>& get_sample_and_hold() { return m_sample_and_hold; }

private:
    void set_mode(VariableStatePulsator::Mode mode) {
        pulsators().set(&VariableStatePulsator::set_mode
                        , Vec<VariableStatePulsator::Mode>::repeated(pulsators().size(), mode));
        m_previous_mode = mode;
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_duration;
    Socket<Facet>& m_duration_type;
    Socket<Facet>& m_offset;
    Socket<Facet>& m_offset_type;
    Socket<Facet>& m_offset_enabled;
    Socket<Facet>& m_legato_amount;
    Socket<Facet>& m_sample_and_hold;

    VariableStatePulsator::Mode m_previous_mode = VariableStatePulsator::Mode::triggered_pulsator;
};


// ==============================================================================================

template<typename FloatType = float>
struct VariableStatePulsatorWrapper {
    using Keys = VariableStatePulsatorNode::Keys;

    ParameterHandler parameter_handler;

    Sequence<Trigger> trigger{ParameterKeys::TRIGGER, parameter_handler, Voices<Trigger>::empty_like()};
    Sequence<Facet, FloatType> duration{Keys::DURATION, parameter_handler, Voices<FloatType>::singular(1.0)};
    Variable<Facet, DomainType> duration_type{Keys::DURATION_TYPE, parameter_handler, DomainType::ticks};
    Sequence<Facet, FloatType> offset{Keys::OFFSET, parameter_handler, Voices<FloatType>::singular(0.0)};
    Variable<Facet, DomainType> offset_type{Keys::OFFSET_TYPE, parameter_handler, DomainType::ticks};
    Variable<Facet, bool> offset_enabled{Keys::OFFSET_ENABLED, parameter_handler, false};
    Sequence<Facet, FloatType> legato_amount{Keys::LEGATO, parameter_handler, Voices<FloatType>::singular(1.0)};
    Sequence<Facet, bool> sample_and_hold{Keys::SAMPLE_AND_HOLD, parameter_handler, Voices<bool>::singular(true)};
    Sequence<Facet, bool> enabled{ParameterKeys::ENABLED, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{ParameterKeys::NUM_VOICES, parameter_handler, 0};

    VariableStatePulsatorNode pulsator_node{Keys::CLASS_NAME
                                            , parameter_handler
                                            , &trigger
                                            , &duration
                                            , &duration_type
                                            , &offset
                                            , &offset_type
                                            , &offset_enabled
                                            , &legato_amount
                                            , &sample_and_hold
                                            , &enabled
                                            , &num_voices};

};

#endif //SERIALISTLOOPER_THREE_STATE_PULSATOR_H
