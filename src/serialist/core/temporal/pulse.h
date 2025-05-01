
#ifndef SERIALISTLOOPER_PULSE_H
#define SERIALISTLOOPER_PULSE_H

#include <optional>
#include "core/types/trigger.h"
#include "core/types/time_point.h"
#include "core/collections/vec.h"
#include "core/collections/held.h"
#include "core/exceptions.h"

namespace serialist {

struct PulseIdentifier {
    std::size_t id;
    bool triggered = false;

    bool operator==(const PulseIdentifier& other) const { return id == other.id; }

    explicit operator Trigger() const { return Trigger::with_manual_id(Trigger::Type::pulse_off, id); }
};


// ==============================================================================================

class Pulse {
public:
    Pulse(std::size_t id, const DomainTimePoint& trigger_time
          , const std::optional<DomainTimePoint>& pulse_off_time = std::nullopt)
            : m_id(id)
              , m_trigger_time(trigger_time)
              , m_pulse_off_time(pulse_off_time) {
        assert_invariants();
    }

    bool elapsed(const TimePoint& current_time) const {
        return m_pulse_off_time && m_pulse_off_time->elapsed(current_time);
    }

    bool has_pulse_off() const {
        return static_cast<bool>(m_pulse_off_time);
    }

    std::size_t get_id() const { return m_id; }

    /** @throws ParameterError if pulse_off_time is not of the same type as the trigger time */
    void set_pulse_off(const DomainTimePoint& time) {
        if (time.get_type() != m_trigger_time.get_type()) {
            throw ParameterError("pulse_off_time must be of the same type as the trigger time");
        }
        m_pulse_off_time = time;
    }


    void scale_duration(double factor) {
        if (m_pulse_off_time) {
            auto duration = m_trigger_time - *m_pulse_off_time;
            m_pulse_off_time = m_trigger_time + (duration * factor);
        }
    }

    /**
     * @brief sets period of the pulse if period is of the same type as the trigger time
     */
    bool try_set_duration(const DomainDuration& duration) {
        if (m_trigger_time.get_type() == duration.get_type()) {
            m_pulse_off_time = m_trigger_time + duration;
            return true;
        }
        return false;
    }

    DomainTimePoint get_trigger_time() const { return m_trigger_time; }

    const std::optional<DomainTimePoint>& get_pulse_off_time() const { return m_pulse_off_time; }

    DomainType get_type() const {
        return m_trigger_time.get_type();
    }

private:
    void assert_invariants() {
        assert(!m_pulse_off_time || m_pulse_off_time->get_type() == m_trigger_time.get_type());
    }

    std::size_t m_id;
    DomainTimePoint m_trigger_time;
    std::optional<DomainTimePoint> m_pulse_off_time;
};


// ==============================================================================================


class Pulses : public Flushable<Trigger> {
public:

    Trigger new_pulse(const DomainTimePoint& pulse_on_time
                      , const std::optional<DomainTimePoint>& pulse_off_time
                      , std::optional<std::size_t> id = std::nullopt) {
//        if (pulse_on_time.get_type() != m_type) {
//            throw ParameterError("pulse_on_time must be of the same type as the pulses");
//        }
//
//        if (pulse_off_time && pulse_off_time->get_type() != m_type) {
//            throw ParameterError("pulse_off_time must be of the same type as the pulses");
//        }

        auto pulse_id = TriggerIds::new_or(id);

        m_pulses.append(Pulse(pulse_id, pulse_on_time, pulse_off_time));

        return Trigger::with_manual_id(Trigger::Type::pulse_on, pulse_id);
    }


