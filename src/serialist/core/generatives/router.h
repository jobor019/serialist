#ifndef SERIALIST_ROUTER_H
#define SERIALIST_ROUTER_H

#include "core/param/socket_handler.h"
#include "core/param/multi_socket.h"
#include "core/generative.h"
#include "core/types/trigger.h"
#include "core/types/facet.h"
#include "core/temporal/time_gate.h"
#include "sequence.h"
#include "variable.h"
#include "temporal/pulse.h"
#include "types/index.h"


namespace serialist {

template<typename T>
using MultiVoices = Vec<Voices<T>>;

enum class RouterMode { route, through, merge, split, mix, distribute };

enum class FlushMode { always , any_pulse /* , pulse_off TODO: Implement */ };

struct RouterDefaults {
    static constexpr auto MODE = RouterMode::route;
    static constexpr auto USE_INDEX = true;
    static constexpr auto FLUSH_MODE = FlushMode::always;
};


// ==============================================================================================


/**
 * @brief class to indicate changes in RouterMapping between two consecutive Router::process() calls
 */
struct MappingChange {

    enum class Type {
        // change from closed to open that never will require any dangling-related flushing,
        // e.g. voice was added, voice was routed on a previously inactive outlet, voice was removed. The latter
        // does require flushing, but will be always be handled in the MultiOutletHeldPulses::process step, which is
        // the only step that allows resizing
        no_change

        // change from open to closed that always will require flushing (e.g. outlet was routed off).
        , closed

        // change from open to open that might require flushing (e.g. voice was moved to a different outlet)
        , index_change
    };


    using SimpleChange = Vec<Type>;
    using NestedChange = Vec<Vec<Type>>;



    MappingChange() = delete;

    template<typename T>
    static SimpleChange compare_diff(const Vec<std::optional<T>>& from
                                  , const Vec<std::optional<T>>& to) {
        static_assert(utils::is_equality_comparable_v<T>);

        if (from == to) {
            return {};
        }

        // Our output diff should always correspond to the size of `to`, even in the cases where `from` is bigger
        auto diff = Vec<Type>::repeated(to.size(), Type::no_change);

        // Note: we're using `std::min` here, as
        //       - to > from: a voice has been added. No need to do any flushing on add
        //       - from < to: a voice has been removed. The dangling voice must be flushed, but we will ignore
        //                     it in this step as it will always be flushed in MultiOutletHeldPulses::process
        for (std::size_t i = 0; i < std::min(from.size(), to.size()) ; ++i) {
            if (!to[i] && from[i]) {
                // mapping was removed or changed from non-empty (open) to empty (closed)
                diff[i] = Type::closed;
            } else if (to[i] && from[i] && *to[i] != *from[i]) {
                // mapping changed from non-empty (open) to non-empty (open) with a different index
                diff[i] = Type::index_change;
            }
            // all other cases (voice was added, no change, etc.): no_change
        }
        return diff;
    }


    template<typename T>
    static SimpleChange compare_diff(const Vec<T>& from , const Vec<T>& to) {
        static_assert(utils::is_equality_comparable_v<T>);

        if (from == to) {
            return {};
        }

        auto changed_voices = Vec<Type>::repeated(to.size(), Type::no_change);

        // Note: std::min, see comment in other compare_diff overload
        for (std::size_t i = 0; i < std::min(from.size(), to.size()); ++i) {
            if (from[i] != to[i]) {
                changed_voices[i] = Type::index_change;
            }
            // otherwise: voice added or removed (no_change in both scenarios), or same index (no_change, obviously)
        }
        return changed_voices;
    }


    template<typename T>
    static NestedChange compare_nested_diff(const Vec<Vec<T>>& from, const Vec<Vec<T>>& to) {
        static_assert(utils::is_equality_comparable_v<T>);
        assert(from.size() == to.size()); // the number of inlets should never change at runtime

        auto num_inlets = from.size();

        auto changed_voices = Vec<Vec<Type>>::allocated(num_inlets);

        for (std::size_t outlet_index = 0; outlet_index < num_inlets; ++outlet_index) {
            auto& from_voices = from[outlet_index];
            auto& to_voices = to[outlet_index];

            changed_voices.append(compare_diff(from_voices, to_voices));
        }

        return changed_voices;
    }


};


// ==============================================================================================

// used by RouterModes `route` and `through`
class Route {
public:
    using MappingType = Vec<std::optional<std::size_t>>;


    explicit Route(MappingType mapping, bool is_through, bool is_voice_mapping)
        : m_mapping(std::move(mapping))
        , m_is_through(is_through)
        , m_is_voice_mapping(is_voice_mapping) {}

    /**
     * @param indices 1d list of indices, where each position corresponds to a given outlet (multi) or voice (single)
     * @param target_size
     * @param index_type
     * @param voice_mapping
     */
    static Route parse_route(const Voices<Facet>& indices
                             , std::size_t target_size
                             , Index::Type index_type
                             , bool voice_mapping) {
        assert(indices.size() >= target_size); // responsibility of caller
        auto firsts = indices.firsts<>();

        auto mapping = MappingType::allocated(target_size);
        for (std::size_t i = 0; i < target_size; ++i) {
            if (firsts[i]) {
                mapping.append(Index::from(*firsts[i], index_type, target_size).get_clip(firsts.size()));
            } else {
                mapping.append(std::nullopt);
            }
        }

        return Route{mapping, false, voice_mapping};
    }


    /**
     * @param enabled 1d boolean mask of the same size as number of inlets,
     *                where each position corresponds to a given outlet (multi) or voice (single)
     * @param target_size
     * @param voice_mapping
     */
    static Route parse_through(const Voices<Facet>& enabled, std::size_t target_size, bool voice_mapping) {
        assert(enabled.size() >= target_size); // responsibility of caller
        auto firsts = enabled.firsts();

        auto mapping = MappingType::allocated(target_size);
        for (std::size_t i = 0; i < target_size; ++i) {
            if (firsts[i] && static_cast<bool>(*firsts[i])) {
                mapping.append(i);
            } else {
                mapping.append(std::nullopt);
            }
        }

        return Route{mapping, true, voice_mapping};
    }


