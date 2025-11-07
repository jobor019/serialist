
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

/**
 * This class is intended for any Generative that uses pulse_offs, where the pulse_offs may be broadcast due
 * to changes in other parameters. The idea is that rather than calling `triggers.adapted_to(num_voices)` directly,
 * we utilize this class to indicate which voices whose indices may have changed due to broadcasting-related aspects.
 *
 * For example, suppose that we have a MakeNoteNode which receives the following:
 * - t=0: triggers={ ON(a), ON(b)         },  note_numbers={60, 62, 64}
 * - t=1: triggers={  -   ,  -    , ON(c) },  note_numbers={60, 62, 64}
 *
 * At t=0, we'll get three outgoing voices:          { ON(a, 60), ON(b, 62), ON(a, 64) }.
 * At t=1, if we don't utilize this class, we'll get { -        , -        , ON(c, 64) }, which means that the third
 *         voice will have a lingering pulse associated with (a), which likely won't ever be released.
 *
 * Utilizing this class, we will get a map indicating that the voice at index 2 has changed due to broadcasting,
 * which allows us to flush the voice accordingly before generating the new value associated with ON(c).
 *
 *
 * Similarly, we may have cases where multiple triggers have their indices re-associated due to change in trigger count,
 * For example, consider the following:
 * - t=0: triggers={ ON(a), ON(b)         },  note_numbers={60, 61, 62, 63, 64, 65, 66}
 * - t=1: triggers={  -   ,  -    , ON(c) },  note_numbers={60, 61, 62, 63, 64, 65, 66}
 *
 * At t=0, this would give us the following associations:           { a, b, a, b, a, b, a }
 * At t=1, we'd have the following associations                     { a, b, c, a, b, c, a }
 * which means that we'd need to flush the following re-associated: { -, -, X, X, X, X, - }
 *
 *
 * Note that this is only relevant for Generatives that use pulse_offs. Most Generatives only care about pulse_on,
 * in which case this class is completely redundant and `triggers.adapted_to(num_voices)` is sufficient
 */
class PulseBroadcastHandler {
public:
    /**
     * @return boolean mask for the indices that need to be flushed due to changes in broadcasting of size `num_voices`
     *         Note that this will not return size-related changes, only broadcast-related changes, e.g.
     *
     *         - num_voices: 5 => 3, triggers.size() = 8 returns {}. voices 3 and 4 will need to be flushed,
     *           but not due to broadcasting, so this is likely handled elsewhere
     *
     *         - num_voices: 5 => 3, triggers.size(): 5 => 3 returns {}. same as above
     *
     *         - num_voices: 3 => 5, triggers.size() = 3 returns {}. Relevant changes in broadcasting apply, but they
     *           don't require flushing as we're only adding voices that previously didn't exist
     *
     *         - num_voices: 3, triggers.size(): 2 = > 3 returns {0, 0, 1}.
     *
     */
    Vec<bool> broadcast(Voices<Trigger>& triggers, std::size_t num_voices) {
        assert(num_voices > 0);

        if (is_first_value()) {
            update_state(triggers, num_voices);
            triggers.adapted_to(num_voices);
            return empty_vector(num_voices);
        }

        assert(m_previous_trigger_size.has_value()
            && m_previous_num_voices.has_value()
            && !m_previous_broadcasted_indices.empty());

        if (*m_previous_trigger_size == triggers.size() && *m_previous_num_voices == num_voices) {
            triggers.adapted_to(num_voices);
            return empty_vector(num_voices); // no changes, no need to update state
        }

        // TODO: This part is redundant but was originally written for clarification
        // if (*m_previous_trigger_size == triggers.size() && *m_previous_num_voices != num_voices) {
        //     // Two cases:
        //     // - num_voices increased: we're adding new broadcasted voices but these were previously empty so no flush needed
        //     // - num_voices decreased: we're removing voices. Flushing required but handled elsewhere
        //     update_state(triggers, num_voices);
        //     return {};
        // }
        //
        // if (*m_previous_trigger_size == *m_previous_num_voices && triggers.size() == num_voices) {
        //     // both voices have equivalent counts both before and after, meaning no broadcasting applies
        //     update_state(triggers, num_voices);
        //     return {};
        // }

        auto current_broadcast_indices = get_broadcast_indices(triggers.size(), num_voices);
        auto diff = get_broadcast_diff(m_previous_broadcasted_indices, current_broadcast_indices);
        update_state(triggers, num_voices, std::move(current_broadcast_indices));

        triggers.adapted_to(num_voices);

        return diff;
    }

    void clear() {
        m_previous_trigger_size = std::nullopt;
        m_previous_num_voices = std::nullopt;
        m_previous_broadcasted_indices.clear();
    }


private:
    bool is_first_value() const {
        return !static_cast<bool>(m_previous_trigger_size);
    }

    static Vec<bool> empty_vector(std::size_t num_voices) {
        return Vec<bool>::zeros(num_voices);
    }

