
#ifndef SERIALISTLOOPER_THREE_STATE_PULSATOR_H
#define SERIALISTLOOPER_THREE_STATE_PULSATOR_H

#include "core/generatives/stereotypes/pulsator_stereotypes.h"
#include "auto_pulsator.h"
#include "triggered_pulsator.h"

class ThruPulsator : public Pulsator {
public:
    Voice<Trigger> handle_external_triggers(double time, const Voice<Trigger>& triggers) override {
        if (!is_running()) {
            return {};
        }

        for (const auto& trigger: triggers) {
            if (trigger.is(Trigger::Type::pulse_on)) {
                m_pulses.append(Pulse<>(trigger.get_id(), time, std::nullopt));

            } else if (trigger.is(Trigger::Type::pulse_off)) {
                m_pulses.remove([trigger](const Pulse<>& pulse) {
                    return pulse.get_id() == trigger.get_id();
                });
            }
        }

        return triggers;
    }


    Vec<Pulse<>> export_pulses() override {
        return m_pulses.drain();
    }


    Voice<Trigger> flush() override {
        return export_pulses().as_type<Trigger>([](const Pulse<>& pulse) {
            return Trigger(Trigger::Type::pulse_off, pulse.get_id());
        });
    }

    void start(double) override { m_running = true; }

    Voice<Trigger> stop() override {
        m_running = false;
        return flush();
    }

    bool is_running() const override {
        return m_running;
    }

    Voice<Trigger> poll(double) override {
        // Irrelevant for ThruPulsator
        return {};
    }

    Voice<Trigger> handle_time_skip(double) override {
        // Since no scheduling occurs, time skips are unproblematic
        return {};
    }

    void import_pulses(const Vec<Pulse<>>& pulses) override {
        // TODO: This situation seems quite problematic, as we cannot guarantee that
        //  associated pulse_offs ever will be received
        m_pulses.extend(pulses);
    }

private:
    Vec<Pulse<>> m_pulses;
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


    Voice<Trigger> poll(double time) override {
        Voice<Trigger> output;
        if (m_new_mode) {
            output.extend(update_mode(*m_new_mode, time));
            m_new_mode = std::nullopt;
        }

        return get_active_pulsator().poll(time);
    }

    Voice<Trigger> flush() override {
        return get_active_pulsator().flush();
    }

    void start(double time) override {
        get_active_pulsator().start(time);
    }

    Voice<Trigger> stop() override {
        return get_active_pulsator().stop();
    }

    bool is_running() const override {
        return get_active_pulsator().is_running();
    }

    Voice<Trigger> handle_external_triggers(double time, const Voice<Trigger>& triggers) override {
        return get_active_pulsator().handle_external_triggers(time, triggers);
    }

    Voice<Trigger> handle_time_skip(double new_time) override {
        return get_active_pulsator().handle_time_skip(new_time);
    }

    Vec<Pulse<>> export_pulses() override {
        return get_active_pulsator().export_pulses();
    }

    void import_pulses(const Vec<Pulse<>>& pulses) override {
        get_active_pulsator().import_pulses(pulses);
    }

    void set_duration(double duration) {
        m_auto_pulsator.set_duration(duration);
        m_triggered_pulsator.set_duration(m_auto_pulsator.get_legato_amount() * m_auto_pulsator.get_duration());

    }

    void set_legato_amount(double legato_amount) {
        m_auto_pulsator.set_legato_amount(legato_amount);
        m_triggered_pulsator.set_duration(m_auto_pulsator.get_legato_amount() * m_auto_pulsator.get_duration());
    }

    void set_sample_and_hold(bool sample_and_hold) {
        m_auto_pulsator.set_sample_and_hold(sample_and_hold);
        // m_triggered_pulsator.set_sample_and_hold(sample_and_hold);
    }

    void set_mode(Mode mode) {
        m_new_mode = mode;
    }

private:
    Voice<Trigger> update_mode(Mode new_mode, double current_time) {
        auto& old_pulsator = get_pulsator(m_mode);
        auto& new_pulsator = get_pulsator(new_mode);

        new_pulsator.import_pulses(old_pulsator.export_pulses());
        auto output = old_pulsator.stop();
        new_pulsator.start(current_time);
        m_mode = new_mode;

        return output;

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


    AutoPulsator m_auto_pulsator;
    TriggeredPulsator m_triggered_pulsator;
    ThruPulsator m_thru_pulsator;

    Mode m_mode = Mode::auto_pulsator;
    std::optional<Mode> m_new_mode = std::nullopt;

};


// ==============================================================================================

class VariableStatePulsatorNode : public PulsatorBase<VariableStatePulsator> {
public:
    static const inline std::string TRIGGER = "trigger";
    static const inline std::string DURATION = "duration";
    static const inline std::string LEGATO = "legato";
    static const inline std::string SAMPLE_AND_HOLD = "sample_and_hold";

    static const inline std::string CLASS_NAME = "VariableStatePulsatorNode";

    VariableStatePulsatorNode(const std::string& id
                              , ParameterHandler& parent
                              , Node<Trigger>* trigger = nullptr
                              , Node<Facet>* duration = nullptr
                              , Node<Facet>* legato_amount = nullptr
                              , Node<Facet>* sample_and_hold = nullptr
                              , Node<Facet>* enabled = nullptr
                              , Node<Facet>* num_voices = nullptr)
            : PulsatorBase<VariableStatePulsator>(id, parent, enabled, num_voices, CLASS_NAME)
              , m_trigger(add_socket(ParameterKeys::TRIGGER, trigger))
              , m_duration(add_socket(DURATION, duration))
              , m_legato_amount(add_socket(LEGATO, legato_amount))
              , m_sample_and_hold(add_socket(SAMPLE_AND_HOLD, sample_and_hold)) {}

    std::size_t get_voice_count() override {
        return voice_count(m_trigger.voice_count()
                           , m_duration.voice_count()
                           , m_legato_amount.voice_count()
                           , m_sample_and_hold.voice_count());
    }

    void update_parameters(std::size_t num_voices, bool size_has_changed) override {
        if (size_has_changed || m_duration.has_changed()) {
            auto duration = m_duration.process().adapted_to(num_voices).firsts_or(1.0);
            get_pulsators_mut().set(&VariableStatePulsator::set_duration, duration.as_type<double>());
        }

        if (size_has_changed || m_legato_amount.has_changed()) {
            auto legato_amount = m_legato_amount.process().adapted_to(num_voices).firsts_or(1.0);
            get_pulsators_mut().set(&VariableStatePulsator::set_legato_amount, legato_amount.as_type<double>());
        }

        if (size_has_changed || m_sample_and_hold.has_changed()) {
            auto sample_and_hold = m_sample_and_hold.process().adapted_to(num_voices).firsts_or(true);
            get_pulsators_mut().set(&VariableStatePulsator::set_sample_and_hold, sample_and_hold.as_type<bool>());
        }
    }

    Voices<Trigger> get_incoming_triggers(const TimePoint&, std::size_t num_voices) override {
        return m_trigger.process(num_voices);
    }

private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_duration;
    Socket<Facet>& m_legato_amount;
    Socket<Facet>& m_sample_and_hold;
};

#endif //SERIALISTLOOPER_THREE_STATE_PULSATOR_H
