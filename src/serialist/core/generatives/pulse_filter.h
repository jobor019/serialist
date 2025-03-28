#ifndef SERIALIST_PULSE_FILTER_H
#define SERIALIST_PULSE_FILTER_H

#include "core/event.h"
#include "core/generative.h"
#include "core/temporal/trigger.h"
#include "core/algo/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "sequence.h"
#include "variable.h"


namespace serialist {
// ==============================================================================================


struct PulseIdentifier {
    std::size_t id;
    bool triggered = false;
};


class PulseFilter : public Flushable<Trigger> {
public:
    enum class Mode { sustain, pause };


    Voice<Trigger> process(Voice<Trigger>&& triggers, bool is_closed) {
        if (!m_closed && !is_closed) {
            return process_open(std::move(triggers));
        }

        // Closed in this cycle
        if (!m_closed && is_closed) {
            m_closed = is_closed;
            return close(std::move(triggers));
        }

        // Opened in this cycle
        if (m_closed && !is_closed) {
            m_closed = is_closed;
            return open(std::move(triggers));
        }

        // Otherwise: already closed
        return process_closed(std::move(triggers));
    }


    Voice<Trigger> flush() override {
        return ids_to_pulse_offs(m_pulses.flush());
    }


    Voice<Trigger> set_mode(Mode mode) {
        // For now, we don't need to handle this case gracefully during runtime
        if (mode != m_mode) {
            m_mode = mode;
            return flush();
        }

        return {};
    }


    Voice<Trigger> set_immediate(bool immediate) {
        // For now, we don't need to handle this case gracefully during runtime
        if (immediate != m_immediate) {
            m_immediate = immediate;
            return flush();
        }

        return {};
    }

private:
    Voice<Trigger> open(Voice<Trigger>&& triggers) {
        Voice<Trigger> flushed;
        if (m_mode == Mode::sustain && m_immediate) {
            flushed = flush_triggered();
        }

        return merge_without_duplicates(std::move(flushed), process_open(std::move(triggers)));
    }

    Voice<Trigger> close(Voice<Trigger>&& triggers) {
        if (m_mode == Mode::sustain) {
            register_pulse_ons(triggers);

            // Flag any pulse_offs without releasing them
            handle_pulse_offs(triggers, false);

            // Remove all pulse offs from output
            triggers.filter_drain([](const Trigger& t) { return !t.is_pulse_off(); });
            return triggers;
        }

        // mode == pause
        if (m_immediate) {
            return flush();
        } else {
            return process_closed(std::move(triggers));
        }
    }

    Voice<Trigger> process_closed(Voice<Trigger>&& triggers) {
        // Pulse on (all modes): ignore
        //
        // Pulse offs:
        //   sustain (immediate & !immediate): flag matching held pulses as triggered, without releasing them
        //   pause (immediate):                same as above
        //   pause (!immediate):               release any matched held pulses
        bool release_matched = m_mode == Mode::pause && !m_immediate;
        return ids_to_pulse_offs(handle_pulse_offs(triggers, release_matched));
    }

    Voice<Trigger> process_open(Voice<Trigger>&& triggers) {
        // Pulse ons (all modes): process as usual
        //
        // Pulse offs:
        //   sustain (!immediate): we may have lingering pulses from a previous closed state, which should all
        //                         be released on the next pulse_off (independently of matching id)
        //   sustain (immediate):  passthrough
        //   pause (both):         passthrough

        Voice<Trigger> flushed;
        if (m_mode == Mode::sustain && !m_immediate && Trigger::contains_pulse_off(triggers)) {
            flushed = flush_triggered();
        }

        register_pulse_ons(triggers);
        handle_pulse_offs(triggers, false);

        return merge_without_duplicates(std::move(flushed), std::move(triggers));
    }

    void register_pulse_ons(const Voice<Trigger>& triggers) {
        for (const auto& trigger : triggers) {
            if (trigger.is_pulse_on()) {
                register_pulse_on(trigger.get_id());
            }
        }
    }

    Voice<PulseIdentifier> handle_pulse_offs(const Voice<Trigger>& triggers, bool release_matched) {
        Voice<PulseIdentifier> output;
        for (const auto& trigger : triggers) {
            if (trigger.is_pulse_off()) {
                if (auto released = handle_pulse_off(trigger.get_id(), release_matched)) {
                    output.append(*released);
                }
            }
        }
        return output;
    }


    void register_pulse_on(std::size_t id) {
        m_pulses.bind(PulseIdentifier{id});
    }


    std::optional<PulseIdentifier> handle_pulse_off(std::size_t id, bool release) {
        if (auto p = m_pulses.find([id](const PulseIdentifier& p) { return p.id == id; })) {
            if (release) {
                m_pulses.release(*p);
                return *p;
            }
            // otherwise: just flag it as releases
            p->triggered = true;
        }
        return std::nullopt;
    }


    Voice<Trigger> flush_triggered() {
        return ids_to_pulse_offs(m_pulses.flush([](const PulseIdentifier& p) { return p.triggered; }));
    }


    static Voice<Trigger> merge_without_duplicates(Voice<Trigger>&& flushed, Voice<Trigger>&& triggers) {
        if (flushed.empty()) return std::move(triggers);
        if (triggers.empty()) return std::move(flushed);

        for (auto flushed_trigger : flushed) {
            if (!triggers.contains(flushed_trigger)) {
                triggers.insert_sorted(flushed_trigger);
            }
        }

        return std::move(triggers);
    }

    static Voice<Trigger> ids_to_pulse_offs(const Voice<PulseIdentifier>& ids) {
        return ids.as_type<Trigger>([](const PulseIdentifier& p) {
            return Trigger::pulse_off(p.id);
        });
    }


    Held<PulseIdentifier, true> m_pulses;
    Mode m_mode = Mode::sustain;
    bool m_immediate = true;


    bool m_closed = false;
};
}
#endif //SERIALIST_PULSE_FILTER_H