    void update_state(const Voices<Trigger>& triggers, std::size_t num_voices, Vec<std::size_t>&& index_map) {
        m_previous_trigger_size = triggers.size();
        m_previous_num_voices = num_voices;
        m_previous_broadcasted_indices = std::move(index_map);
    }


    void update_state(const Voices<Trigger>& triggers, std::size_t num_voices) {
        update_state(triggers, num_voices, get_broadcast_indices(triggers.size(), num_voices));
    }

    static Vec<std::size_t> get_broadcast_indices(std::size_t trigger_size, std::size_t num_voices) {
        assert(trigger_size > 0);

        auto v = Vec<std::size_t>::allocated(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            v.append(i % trigger_size);
        }
        return v;
    }

    /** @brief Returns boolean mask for the indices that need to be flushed due to changes in broadcasting */
    static Vec<bool> get_broadcast_diff(const Vec<std::size_t>& previous, const Vec<std::size_t>& current) {
        // Note: this function only returns changes significant for broadcasting, not size changes
        auto size = std::min(previous.size(), current.size());

        Vec<bool> diff = Vec<bool>::zeros(current.size()); // still want boolean mask to have same size as current
        for (std::size_t i = 0; i < size; ++i) {
            diff[i] = previous[i] != current[i];
        }

        return diff;

    }

    std::optional<std::size_t> m_previous_trigger_size;
    std::optional<std::size_t> m_previous_num_voices;
    Vec<std::size_t> m_previous_broadcasted_indices;
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

        assert(output.size() >= flushed.size());
        auto num_mergeable = std::min(output.size(), flushed.size());

        for (std::size_t outlet_index = 0; outlet_index < num_mergeable; ++outlet_index) {
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
        assert(output.size() <= num_outlets());

        for (std::size_t outlet_index = 0; outlet_index < output.size(); ++outlet_index) {
            process(output[outlet_index], outlet_index);
        }

        for (std::size_t outlet_index = output.size(); outlet_index < num_outlets(); ++outlet_index) {
            output.append(m_held[outlet_index].flush().as_type<Trigger>());
        }

        assert(output.size() == num_outlets());
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
        auto flushed = Vec<Voices<Trigger>>::allocated(num_outlets());

        for (std::size_t outlet_index = 0; outlet_index < num_outlets(); ++outlet_index) {
            flushed.append(flush_outlet(outlet_index));
        }

        return flushed;
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

        auto& outlet = m_held[outlet_index];
        if (voice_index >= outlet.size()) {
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
            bool should_flush = !triggered_only || p.triggered;
            return !should_flush; // once again: confusingly flushes the elements for which the condition returns false
        });

        insert_unique(output, std::move(flushed), index);
    }


    Vec<OutletHeld> m_held;
};



// TODO: Remove: legacy helper classes associated with classes that are no longer part of the library

// ==============================================================================================

