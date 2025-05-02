
#ifndef TESTUTILS_CMS_H
#define TESTUTILS_CMS_H

#include "condition.h"

namespace serialist::test::cms {

template<typename T>
std::unique_ptr<VoicesComparison<T>> empty(std::size_t voice_index) {
    return std::make_unique<VoicesComparison<T>>([=](const Voices<T>& v) {
        if (voice_index >= v.size()) {
            return false;
        }

        return v[voice_index].empty();
    });
}


/**
 * @brief size of entire Voices<T> object
 */
template<typename T>
std::unique_ptr<FixedSizeComparison<T>> size(std::size_t voices_size) {
    return std::make_unique<FixedSizeComparison<T>>(voices_size, std::nullopt);
}


/**
 * @brief size of a single Voice<T> at a specific index
 */
template<typename T>
std::unique_ptr<VoicesComparison<T>> size(std::size_t voice_index, std::size_t size) {
    return std::make_unique<VoicesComparison<T>>([=](const Voices<T>& v) {
        if (voice_index >= v.size()) {
            return false;
        }

        return v[voice_index].size() == size;
    });
}

inline std::unique_ptr<FixedSizeComparison<Event>> sizee(std::size_t n) { return size<Event>(n); }


// ==============================================================================================
// TRIGGER COMPARISONS
// ==============================================================================================

inline std::unique_ptr<VoicesComparison<Trigger>> containst(Trigger::Type type
                                                            , std::size_t voice_index
                                                            , std::optional<std::size_t> id = std::nullopt) {
    return std::make_unique<VoicesComparison<Trigger>>([=](const Voices<Trigger>& v) {
        if (voice_index >= v.size()) {
            return false;
        }

        if (id) {
            return Trigger::contains(v[voice_index], type, *id);
        }
        return Trigger::contains(v[voice_index], type);
    });
}


inline std::unique_ptr<VoicesComparison<Trigger>> constainst_on(std::size_t voice_index
                                                                , std::optional<std::size_t> id = std::nullopt) {
    return containst(Trigger::Type::pulse_on, voice_index, id);
}


inline std::unique_ptr<VoicesComparison<Trigger>> containst_off(std::size_t voice_index
                                                                , std::optional<std::size_t> id = std::nullopt) {
    return containst(Trigger::Type::pulse_off, voice_index, id);
}


inline std::unique_ptr<VoicesComparison<Trigger>> equalst(Trigger::Type type
                                                          , std::size_t voice_index
                                                          , std::optional<std::size_t> id = std::nullopt) {
    return std::make_unique<VoicesComparison<Trigger>>([=](const Voices<Trigger>& v) {
        if (voice_index >= v.size()) {
            return false;
        }

        if (id) {
            return Trigger::contains(v[voice_index], type, *id);
        }
        return Trigger::contains(v[voice_index], type);
    });
}


inline std::unique_ptr<VoicesComparison<Trigger>> equalst(Trigger::Type type
                                                          , std::pair<std::size_t, std::size_t>&& voice_and_entry_index
                                                          , std::optional<std::size_t> id = std::nullopt) {
    return std::make_unique<VoicesComparison<Trigger>>([=](const Voices<Trigger>& v) {
        auto [voice_index, entry_index] = voice_and_entry_index;
        if (voice_index >= v.size()) {
            return false;
        }

        auto& voice = v[voice_index];

        if (entry_index >= voice.size()) {
            return false;
        }

        auto& trigger = voice[entry_index];

        if (id) {
            return trigger.is(type) && trigger.get_id() == *id;
        }
        return trigger.is(type);
    });
}


inline std::unique_ptr<VoicesComparison<Trigger>> equalst_on(std::size_t voice_index
                                                              , std::optional<std::size_t> id = std::nullopt) {
    return equalst(Trigger::Type::pulse_on, voice_index, id);
}


inline std::unique_ptr<VoicesComparison<Trigger>> equalst_on(std::pair<std::size_t, std::size_t>&& voice_and_entry_index
                                                              , std::optional<std::size_t> id = std::nullopt) {
    return equalst(Trigger::Type::pulse_on, std::move(voice_and_entry_index), id);
}


inline std::unique_ptr<VoicesComparison<Trigger>> equalst_off(std::size_t voice_index
                                                               , std::optional<std::size_t> id = std::nullopt) {
    return equalst(Trigger::Type::pulse_off, voice_index, id);
}


inline std::unique_ptr<VoicesComparison<Trigger>> equalst_off(std::pair<std::size_t, std::size_t>&& voice_and_entry_index
                                                               , std::optional<std::size_t> id = std::nullopt) {
    return equalst(Trigger::Type::pulse_off, std::move(voice_and_entry_index), id);
}


inline std::unique_ptr<VoicesComparison<Trigger>> sortedt() {
    return std::make_unique<VoicesComparison<Trigger>>([](const Voices<Trigger>& v) {
        for (const auto& voice : v) {
            if (!voice.is_sorted())
                return false;
        }
        return true;
    });
}


// ==============================================================================================
// EVENTS
// ==============================================================================================

inline std::unique_ptr<VoicesComparison<Event>> equalse(std::size_t voice_index, const NoteComparator& c) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::matches_chord({c}, vs[voice_index]);
    });
}


inline std::unique_ptr<VoicesComparison<Event>> equalse(std::size_t voice_index, const Vec<NoteComparator>& cs) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::matches_chord(cs, vs[voice_index]);
    });
}

inline std::unique_ptr<VoicesComparison<Event>> containse(std::size_t voice_index, const NoteComparator& c) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::contains(c, vs[voice_index]);
    });
}


inline std::unique_ptr<VoicesComparison<Event>> containse(std::size_t voice_index, const Vec<NoteComparator>& cs) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::contains_chord(cs, vs[voice_index]);
    });
}


}

#endif //TESTUTILS_CMS_H