    Voice<Trigger> flush() override {
        return m_pulses.drain()
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Vec<Pulse> drain_elapsed(const TimePoint& time, bool include_endless = false) {
        return m_pulses.filter_drain([time, include_endless](const Pulse& p) {
            return !(p.elapsed(time) || (include_endless && !p.has_pulse_off()));
        });
    }

    Voice<Trigger> drain_elapsed_as_triggers(const TimePoint& time, bool include_endless = false) {
        return drain_elapsed(time, include_endless)
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Pulse> drain_endless() {
        return m_pulses.filter_drain([](const Pulse& p) {
            return !p.has_pulse_off();
        });
    }

    Voice<Pulse> drain_by_id(std::size_t id) {
        return m_pulses.filter_drain([id](const Pulse& p) {
            return p.get_id() != id; // remove all elements for which the id matches
        });
    }

    Voice<Trigger> drain_by_id_as_triggers(std::size_t id) {
        return drain_by_id(id)
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Trigger> drain_endless_as_triggers() {
        return drain_endless()
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Pulse> drain_non_matching(DomainType type) {
        return m_pulses.filter_drain([&type](const Pulse& p) {
            return p.get_trigger_time().get_type() == type; // remove all elements for which the type does not match
        });
    }

    Voice<Trigger> drain_non_matching_as_triggers(DomainType type) {
        return drain_non_matching(type)
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }



    void scale_durations(double factor) {
        for (Pulse& p : m_pulses) {
            p.scale_duration(factor);
        }
    }

    void try_set_durations(const DomainDuration& duration) {
        for (Pulse& p : m_pulses) {
            p.try_set_duration(duration);
        }
    }



    const Pulse* last_of_type(DomainType type) const {
        const Pulse* output = nullptr;
        for (const auto& p: m_pulses) {
            if (p.get_trigger_time().get_type() == type && p.has_pulse_off()) {
                if (!output || output->get_pulse_off_time()->get_value() < p.get_pulse_off_time()->get_value()) {
                    output = &p;
                }
            }
        }
        return output;
    }



//    /**
//     * @brief Sets period for all existing pulses to fixed value
//     */
//    void reschedule(const DomainDuration& new_duration) {
//        for (Pulse& p : m_pulses) {
//            // TODO: This is not a good strategy: what if initial DTP is incompatible with new_duration?
//            p.set_pulse_off(p.get_trigger_time() + new_duration);
//        }
//    }

    Voice<Trigger> reset() {
        return flush();
    }

    bool empty() const {
        return m_pulses.empty();
    }

//    void set_type(DomainType type, const TimePoint& last_transport_time) {
//        if (m_type == type) {
//            return;
//        }
//
//        for (Pulse& p : m_pulses) {
//            p.set_type(type, last_transport_time);
//        }
//
//        m_type = type;
//    }

//    DomainType get_type() const { return m_type; }

    const Vec<Pulse>& vec() const { return m_pulses; }

    Vec<Pulse>& vec_mut() { return m_pulses; }


private:
    Vec<Pulse> m_pulses;
//    DomainType m_type;
};


// ==============================================================================================

/**
 * @brief Tracks held pulses for multiple outlets, i.e. Vec<Voices<Trigger>>
 */
class MultiOutletHeldPulses {
public:
    using OutletHeld = MultiVoiceHeld<PulseIdentifier>;

    explicit MultiOutletHeldPulses(std::size_t num_outlets) : m_held(create_container(num_outlets)) {}


    static void merge_into(Vec<Voices<Trigger>>& output, Vec<Voices<Trigger>>&& flushed) {
        if (flushed.empty()) {
            return;
        }

        assert(output.size() == flushed.size()); // number of outlets should be the same
        auto num_outlets = output.size();

        for (std::size_t outlet_index = 0; outlet_index < num_outlets; ++outlet_index) {
            if (!flushed.empty()) {
                auto& outlet_voices = output[outlet_index];
                auto& flushed_voices = flushed[outlet_index];

                // voices flushed from flush_dangling_triggers should always be bounded by output's size
                assert(flushed_voices.size() <= outlet_voices.size());

                for (std::size_t voice_index = 0; voice_index < flushed_voices.size(); ++voice_index) {
                    insert_unique(outlet_voices[voice_index], std::move(flushed_voices[voice_index]));
                }
            }
        }
    }


    void process(Vec<Voices<Trigger>>& output) {
        assert(output.size() == num_outlets());

        for (std::size_t outlet_index = 0; outlet_index < output.size(); ++outlet_index) {
            process(output[outlet_index], outlet_index);
        }
    }

    void process(Voices<Trigger>& voices, std::size_t outlet_index) {
        assert(outlet_index < num_outlets());

        if (m_held[outlet_index].size() != voices.size()) {
            // Note: only flushes extra voices if size has shrunk
            auto flushed = m_held[outlet_index].resize(voices.size()).as_type<Trigger>();

            for (std::size_t voice_index = 0; voice_index < voices.size(); ++voice_index) {
                process(voices[voice_index], outlet_index, voice_index);
            }

            voices.merge_uneven(flushed, true);

        } else {
            for (std::size_t voice_index = 0; voice_index < voices.size(); ++voice_index) {
                process(voices[voice_index], outlet_index, voice_index);
            }
        }
    }


    Vec<Voices<Trigger>> flush() {
        throw std::runtime_error("not implemented");
    }


    Voices<Trigger> flush_outlet(std::size_t outlet_index
                                 , std::optional<std::size_t> target_voice_count = std::nullopt) {
        assert(outlet_index < num_outlets());

        auto& outlet = m_held[outlet_index];

        if (!target_voice_count || *target_voice_count == outlet.size()) {
            return outlet.flush().as_type<Trigger>();
        }

        auto flushed = Vec<Voice<Trigger>>::allocated(*target_voice_count);

        for (std::size_t voice_index = 0; voice_index < *target_voice_count; ++voice_index) {
            flushed.append(flush_voice(outlet_index, voice_index, true).as_type<Trigger>());
        }

        return Voices<Trigger>{flushed};
    }


    Voice<Trigger> flush_voice(std::size_t outlet_index, std::size_t voice_index, bool allow_out_of_bounds = false) {
        assert(outlet_index < num_outlets());

        auto& outlet = m_held[outlet_index];

        if (voice_index >= outlet.size()) {
            if (allow_out_of_bounds) {
                return {};
            }
            throw ParameterError("Voice index out of bounds");
        }

        return outlet.flush(voice_index).as_type<Trigger>();
    }


    void flag_as_triggered(std::size_t outlet_index) {
        assert(outlet_index < num_outlets());

        auto& outlet = m_held[outlet_index];
        for (std::size_t voice_index = 0; voice_index < outlet.size(); ++voice_index) {
            flag_as_triggered(outlet_index, voice_index);
        }
    }


    void flag_as_triggered(std::size_t outlet_index, std::size_t voice_index, bool allow_out_of_bounds = false) {
        assert(outlet_index < num_outlets());

        // m_held[outlet_index].get_internal_object().get_objects()[voice_index].get_held_mut()
        auto& outlet = m_held[outlet_index];
        if (outlet.size() >= voice_index) {
            if (allow_out_of_bounds) {
                return;
            }
            throw ParameterError("Voice index out of bounds");

        }

        for (auto& p : outlet.get_vec_mut(voice_index)) {
            p.triggered = true;
        }
    }


    std::size_t num_outlets() const { return m_held.size(); }


private:
    static Vec<OutletHeld> create_container(std::size_t num_outlets) {
        return Vec<OutletHeld>::repeated(num_outlets, OutletHeld{1});
    }

    static void insert_unique(Voice<Trigger>& v, const Trigger& trigger, long index = 0) {
        assert(trigger.is_pulse_off());

        if (!Trigger::contains(v, Trigger::Type::pulse_off, trigger.get_id())) {
            v.insert(index, trigger);
        }
    }


    static void insert_unique(Voice<Trigger>& v, Vec<Trigger>&& triggers, long index = 0) {
        triggers.filter_drain([&v](const Trigger& trigger) {
            return !Trigger::contains_pulse_off(v, trigger.get_id());
        });

        v.insert(index, std::move(triggers));
    }


    /**
     * @brief insert a pulse off with the given identifier to voice unless it's already present
     */
    static void insert_unique(Voice<Trigger>& v, const PulseIdentifier& identifier, long index = 0) {
        insert_unique(v, static_cast<Trigger>(identifier), index);
    }


    static void insert_unique(Voice<Trigger>& v, Vec<PulseIdentifier>&& identifiers, long index = 0) {
        insert_unique(v, identifiers.as_type<Trigger>(), index);
    }


    void process(Voice<Trigger>& voice, std::size_t outlet_index, std::size_t voice_index) {
        // note: entire voices object of outlet_index, not just a single voice
        auto& voices_held = m_held[outlet_index];

        assert(voice_index < voices_held.size());

        for (std::size_t i = 0; i < voice.size(); ++i) {
            if (auto& trigger = voice[i]; trigger.is_pulse_on()) {
                voices_held.bind({trigger.get_id()}, voice_index);
            } else if (trigger.is_pulse_off()) {
                voices_held.release({trigger.get_id()}, voice_index);
            }
        }

        // Flush triggered pulses. Note that pulses bound this cycle will never be triggered,
        // so this is safe to call after appending the new pulses
        if (Trigger::contains_any_pulse(voice)) {
            flush_into(voice, outlet_index, voice_index, true);
        }
    }


    void flush_into(Voice<Trigger>& output
                    , std::size_t outlet_index
                    , std::size_t voice_index
                    , bool triggered_only = false
                    , long index = 0) {
        auto flushed = m_held[outlet_index].flush(voice_index, [triggered_only](const PulseIdentifier& p) {
            return !triggered_only || p.triggered;
        });

        insert_unique(output, std::move(flushed), index);
    }


    Vec<OutletHeld> m_held;
};


} // namespace serialist

#endif //SERIALISTLOOPER_PULSE_H