// class Pulse {
// public:
//     Pulse(std::size_t id, const DomainTimePoint& trigger_time
//           , const std::optional<DomainTimePoint>& pulse_off_time = std::nullopt)
//             : m_id(id)
//               , m_trigger_time(trigger_time)
//               , m_pulse_off_time(pulse_off_time) {
//         assert_invariants();
//     }
//
//     bool elapsed(const TimePoint& current_time) const {
//         return m_pulse_off_time && m_pulse_off_time->elapsed(current_time);
//     }
//
//     bool has_pulse_off() const {
//         return static_cast<bool>(m_pulse_off_time);
//     }
//
//     std::size_t get_id() const { return m_id; }
//
//     /** @throws ParameterError if pulse_off_time is not of the same type as the trigger time */
//     void set_pulse_off(const DomainTimePoint& time) {
//         if (time.get_type() != m_trigger_time.get_type()) {
//             throw ParameterError("pulse_off_time must be of the same type as the trigger time");
//         }
//         m_pulse_off_time = time;
//     }
//
//
//     void scale_duration(double factor) {
//         if (m_pulse_off_time) {
//             auto duration = m_trigger_time - *m_pulse_off_time;
//             m_pulse_off_time = m_trigger_time + (duration * factor);
//         }
//     }
//
//     /**
//      * @brief sets period of the pulse if period is of the same type as the trigger time
//      */
//     bool try_set_duration(const DomainDuration& duration) {
//         if (m_trigger_time.get_type() == duration.get_type()) {
//             m_pulse_off_time = m_trigger_time + duration;
//             return true;
//         }
//         return false;
//     }
//
//     DomainTimePoint get_trigger_time() const { return m_trigger_time; }
//
//     const std::optional<DomainTimePoint>& get_pulse_off_time() const { return m_pulse_off_time; }
//
//     DomainType get_type() const {
//         return m_trigger_time.get_type();
//     }
//
// private:
//     void assert_invariants() {
//         assert(!m_pulse_off_time || m_pulse_off_time->get_type() == m_trigger_time.get_type());
//     }
//
//     std::size_t m_id;
//     DomainTimePoint m_trigger_time;
//     std::optional<DomainTimePoint> m_pulse_off_time;
// };
//
//
// // ==============================================================================================
//
//
// class Pulses : public Flushable<Trigger> {
// public:
//
//     Trigger new_pulse(const DomainTimePoint& pulse_on_time
//                       , const std::optional<DomainTimePoint>& pulse_off_time
//                       , std::optional<std::size_t> id = std::nullopt) {
// //        if (pulse_on_time.get_type() != m_type) {
// //            throw ParameterError("pulse_on_time must be of the same type as the pulses");
// //        }
// //
// //        if (pulse_off_time && pulse_off_time->get_type() != m_type) {
// //            throw ParameterError("pulse_off_time must be of the same type as the pulses");
// //        }
//
//         auto pulse_id = TriggerIds::new_or(id);
//
//         m_pulses.append(Pulse(pulse_id, pulse_on_time, pulse_off_time));
//
//         return Trigger::with_manual_id(Trigger::Type::pulse_on, pulse_id);
//     }
//
//
//     Voice<Trigger> flush() override {
//         return m_pulses.drain()
//                 .template as_type<Trigger>([](const Pulse& p) {
//                     return Trigger::pulse_off(p.get_id());
//                 });
//     }
//
//     Vec<Pulse> drain_elapsed(const TimePoint& time, bool include_endless = false) {
//         return m_pulses.filter_drain([time, include_endless](const Pulse& p) {
//             return !(p.elapsed(time) || (include_endless && !p.has_pulse_off()));
//         });
//     }
//
//     Voice<Trigger> drain_elapsed_as_triggers(const TimePoint& time, bool include_endless = false) {
//         return drain_elapsed(time, include_endless)
//                 .template as_type<Trigger>([](const Pulse& p) {
//                     return Trigger::pulse_off(p.get_id());
//                 });
//     }
//
//     Voice<Pulse> drain_endless() {
//         return m_pulses.filter_drain([](const Pulse& p) {
//             return !p.has_pulse_off();
//         });
//     }
//
//     Voice<Pulse> drain_by_id(std::size_t id) {
//         return m_pulses.filter_drain([id](const Pulse& p) {
//             return p.get_id() != id; // remove all elements for which the id matches
//         });
//     }
//
//     Voice<Trigger> drain_by_id_as_triggers(std::size_t id) {
//         return drain_by_id(id)
//                 .template as_type<Trigger>([](const Pulse& p) {
//                     return Trigger::pulse_off(p.get_id());
//                 });
//     }
//
//     Voice<Trigger> drain_endless_as_triggers() {
//         return drain_endless()
//                 .template as_type<Trigger>([](const Pulse& p) {
//                     return Trigger::pulse_off(p.get_id());
//                 });
//     }
//
//     Voice<Pulse> drain_non_matching(DomainType type) {
//         return m_pulses.filter_drain([&type](const Pulse& p) {
//             return p.get_trigger_time().get_type() == type; // remove all elements for which the type does not match
//         });
//     }
//
//     Voice<Trigger> drain_non_matching_as_triggers(DomainType type) {
//         return drain_non_matching(type)
//                 .template as_type<Trigger>([](const Pulse& p) {
//                     return Trigger::pulse_off(p.get_id());
//                 });
//     }
//
//
//
//     void scale_durations(double factor) {
//         for (Pulse& p : m_pulses) {
//             p.scale_duration(factor);
//         }
//     }
//
//     void try_set_durations(const DomainDuration& duration) {
//         for (Pulse& p : m_pulses) {
//             p.try_set_duration(duration);
//         }
//     }
//
//
//
//     const Pulse* last_of_type(DomainType type) const {
//         const Pulse* output = nullptr;
//         for (const auto& p: m_pulses) {
//             if (p.get_trigger_time().get_type() == type && p.has_pulse_off()) {
//                 if (!output || output->get_pulse_off_time()->get_value() < p.get_pulse_off_time()->get_value()) {
//                     output = &p;
//                 }
//             }
//         }
//         return output;
//     }
//
//
//
// //    /**
// //     * @brief Sets period for all existing pulses to fixed value
// //     */
// //    void reschedule(const DomainDuration& new_duration) {
// //        for (Pulse& p : m_pulses) {
// //            // TODO: This is not a good strategy: what if initial DTP is incompatible with new_duration?
// //            p.set_pulse_off(p.get_trigger_time() + new_duration);
// //        }
// //    }
//
//     Voice<Trigger> reset() {
//         return flush();
//     }
//
//     bool empty() const {
//         return m_pulses.empty();
//     }
//
// //    void set_type(DomainType type, const TimePoint& last_transport_time) {
// //        if (m_type == type) {
// //            return;
// //        }
// //
// //        for (Pulse& p : m_pulses) {
// //            p.set_type(type, last_transport_time);
// //        }
// //
// //        m_type = type;
// //    }
//
// //    DomainType get_type() const { return m_type; }
//
//     const Vec<Pulse>& vec() const { return m_pulses; }
//
//     Vec<Pulse>& vec_mut() { return m_pulses; }
//
//
// private:
//     Vec<Pulse> m_pulses;
// //    DomainType m_type;
// };


} // namespace serialist

#endif //SERIALISTLOOPER_PULSE_H