    static Vec<MappingChange::Type> changed(const Route& from, const Route& to) {
        const auto& from_map = from.mapping();
        const auto& to_map = to.mapping();

        // When changing mode between the two internal modes `route` and `through`, we flag all voices for flushing
        // If the mapping also shrinks, this is fine as this will be flushed by MultiOutletHeldPulses
        if (to.m_is_through != from.m_is_through) {
            return Vec<MappingChange::Type>::repeated(to_map.size(), MappingChange::Type::closed);
        }

        return MappingChange::compare_diff(from_map, to_map);
    }


    Vec<MappingChange::Type> get_changes(const Route& new_value) const {
        return changed(*this, new_value);
    }


    /**
     * @brief apply to 1-1 mapping (i.e. mapping defines individual voices in 1 inlet 1 outlet)
     */
    template<typename T>
    Voices<T> apply_single(Voices<T>&& input) const {
        if (m_mapping.empty()) {
            return Voices<T>::empty_like();
        }

        // This should be handled at parse time.
        // We need the spec to correspond to actual output in later stages (PulseRouter)
        assert(input.size() >= m_mapping.size());

        auto output = Vec<Voice<T>>::allocated(m_mapping.size());
        for (const auto& i : m_mapping) {
            if (i) {
                output.append(input[*i]);
            } else {
                output.append(Voice<T>{});
            }
        }
        return {Voices<T>{output}};
    }


    /**
     * @brief apply to 1-1 mapping (i.e. mapping defines inlet and outlets)
     */
    template<typename T>
    MultiVoices<T> apply_multi(MultiVoices<T>&& input) const {
        if (m_mapping.empty()) {
            return {Voices<T>::empty_like()};
        }

        // This should be handled at parse time.
        // We need the spec to correspond to actual output in later stages (PulseRouter)
        assert(input.size() >= m_mapping.size());

        auto output = MultiVoices<T>::allocated(m_mapping.size());
        for (const auto& i : m_mapping) {
            if (i) {
                output.append(input[*i]);
            } else {
                output.append(Voices<T>::empty_like());
            }
        }
        return output;
    }


    bool equals(const Route& other) const {
        return m_is_through == other.m_is_through && m_mapping == other.m_mapping;
    }


    bool is_empty() const { return m_mapping.empty(); }


    bool is_voice_mapping() const {
        return m_is_voice_mapping;
    }


    const MappingType& mapping() const { return m_mapping; }

private:
    MappingType m_mapping;
    bool m_is_through; // Stored to be track changes between modes route and through
    bool m_is_voice_mapping; // True if mapping between individual voices, false if mapping between inlets
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class Merge {
public:
    explicit Merge(Vec<std::size_t> mapping) : m_mapping(std::move(mapping)) {}


    static Merge parse(const Voices<Facet>& counts
                             , std::size_t num_active_inlets
                             , const Vec<std::size_t>& voice_counts_per_inlet
                             , Index::Type index_type) {
        assert(counts.size() >= num_active_inlets);
        assert(voice_counts_per_inlet.size() >= num_active_inlets);

        auto output_count = Vec<std::size_t>::zeros(num_active_inlets);
        for (std::size_t i = 0; i < num_active_inlets; ++i) {
            if (auto count = counts[i].first()) {
                auto num_voices = voice_counts_per_inlet[i];
                output_count[i] = Index::from(*count, index_type, num_voices).get_clip(num_voices);
            }
            // otherwise 0
        }

        return Merge{output_count};
    }


    static Vec<MappingChange::Type> changed(const Merge& from, const Merge& to) {
        // Comparing two merge mappings is significantly faster than inverting both and comparing the inverse mappings
        if (from.mapping() == to.mapping()) {
            return {};
        }

        // When converted to index maps, we can use the normal compare_diff to compare the two
        return MappingChange::compare_diff(from.inverse(), to.inverse());
    }


    Vec<MappingChange::Type> get_changes(const Merge& new_value) const {
        return changed(*this, new_value);
    }


    template<typename T>
    MultiVoices<T> apply(MultiVoices<T>&& input) {
        assert(input.size() >= m_mapping.size());

        auto merged = Vec<Voice<T>>::allocated(m_mapping.sum());

        for (std::size_t i = 0; i < m_mapping.size(); ++i) {
            if (m_mapping[i] > 0) {
                merged.extend(input[i].vec().slice(0, m_mapping[i]));
            }
        }

        if (merged.empty()) {
            return MultiVoices<T>::singular(Voices<T>::empty_like());
        }

        return MultiVoices<T>::singular(Voices<T>{std::move(merged)});

    }


    bool equals(const Merge& other) const {
        return m_mapping == other.m_mapping;
    }


    /**
     * @return the list of inlets each voice corresponds to,
     *         e.g. if we have mapping (count) [3 2 1], the inverse would be [0 0 0 1 1 2] (inlets)
     */
    Vec<std::size_t> inverse() const {
        auto inversed = Vec<std::size_t>::allocated(m_mapping.sum());

        for (std::size_t i = 0; i < m_mapping.size(); ++i) {
            for (std::size_t j = 0; j < m_mapping[i]; ++j) {
                inversed.append(i);
            }
        }
        return inversed;
    }

    bool is_empty() const { return m_mapping.empty(); }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    bool is_voice_mapping() const {
        return true; // indices in `changed` comparisons always corresponds to individual voices, not inlets
    }


    const Vec<std::size_t>& mapping() const { return m_mapping; }

private:
    Vec<std::size_t> m_mapping;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class Split {
public:
    explicit Split(Vec<std::size_t> mapping) : m_mapping(std::move(mapping)) {}


    static Split parse(const Voices<Facet>& counts
                       , std::size_t num_active_outlets
                       , std::size_t num_voices_total
                       , Index::Type index_type) {
        assert(counts.size() >= num_active_outlets);

        auto output_count = Vec<std::size_t>::zeros(num_active_outlets);

        std::size_t current_count = 0;

        for (std::size_t i = 0; i < num_active_outlets; ++i) {
            if (auto first = counts[i].first()) {
                auto count = static_cast<std::size_t>(
                    Index::from(*first, index_type, num_voices_total).get_clip(num_voices_total)
                );

                if (current_count + count >= num_voices_total) {
                    output_count[i] = num_voices_total - current_count;
                    break; // remaining outlets will be 0
                }
                output_count[i] = count;
                current_count += count;
            }
        }

        return Split{output_count};
    }


    static Vec<Vec<MappingChange::Type>> changed(const Split& from, const Split& to) {
        // Comparing two split mappings is significantly faster than inverting both and comparing the inverse mappings
        if (from.mapping() == to.mapping()) {
            return {};
        }

        // an inverse mapping is exactly the same as a Distribute mapping, without any optionals
        return MappingChange::compare_nested_diff(from.inverse(), to.inverse());
    }


    Vec<Vec<MappingChange::Type>> get_changes(const Split& new_value) const {
        return changed(*this, new_value);
    }


    template<typename T>
    MultiVoices<T> apply(Voices<T>&& input) {
        auto split = MultiVoices<T>::repeated(m_mapping.size(), Voices<T>::empty_like());

        std::size_t start = 0;

        for (std::size_t i = 0; i < m_mapping.size(); ++i) {
            if (m_mapping[i] > 0) {
                auto end = start + m_mapping[i];
                split[i] = Voices<T>{input.vec().slice(start, end)};

                start = end;
            }
        }

        return split;
    }


    bool equals(const Split& other) const {
        return m_mapping == other.m_mapping;
    }

    Vec<Vec<std::size_t>> inverse() const {
        auto num_outlets = m_mapping.size();
        auto inversed = Vec<Vec<std::size_t>>::allocated(num_outlets);

        std::size_t voice_index = 0;
        for (std::size_t outlet_index = 0; outlet_index < num_outlets; ++outlet_index) {
            auto count = m_mapping[outlet_index];
            inversed.append(Vec<std::size_t>::range(voice_index, voice_index + count));
            voice_index += count;
        }

        return inversed;
    }


    bool is_empty() const { return m_mapping.empty(); }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    bool is_voice_mapping() const {
        return false; // irrelevant for split
    }


    const Vec<std::size_t>& mapping() const { return m_mapping; }

private:
    Vec<std::size_t> m_mapping;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


class Mix {
public:
    using InletIndex = std::size_t;
    using VoiceIndex = std::size_t;

    using ElementType = std::optional<std::pair<InletIndex, VoiceIndex>>;
    using MappingType = Vec<ElementType>;

    explicit Mix(MappingType mapping) : m_mapping(std::move(mapping)) {}


    // Note: while it may seem redundant to first parse the spec, then apply it, the spec itself is required
    //       for the PulseRouter to determine which voices should be flushed
    template<typename T>
    static Mix parse(const Voices<Facet>& inlet_and_voice_idx, const MultiVoices<T>& input, Index::Type index_type) {
        auto mapping = MappingType::allocated(inlet_and_voice_idx.size());

        for (const auto& iv : inlet_and_voice_idx) {
            mapping.append(parse_element(iv, input, index_type));
        }

        return Mix{mapping};
    }


    static Vec<MappingChange::Type> changed(const Mix& from, const Mix& to) {
        return MappingChange::compare_diff(from.mapping(), to.mapping());
    }


    Vec<MappingChange::Type> get_changes(const Mix& new_value) const {
        return changed(*this, new_value);
    }


    template<typename T>
    MultiVoices<T> apply(MultiVoices<T>&& input) {
        if (m_mapping.empty()) {
            return {Voices<T>::empty_like()};
        }

        auto mixed = Vec<Voice<T>>::allocated(m_mapping.size());

        for (const auto& elem : m_mapping) {
            if (elem) {
                mixed.append(input[elem->first][elem->second]);
            } else {
                mixed.append(Voice<T>{});
            }
        }

        return MultiVoices<T>::singular(Voices<T>{std::move(mixed)});
    }


    bool equals(const Mix& other) const {
        return m_mapping == other.m_mapping;
    }

    bool is_empty() const { return m_mapping.empty(); }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    bool is_voice_mapping() const {
        return true; // indices in `changed` comparisons always corresponds to individual voices, not inlets
    }


    const MappingType& mapping() const { return m_mapping; }

private:
    template<typename T>
    static ElementType parse_element(const Voice<Facet>& index_and_element
                                     , const MultiVoices<T>& input
                                     , Index::Type index_type) {
        // We expect each element to be a tuple (inlet_index, voice_index)
        if (index_and_element.size() < 2) {
            return std::nullopt;
        }

        auto num_inlets = input.size();
        auto inlet_index = Index::from(index_and_element[0], index_type, num_inlets).get_pass(num_inlets);

        if (!inlet_index) {
            return std::nullopt; // inlet index out of bounds
        }

        auto i = static_cast<std::size_t>(*inlet_index);

        auto num_voices = input[i].size();
        auto voice_index = Index::from(index_and_element[1], index_type, num_voices).get_pass(num_voices);

        if (!voice_index) {
            return std::nullopt; // voice index out of bounds
        }

        auto j = static_cast<std::size_t>(*voice_index);
        return std::pair{i, j};
    }


    MappingType m_mapping;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



class Distribute {
public:
    using ElementType = Vec<std::optional<std::size_t>>; // position in vec corresponds to voice index in inlet,
                                                         // value to index in input voice
    using MappingType = Vec<ElementType>;                // position in vec corresponds to outlet index

    explicit Distribute(MappingType mapping) : m_mapping(std::move(mapping)) {}


    template<typename T>
    static Distribute parse(const Voices<Facet>& list_of_indices, const Voices<T>& input, Index::Type t) {
        auto mapping = MappingType::allocated(list_of_indices.size());

        for (const auto& is : list_of_indices) {
            mapping.append(parse_element(is, input, t));
        }

        return Distribute{mapping};
    }

    static Vec<Vec<MappingChange::Type>> changed(const Distribute& from, const Distribute& to) {
        return MappingChange::compare_nested_diff(from.mapping(), to.mapping());
    }

    Vec<Vec<MappingChange::Type>> get_changes(const Distribute& new_value) const {
        return changed(*this, new_value);
    }


    template<typename T>
    MultiVoices<T> apply(Voices<T>&& input) {
        auto distributed = Vec<Vec<Voice<T>>>::repeated(m_mapping.size(), Vec<Voice<T>>{});

        for (std::size_t inlet_index = 0; inlet_index < m_mapping.size(); ++inlet_index) {
            for (const auto& voice_index : m_mapping[inlet_index]) {
                if (voice_index) {
                    distributed[inlet_index].append(input[*voice_index]);
                } else {
                    distributed[inlet_index].append(Voice<T>{});
                }
            }
        }

        return MultiVoices<T>{
            distributed.template as_type<Voices<T>>([](const Vec<Voice<T>>& v) -> Voices<T> {
                if (v.empty()) {
                    return Voices<T>::empty_like();
                }
                return Voices<T>{v};
            })
        };
    }


    bool equals(const Distribute& other) const {
        return m_mapping == other.m_mapping;
    }

    bool is_empty() const { return m_mapping.empty(); }


    // ReSharper disable once CppMemberFunctionMayBeStatic
    bool is_voice_mapping() const {
        return false; // irrelevant
    }


    const MappingType& mapping() const { return m_mapping; }


private:
    template<typename T>
        static ElementType parse_element(const Voice<Facet>& element_spec, const Voices<T>& input, Index::Type t) {
        // We expect each element to be a list of zero or more elements corresponding to indices in `input`
        if (element_spec.empty()) {
            return {};
        }

        auto num_voices = input.size();

        auto indices = ElementType::allocated(element_spec.size());

        for (const auto& e : element_spec) {
            if (auto voice_index = Index::from(e, t, num_voices).get_pass(num_voices)) {
                indices.append(static_cast<std::size_t>(*voice_index));
            } else {
                indices.append(std::nullopt); // voice index out of bounds
            }
        }

        return indices;
    }


    MappingType m_mapping;
};


// ==============================================================================================

class RouterMapping {
public:
    // Note: it's critical that all mapping adhere to the actual size of the input,
    //       as we're using these to determine whether we need to flush certain voices / inlets in PulseRouter.
    //       This is why all mappings first compute their spec and then apply it,
    //       rather than doing both in the same step.

    using MappingType = std::variant<Route, Merge, Split, Mix, Distribute>;

    explicit RouterMapping(MappingType mapping) : m_mapping(std::move(mapping)) {}


    static RouterMapping empty(RouterMode router_mode) {
        switch (router_mode) {
            case RouterMode::route:
                return RouterMapping{Route{{}, false, false}};
            case RouterMode::through:
                return RouterMapping{Route{{}, true, false}};
            case RouterMode::merge:
                return RouterMapping{Merge{{}}};
            case RouterMode::split:
                return RouterMapping{Split{{}}};
            case RouterMode::mix:
                return RouterMapping{Mix{{}}};
            case RouterMode::distribute:
                return RouterMapping{Distribute{{}}};
            default:
                throw std::invalid_argument("invalid router mode encountered in RouterMapping");
        }
    }


    template<typename T>
    bool is() const noexcept {
        return std::holds_alternative<T>(m_mapping);
    }


    template<typename T>
    T as() const {
        return std::get<T>(m_mapping);
    }

    bool is_empty() const {
        return std::visit([](const auto& mapping) {
            return mapping.is_empty();
        }, m_mapping);
    }


    bool matches(const RouterMapping& other) const {
        return std::visit([](const auto& a, const auto& b) -> bool {
            using A = std::decay_t<decltype(a)>;
            using B = std::decay_t<decltype(b)>;
            if constexpr (std::is_same_v<A, B>) {
                return a.equals(b);
            } else {
                return false;
            }
        }, m_mapping, other.m_mapping);
    }


    MultiVoices<Trigger> flush_dangling_triggers(const MultiVoices<Trigger>& output_triggers
                                                  , std::optional<RouterMapping>& previous_mapping
                                                  , MultiOutletHeldPulses& held
                                                  , FlushMode flush_mode) const {
        // First input, no need to compare anything
        if (!previous_mapping) {
            return {};
        }

        auto& m = previous_mapping->m_mapping;

        return std::visit([this, &output_triggers, &m, &held, &flush_mode](const auto& mapping) {
            using T = std::decay_t<decltype(mapping)>;

            if (auto* other = std::get_if<T>(&m)) {
                auto changes = other->get_changes(mapping);

                if (changes.empty()) {
                    return MultiVoices<Trigger>{};
                }

                bool is_voice_mapping = mapping.is_voice_mapping();

                if constexpr (std::is_same_v<decltype(changes), MappingChange::SimpleChange>) {
                    return flush_dangling_simple(changes, held, output_triggers, flush_mode, is_voice_mapping);
                } else if constexpr (std::is_same_v<decltype(changes), MappingChange::NestedChange>) {
                    return flush_dangling_nested(changes, held, output_triggers, flush_mode);
                } else {
                    throw std::runtime_error("Invalid change type encountered");
                }
            }

            // otherwise: mapping type changed between consecutive calls: flush all voices up to output size
            //            Note: we do not allow resizing here. We ignore any voice beyond the size of our map,
            //            as these will be flushed at a later stage.
            auto num_outlets = output_triggers.size();
            auto flushed = MultiVoices<Trigger>::allocated(num_outlets);
            for (std::size_t i = 0; i < num_outlets; ++i) {
                auto target_voice_count = output_triggers[i].size();
                flushed.append(held.flush_outlet(i, target_voice_count));
            }
            return flushed;

        }, m_mapping);
    }

private:
    template <typename T>
    auto changed(const T& from, const T& to) {
        // Declared outside dependent context of `handle_trigger_flushing` to avoid compiler warnings
        return T::changed(from, to);
    }


    /**
     * @brief modes route, through, merge and mix
     */
    static MultiVoices<Trigger> flush_dangling_simple(const MappingChange::SimpleChange& changes
                                                      , MultiOutletHeldPulses& held
                                                      , const MultiVoices<Trigger>& output_triggers
                                                      , FlushMode flush_mode
                                                      , bool is_voice_mapping) {
        if (changes.empty()) {
            return {};
        }

        // mapping refers to voices indices in the first outlet
        if (is_voice_mapping) {
            assert(output_triggers.size() == 1);
            return {flush_dangling_voices(0, changes, held, output_triggers, flush_mode)};
        }

        // otherwise: mapping refers to entire outlets
        assert(output_triggers.size() == changes.size());
        return flush_dangling_outlets(changes, held, output_triggers, flush_mode);
    }


    static MultiVoices<Trigger> flush_dangling_nested(const MappingChange::NestedChange& changes
                                                      , MultiOutletHeldPulses& held
                                                      , const MultiVoices<Trigger>& output_triggers
                                                      , FlushMode flush_mode) {
        if (changes.empty()) {
            return {};
        }

        auto num_outlets = changes.size();
        auto flushed = MultiVoices<Trigger>::allocated(num_outlets);

        for (std::size_t outlet_index = 0; outlet_index < num_outlets; ++outlet_index) {
            flushed.append(flush_dangling_voices(outlet_index
                                                 , changes[outlet_index]
                                                 , held
                                                 , output_triggers
                                                 , flush_mode));
        }

        return flushed;
    }

    /**
     * @brief flush dangling for when the indices in `changes` correspond to entire outlets
     */
    static MultiVoices<Trigger> flush_dangling_outlets(const MappingChange::SimpleChange& changes
                                                       , MultiOutletHeldPulses& held
                                                       , const MultiVoices<Trigger>& output_triggers
                                                       , FlushMode flush_mode) {
        auto num_outlets = changes.size();
        auto flushed = MultiVoices<Trigger>::repeated(num_outlets, Voices<Trigger>::empty_like());

        for (std::size_t outlet_index = 0; outlet_index < num_outlets; ++outlet_index) {
            auto target_voice_count = output_triggers[outlet_index].size();

            if (changes[outlet_index] == MappingChange::Type::index_change) {

                if (flush_mode == FlushMode::always || !output_triggers.empty()) {
                    flushed[outlet_index] = held.flush_outlet(outlet_index, target_voice_count);
                } else {
                    held.flag_as_triggered(outlet_index);
                }

            } else if (changes[outlet_index] == MappingChange::Type::closed) {
                flushed[outlet_index] = held.flush_outlet(outlet_index, target_voice_count);
            }
            // else: no flush needed
        }

        return flushed;
    }


    /**
     * @brief flush dangling for when the indices in `changes` correspond to individual voices on a specific inlet
     */
    static Voices<Trigger> flush_dangling_voices(std::size_t outlet_index
                                                 , const MappingChange::SimpleChange& changes
                                                 , MultiOutletHeldPulses& held
                                                 , const MultiVoices<Trigger>& output_triggers
                                                 , FlushMode flush_mode) {
        auto num_voices = changes.size();
        auto flushed = Vec<Vec<Trigger>>::repeated(num_voices, Vec<Trigger>{});

        for (std::size_t voice_index = 0; voice_index < num_voices; ++voice_index) {
            if (changes[voice_index] == MappingChange::Type::index_change) {
                if (flush_mode == FlushMode::always) {
                    flushed[voice_index] = held.flush_voice(outlet_index, voice_index, true);
                } else {
                    held.flag_as_triggered(outlet_index, voice_index, true);
                }

            } else if (changes[voice_index] == MappingChange::Type::closed) {
                flushed[voice_index] = held.flush_voice(outlet_index, voice_index, true);
            }
            // else: no flush needed
        }

        return Voices<Trigger>{flushed};
    }



    MappingType m_mapping;
};


// ==============================================================================================

template<typename T>
class Router {
public:
    Router(std::size_t num_inlets, std::size_t num_outlets)
        : m_num_inlets(num_inlets)
        , m_num_outlets(num_outlets) {
        assert(m_num_inlets > 0);
        assert(m_num_outlets > 0);
    }


    std::pair<MultiVoices<T>, RouterMapping> process(MultiVoices<T>&& input
                                                     , const Voices<Facet>& mapping
                                                     , RouterMode mode
                                                     , Index::Type index_type) {
        assert(input.size() == m_num_inlets);

        if (mapping.is_empty_like()) {
            return default_empty(mode);
        }

        // Single only supports modes route and through, hence the separate implementation
        if (m_num_inlets == 1 && m_num_outlets == 1) {
            if (mode == RouterMode::through) {
                return through_single(std::move(input), mapping);
            }

            // All other modes: default to `route` in the single inlet single outlet scenario
            return route_single(std::move(input), mapping, index_type);
        }

        switch (mode) {
            case RouterMode::through: return through_multi(std::move(input), mapping);
            case RouterMode::merge: return merge(std::move(input), mapping, index_type);
            case RouterMode::split: return split(std::move(input), mapping, index_type);
            case RouterMode::mix: return mix(std::move(input), mapping, index_type);
            case RouterMode::distribute: return distribute(std::move(input), mapping, index_type);
            default: return route_multi(std::move(input), mapping, index_type);
        }
    }


    /**
     * @brief append empty voices to any non-routed outlet, to ensure that every outlet always has a value
     */
    MultiVoices<T> adjust_size(MultiVoices<T>&& output) const {
        assert(output.size() <= m_num_outlets);

        if (output.size() == m_num_outlets)
            return output;

        for (std::size_t outlet_index = output.size(); outlet_index < m_num_outlets; ++outlet_index) {
            output.append(Voices<T>::empty_like());
        }

        return output;

    }

private:
    std::pair<MultiVoices<T>, RouterMapping> route_single(MultiVoices<T>&& input
                                                          , const Voices<Facet>& indices
                                                          , Index::Type index_type) {
        auto& voices = input[0];
        auto num_active_voices = std::min(voices.size(), indices.size());

        auto route_mapping = Route::parse_route(indices, num_active_voices, index_type, true);
        auto output = route_mapping.apply_single(std::move(voices));

        return {{output}, RouterMapping{route_mapping}};
    }


    std::pair<MultiVoices<T>, RouterMapping> route_multi(MultiVoices<T>&& input
                                                         , const Voices<Facet>& indices
                                                         , Index::Type index_type) {

        auto num_active_outlets = std::min({input.size(), indices.size(), m_num_outlets});

        auto route_mapping = Route::parse_route(indices, num_active_outlets, index_type, false);
        auto output = route_mapping.apply_multi(std::move(input));

        return {output, RouterMapping{route_mapping}};
    }


    std::pair<MultiVoices<T>, RouterMapping> through_single(MultiVoices<T>&& input, const Voices<Facet>& boolean_mask) {
        auto& voices = input[0];
        auto num_active_voices = std::min(voices.size(), boolean_mask.size());

        auto route_mapping = Route::parse_through(boolean_mask, num_active_voices, true);
        auto output = route_mapping.apply_single(std::move(voices));

        return {{output}, RouterMapping{route_mapping}};
    }


    std::pair<MultiVoices<T>, RouterMapping> through_multi(MultiVoices<T>&& input, const Voices<Facet>& boolean_mask) {
        auto num_active_outlets = std::min({input.size(), boolean_mask.size(), m_num_outlets});

        auto route_mapping = Route::parse_through(boolean_mask, num_active_outlets, false);
        auto output = route_mapping.apply_multi(std::move(input));

        return {output, RouterMapping{route_mapping}};
    }


    /**
     * Note: if phase (is_index=false): value corresponds to relative voice count [0, 1) of that particular inlet
     */
    std::pair<MultiVoices<T>, RouterMapping> merge(MultiVoices<T>&& input
                                                   , const Voices<Facet>& counts
                                                   , Index::Type index_type) {
        auto num_active_inlets = std::min(input.size(), counts.size());
        auto voice_count_per_inlet = voice_counts(input);

        auto mapping = Merge::parse(counts, num_active_inlets, voice_count_per_inlet, index_type);
        auto output = mapping.apply(std::move(input));

        return {output, RouterMapping{mapping}};
    }


    /**
     * Note: if phase (is_index=false): corresponds to fraction of total voice count from inlet.
     */
    std::pair<MultiVoices<T>, RouterMapping> split(MultiVoices<T>&& input
                                                   , const Voices<Facet>& counts
                                                   , Index::Type index_type) {
        auto& voices = input[0];
        auto num_active_outlets = std::min(m_num_outlets, counts.size());

        auto mapping = Split::parse(counts, num_active_outlets, voices.size(), index_type);
        auto output = mapping.apply(std::move(voices));

        return {output, RouterMapping{mapping}};
    }


    std::pair<MultiVoices<T>, RouterMapping> mix(MultiVoices<T>&& input
                                                 , const Voices<Facet>& spec
                                                 , Index::Type index_type) {
        auto mapping = Mix::parse(spec, input, index_type);
        auto output = mapping.apply(std::move(input));

        return {output, RouterMapping{mapping}};
    }


    std::pair<MultiVoices<T>, RouterMapping> distribute(MultiVoices<T>&& input
                                                        , const Voices<Facet>& spec
                                                        , Index::Type index_type) {
        auto& voices = input[0];
        auto mapping = Distribute::parse(spec, voices, index_type);
        auto output = mapping.apply(std::move(voices));

        return {output, RouterMapping{mapping}};
    }

public:
    std::pair<MultiVoices<T>, RouterMapping> default_empty(RouterMode mode) const {
        return {
            MultiVoices<T>::repeated(m_num_outlets, Voices<T>::empty_like())
            , RouterMapping::empty(mode)
        };
    }


    std::size_t num_inlets() const { return m_num_inlets; }
    std::size_t num_outlets() const { return m_num_outlets; }

private:
    static Vec<std::size_t> voice_counts(const MultiVoices<T>& m) {
        return m.template as_type<std::size_t>([](const Voices<T>& v) {
            return v.size();
        });
    }


    const std::size_t m_num_inlets;
    const std::size_t m_num_outlets;
};


// ==============================================================================================

template<typename T>
class RouterBase {
public:
    RouterBase() = default;
    virtual ~RouterBase() = default;
    RouterBase(const RouterBase&) = default;
    RouterBase& operator=(const RouterBase&) = default;
    RouterBase(RouterBase&&) noexcept = default;
    RouterBase& operator=(RouterBase&&) noexcept = default;

    virtual MultiVoices<T> process(MultiVoices<T>&& input
                                   , const Voices<Facet>& spec
                                   , RouterMode mode
                                   , Index::Type index_type
                                   , FlushMode flush_mode) = 0;

    virtual std::optional<MultiVoices<T>> flush() = 0;

    virtual MultiVoices<T> default_empty() const { return MultiVoices<T>{}; }

    virtual std::size_t num_inlets() const = 0;
    virtual std::size_t num_outlets() const = 0;
};


// ==============================================================================================


class FacetRouter : public RouterBase<Facet> {
public:
    FacetRouter(std::size_t num_inlets, std::size_t num_outlets): m_router(num_inlets, num_outlets) {}


    MultiVoices<Facet> process(MultiVoices<Facet>&& input
                               , const Voices<Facet>& spec
                               , RouterMode mode
                               , Index::Type index_type
                               , FlushMode) override {
        // For Facet input, we simply discard the Mapping as we have no state to handle on flush
        auto output = m_router.process(std::move(input), spec, mode, index_type).first;

        return m_router.adjust_size(std::move(output));
    }


    std::optional<MultiVoices<Facet>> flush() override {
        // flushing is not a relevant operation for Facet values
        return std::nullopt;
    }


    std::size_t num_inlets() const override { return m_router.num_inlets(); }
    std::size_t num_outlets() const override { return m_router.num_outlets(); }

private:

    Router<Facet> m_router;

};


// ==============================================================================================


class PulseRouter : public RouterBase<Trigger> {
public:

    PulseRouter(std::size_t num_inlets, std::size_t num_outlets)
    : m_router(num_inlets, num_outlets)
    , m_held(num_outlets) {}


    MultiVoices<Trigger> process(MultiVoices<Trigger>&& input
                                 , const Voices<Facet>& spec
                                 , RouterMode mode
                                 , Index::Type index_type
                                 , FlushMode flush_mode) override {
        // The procedure here is as follows:
        //
        // - (a) we process the current routing, generating `output` containing routed pulse_ons and pulse_offs
        //
        // - (b) if the mapping has changed (by `spec` changing or by changes in voice count on any inlet), we
        //       flush or flag (depending on FlushMode) any dangling voices in `m_held`.
        //       Note that the output from this step will always have the same size as `output`, meaning that,
        //       any dangling voices resulting from changes in size (shrink) will not be handled in this step,
        //       but rather in MultiOutletHeldPulses::process.
        //       Also note that the flushed pulses will be inserted at the _start_ of each corresponding voice in
        //       `output`.
        //
        // - (c) we pass `output` into MultiOutletHeldPulses::process, which will do four things:
        //       1. resize the internal `MultiVoiceHeld` container of every Voices object to the corresponding size
        //          in `output`, flushing dangling voices if the size is shrunk.
        //       2. bind any new pulse_on that exists in output
        //       3. release any pulse_off that exists in output (without appending to `output`, obviously)
        //       4. if a voice is non-empty and any of its pulses are flagged as triggered, append them at the _start_
        //          of the corresponding voice in `output`
        //
        // - (d) we append our flushed dangling pulse offs at the _start_ of `output`
        //

        auto [output, mapping] = m_router.process(std::move(input), spec, mode, index_type);

        // Since MultioutletHeldPulses cannot distinguish between a single voice with no output (Voices::empty_like)
        // and a completely empty output resulting from an empty mapping (Voices::empty_like too),
        // we need to handle this case separately by flushing all outlets
        if (mapping.is_empty()) {
            // If the mapping is empty, no output should've passed through
            assert(output.size() == 1 && output[0].is_empty_like());
            m_previous_mapping = std::move(mapping);
            return m_held.flush();
        }

        auto flushed = mapping.flush_dangling_triggers(output, m_previous_mapping, m_held, flush_mode);

        m_held.process(output);

        MultiOutletHeldPulses::merge_into(output, std::move(flushed));

        m_previous_mapping = std::move(mapping);

        return m_router.adjust_size(std::move(output));
    }


    std::optional<MultiVoices<Trigger>> flush() override {
        return m_held.flush();
    }

    std::size_t num_inlets() const override { return m_router.num_inlets(); }
    std::size_t num_outlets() const override { return m_router.num_outlets(); }

private:
    Router<Trigger> m_router;
    MultiOutletHeldPulses m_held; // fixed size, always same as num outlets

    std::optional<RouterMapping> m_previous_mapping = std::nullopt;
};


// ==============================================================================================

template<typename T>
class RouterNode : public MultiNode<T> {
public:
    static constexpr bool IS_TRIGGER = std::is_same_v<T, Trigger>;


    struct Keys {
        static const inline std::string INPUT = "input";
        static const inline std::string ROUTING_MAP = "routing_map";
        static const inline std::string MODE = "mode";
        static const inline std::string USES_INDEX = "uses_index";
        static const inline std::string FLUSH_MODE = "flush_mode";

        static const inline std::string CLASS_NAME = "router";
    };


    RouterNode(const std::string& id
               , ParameterHandler& parent
               , std::size_t num_outlets
               , Vec<Node<T>*> inputs
               , Node<Facet>* routing_map = nullptr
               , Node<Facet>* mode = nullptr
               , Node<Facet>* uses_index = nullptr
               , Node<Facet>* flush_mode = nullptr
               , Node<Facet>* enabled = nullptr)
        : m_parameter_handler(Specification(param::types::generative)
                              .with_identifier(id)
                              .with_static_property(param::properties::template_class, Keys::CLASS_NAME)
                              , parent)
        , m_socket_handler(m_parameter_handler)
        , m_inputs(std::move(inputs), m_socket_handler, Keys::INPUT)
        , m_routing_map(m_socket_handler.create_socket<Facet>(Keys::ROUTING_MAP, routing_map))
        , m_mode(m_socket_handler.create_socket<Facet>(Keys::MODE, mode))
        , m_uses_index(m_socket_handler.create_socket<Facet>(Keys::USES_INDEX, uses_index))
        , m_flush_mode(m_socket_handler.create_socket<Facet>(Keys::FLUSH_MODE, flush_mode))
        , m_enabled(m_socket_handler.create_socket<Facet>(param::properties::enabled, enabled)) {
        if constexpr (IS_TRIGGER) {
            m_router = std::make_unique<PulseRouter>(m_inputs.size(), num_outlets);
        } else {
            m_router = std::make_unique<FacetRouter>(m_inputs.size(), num_outlets);
        }
    }


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    Vec<Voices<T>> process() override {
        if (auto t = m_time_gate.pop_time(); !t) {
            return m_current_value;
        }

        if (auto flushed = process_enabled_state()) {
            m_current_value = *flushed;
            return m_current_value;
        }

        auto mode = m_mode.process().first_or(RouterDefaults::MODE);
        auto uses_index = m_uses_index.process().first_or(RouterDefaults::USE_INDEX);
        auto flush_mode = m_flush_mode.process().first_or(RouterDefaults::FLUSH_MODE);

        auto index_type = uses_index ? Index::Type::index : Index::Type::phase;

        auto input = m_inputs.process();
        auto routing_map = m_routing_map.process();

        assert(input.size() == m_router->num_inlets());

        m_current_value = m_router->process(std::move(input), routing_map, mode, index_type, flush_mode);
        return m_current_value;
    }


    std::vector<Generative*> get_connected() override { return m_socket_handler.get_connected(); }
    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }
    void disconnect_if(Generative& connected_to) override { m_socket_handler.disconnect_if(connected_to); }

private:
    std::optional<MultiVoices<T>> process_enabled_state() {
        if constexpr (IS_TRIGGER) {
            if (auto state = m_enabled_gate.update(is_enabled()); state == EnabledState::disabled_this_cycle) {
                return m_router->flush();

            } else if (state == EnabledState::disabled_previous_cycle || state == EnabledState::disabled) {
                return m_router->default_empty();
            }

        } else {
            if (!is_enabled()) {
                return m_router->default_empty();
            }
        }

        return std::nullopt;
    }


    bool is_enabled() {
        return m_enabled.process().first_or(true) && m_inputs.any_is_connected() && m_routing_map.is_connected();
    }


    std::optional<MultiVoices<T>>handle_enabled_state(EnabledState state) {
        if (state == EnabledState::disabled_this_cycle) {
            return m_router->flush();
        } else if (state == EnabledState::disabled_previous_cycle || state == EnabledState::disabled) {
            return Voices<Trigger>::empty_like();
        }
        return std::nullopt;
    }


    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

    std::unique_ptr<RouterBase<T>> m_router;

    TimeGate m_time_gate;
    EnabledGate m_enabled_gate;

    MultiSocket<T> m_inputs;         // Note: this is not registered in the SocketHandler!
    Socket<Facet>& m_routing_map;    // Sequence

    Socket<Facet>& m_mode;          // Variable
    Socket<Facet>& m_uses_index;    // Variable
    Socket<Facet>& m_flush_mode;    // Variable

    Socket<Facet>& m_enabled;

    MultiVoices<T> m_current_value;
};


// ==============================================================================================

template<typename T, typename FloatType = double>
struct RouterWrapper {
    using Keys = typename RouterNode<T>::Keys;

    using SequenceType = std::conditional_t<
        std::is_same_v<T, Facet>,
        Sequence<Facet, double>,
        Sequence<Trigger>
    >;


    RouterWrapper(std::size_t num_inlets, std::size_t num_outlets)
        : inputs(create_inputs(num_inlets, ph))
        , router_node(Keys::CLASS_NAME
                      , ph
                      , num_outlets
                      , cast_inputs(inputs)
                      , &routing_map
                      , &mode
                      , &uses_index
                      , &flush_mode
                      , &enabled) {
        static_assert(std::is_same_v<T, Trigger> || std::is_same_v<T, Facet>);
    }


    static Vec<std::unique_ptr<SequenceType>> create_inputs(std::size_t num_inlets, ParameterHandler& ph) {
        auto v = Vec<std::unique_ptr<SequenceType>>::allocated(num_inlets);
        for (std::size_t i = 0; i < num_inlets; ++i) {
            v.append(std::make_unique<SequenceType>(Keys::INPUT + std::to_string(i), ph));
        }

        return v;
    }


    template<typename U = FloatType>
    void set_input(std::size_t index, const Voices<U>& value) {
        static_assert(std::is_same_v<U, FloatType> || std::is_same_v<U, Trigger>);
        assert(index < inputs.size());

        inputs[index]->set_values(value);
    }


    ParameterHandler ph;

    Vec<std::unique_ptr<SequenceType>> inputs;
    Sequence<Facet, FloatType> routing_map{Keys::ROUTING_MAP, ph};
    Variable<Facet, RouterMode> mode{Keys::MODE, ph, RouterDefaults::MODE};
    Variable<Facet, bool> uses_index{Keys::USES_INDEX, ph, RouterDefaults::USE_INDEX};
    Variable<Facet, FlushMode> flush_mode{Keys::FLUSH_MODE, ph, RouterDefaults::FLUSH_MODE};

    Variable<Facet, bool> enabled{param::properties::enabled, ph, true};

    RouterNode<T> router_node;

private:
    static Vec<Node<T>*> cast_inputs(const Vec<std::unique_ptr<SequenceType>>& uptrs) {
        auto raw_ptrs = Vec<Node<T>*>::allocated(uptrs.size());
        for (const auto& uptr : uptrs) {
            raw_ptrs.append(uptr.get());
        }
        return raw_ptrs;
    }

};


using RouterFacetWrapper = RouterWrapper<Facet>;
using RouterPulseWrapper = RouterWrapper<Trigger>;

}

#endif //SERIALIST_ROUTER_H
