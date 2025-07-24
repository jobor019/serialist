#ifndef TESTUTILS_RESULTS_H
#define TESTUTILS_RESULTS_H

#include <variant>
#include <core/policies/policies.h>
#include "core/types/facet.h"
#include <core/generative.h>
#include <core/collections/vec.h>
#include <catch2/catch_test_macros.hpp>
#include "exceptions.h"
#include "core/types/trigger.h"


namespace serialist::test {
template<typename T> class StepResult {
public:
    static constexpr std::size_t NUM_DECIMALS_DETAILED = 16;


    static StepResult success(const Voices<T>& voices
                              , const TimePoint& time
                              , std::size_t step_index
                              , DomainType primary_domain) {
        return StepResult(voices, time, step_index, primary_domain);
    }


    static StepResult failure(const std::string& err
                              , const TimePoint& time
                              , std::size_t step_index
                              , DomainType primary_domain) {
        return StepResult(err, time, step_index, primary_domain);
    }


    StepResult edited(const Voices<T>& new_voices
                              , const TimePoint& new_time
                              , std::size_t new_step_index) const {
        if (!is_successful()) {
            throw test_error("Cannot edit a failed StepResult: " + to_string());
        }

        return StepResult(new_voices, new_time, new_step_index, m_primary_domain, voices());
    }


    StepResult merged(const StepResult& new_result) const {
        if (!is_successful()) {
            throw test_error("Cannot merge a failed StepResult. lhs:" + to_string());
        }

        if (!new_result.is_successful()) {
            throw test_error("Cannot merge a failed StepResult. rhs:" + new_result.to_string());
        }

        auto merged_voices = voices().cloned().merge(new_result.voices());
        auto new_time = new_result.time();
        auto new_step_index = step_index() + 1;
        return edited(merged_voices, new_time, new_step_index);
    }


    friend std::ostream& operator<<(std::ostream& os, const StepResult& obj) {
        os << obj.to_string();
        return os;
    }


    void print(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::cout << to_string(compact, num_decimals) << "\n";
    }


    void print_detailed(std::size_t num_decimals = NUM_DECIMALS_DETAILED) const {
        print(false, num_decimals);
    }


    std::string to_string(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        if (is_successful()) {
            return std::string{"StepResult"}
                   + (edited() ? "[edited]" : "")
                   + "(v=" + render_output(compact, num_decimals)
                   + ", " + render_step_info( compact, num_decimals) + ")";
        } else {
            return render_error() + " " + render_step_info(compact, num_decimals);
        }
    }


    std::string to_string_detailed(std::size_t num_decimals = NUM_DECIMALS_DETAILED) const {
        return to_string(false, num_decimals);
    }


    const Voices<T>& voices() const {
        if (is_successful()) {
            return std::get<Voices<T>>(m_value);
        } else {
            throw test_error(render_error());
        }
    }


    std::optional<T> v11() const {
        if (is_successful()) {
            return voices().first();
        }
        return std::nullopt;
    }


    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Facet>>>
    std::optional<double> v11f() const {
        if (is_successful()) {
            if (auto v = voices().first()) {
                return static_cast<double>(*v);
            }
        }
        return std::nullopt;
    }


    /** Returns the id of the last trigger in the first voice, if any */
    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Trigger>>>
    std::optional<std::size_t> trigger_id() const {
        if (auto t = v11()) {
            return t->get_id();
        }
        return std::nullopt;
    }


    /** Returns the id of the last trigger of the given type in the first voice, if any */
    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Trigger>>>
    std::optional<std::size_t> trigger_id_of_type(Trigger::Type type) const {
        if (is_successful()) {
            const Trigger* last_trigger_of_type = nullptr;
            auto first_voice = voices()[0];
            for (const Trigger& trigger : first_voice) {
                if (trigger.is(type)) {
                    last_trigger_of_type = &trigger;
                }
            }
            if (last_trigger_of_type) {
                return last_trigger_of_type->get_id();
            }
        }
        return std::nullopt;
    }


    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Trigger>>>
    std::optional<std::size_t> pulse_on_id() const {
        return trigger_id_of_type(Trigger::Type::pulse_on);
    }


    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Trigger>>>
    std::optional<std::size_t> pulse_off_id() const {
        return trigger_id_of_type(Trigger::Type::pulse_off);
    }


    bool is_successful() const { return std::holds_alternative<Voices<T>>(m_value); }
    const TimePoint& time() const { return m_time; }
    std::size_t step_index() const { return m_step_index; }

    bool edited() const { return static_cast<bool>(m_value_before_edit); }
    std::optional<Voices<T>> before_edit() const { return m_value_before_edit; }


private:
    StepResult(const std::variant<Voices<T>, std::string>& value
               , const TimePoint& time
               , std::size_t step_index
               , DomainType primary_domain
               , std::optional<Voices<T>> value_before_edit = std::nullopt)
        : m_value(value)
        , m_time(time)
        , m_step_index(step_index)
        , m_primary_domain(primary_domain)
        , m_value_before_edit(value_before_edit) {}


    std::string render_output(bool compact, std::optional<std::size_t> num_decimals = std::nullopt) const {
        const auto& v = voices();
        std::size_t num_elements = v.numel();
        if (!compact || num_elements < 10) {
            return v.to_string(num_decimals);
        } else {
            return "Voices(size="
                   + std::to_string(v.size())
                   + ", numel="
                   + std::to_string(num_elements) + ")";
        }
    }


    std::string render_error() const {
        return std::get<std::string>(m_value);
    }


    std::string render_step_info(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::stringstream ss;

        ss << " i=" << m_step_index << ", t=";
        if (compact) {
            ss << DomainTimePoint::from_time_point(m_time, m_primary_domain).to_string_compact();
        } else {
            ss << DomainTimePoint::from_time_point(m_time, m_primary_domain).to_string_compact(num_decimals);
        }
        return ss.str();
    }


    std::variant<Voices<T>, std::string> m_value; // Voices<T> if successful, otherwise error description
    TimePoint m_time;
    std::size_t m_step_index;
    DomainType m_primary_domain;

    std::optional<Voices<T>> m_value_before_edit = std::nullopt;
};


// ==============================================================================================

template<typename T> class RunResult {
public:
    RunResult(const Vec<StepResult<T>>& run_output
              , DomainType primary_domain)
        : m_run_output(run_output)
        , m_primary_domain(primary_domain) {
        // run_output should always contain at least a single StepResult<T>::failure or a single StepResult<T>::success
        if (run_output.empty()) throw test_error("Empty run output");
    }


    static RunResult failure(const std::string& err
                             , const TimePoint& t = TimePoint{}
                             , std::size_t step_index = 0
                             , DomainType primary_domain = DomainType::ticks
                             , const Vec<StepResult<T>>& output_history = {}) {
        auto run_output = output_history.cloned();
        run_output.append(StepResult<T>::failure(err, t, step_index, primary_domain));
        return {run_output, primary_domain};
    }


    static RunResult dummy(const T& v) { return dummy(Voices<T>::singular(v)); }
    static RunResult dummy(const StepResult<T>& s) { return RunResult({s}, DomainType::ticks); }


    static RunResult dummy(const Voices<T>& v) {
        return dummy(StepResult<T>::success(v, TimePoint{}, 0, DomainType::ticks));
    }


    static RunResult dummy(const Vec<T>& vs, bool transpose = true) {
        if (transpose) {
            auto h = Vec<StepResult<T>>::allocated(vs.size());
            for (std::size_t i = 0; i < vs.size(); ++i) {
                h.append(StepResult<T>::success(Voices<T>::singular(vs[i]), TimePoint{}, i, DomainType::ticks));
            }
            return RunResult(h, DomainType::ticks);
        } else {
            auto voices = Voices<T>{vs};
            return RunResult({StepResult<T>::success(voices, TimePoint{}, 0, DomainType::ticks)}, DomainType::ticks);
        }
    }

    RunResult merged(const StepResult<T>& result_to_merge) const {
        if (!is_successful()) {
            throw test_error("Cannot merge failed RunResult. lhs: " + to_string());
        }

        auto run_output = history();
        auto lhs_to_merge = last();

        run_output.append(lhs_to_merge.merged(result_to_merge));
        return RunResult(run_output, m_primary_domain);
    }


    friend std::ostream& operator<<(std::ostream& os, const RunResult&) {
        // Note: we don't want to allow RunResult to be printed directly,
        //       as this will lead to incredibly confusing results from Catch2 when evaluated over MatchType::any/all
        return os;
    }


    RunResult subset(std::size_t start, std::size_t end) const {
        return RunResult(m_run_output.slice(start, end), m_primary_domain);
    }


    RunResult history_subset() const { return RunResult(history(), m_primary_domain); }
    RunResult last_subset() const { return RunResult({last()}, m_primary_domain); }
    RunResult first_subset() const { return RunResult({first()}, m_primary_domain); }
    auto unpack() const { return std::make_tuple(history_subset(), last_subset()); }

    Vec<StepResult<T>> history() const { return m_run_output.slice(0, -1); }

    StepResult<T> first() const { return *m_run_output.first(); }
    StepResult<T> last() const { return *m_run_output.last(); }


    std::optional<Voices<T>> voices() const {
        if (is_successful()) {
            return last().voices();
        }
        return std::nullopt;
    }

    std::optional<T> v11() const {
        if (is_successful()) {
            return last().voices().first();
        }
        return std::nullopt;
    }


    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Facet>>> std::optional<double> v11f() const {
        if (is_successful()) {
            if (auto v = last().voices().first()) {
                return static_cast<double>(*v);
            }
        }
        return std::nullopt;
    }


    /** Returns the id of the last trigger in the first voice, if any */
    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Trigger>>>
    std::optional<std::size_t> trigger_id() const {
        if (is_successful()) {
            return last().trigger_id();
        }
        return std::nullopt;
    }

    /** Returns the id of the last pulse_on in the first voice, if any */
    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Trigger>>>
    std::optional<std::size_t> pulse_on_id() const {
        if (is_successful()) {
            return last().pulse_on_id();
        }
        return std::nullopt;
    }


    /** Returns the id of the last pulse_off in the first voice, if any */
    template<typename U = T, typename = std::enable_if_t<std::is_same_v<U, Trigger>>>
    std::optional<std::size_t> pulse_off_id() const {
        if (is_successful()) {
            return last().pulse_off_id();
        }
        return std::nullopt;
    }


    const Vec<StepResult<T>>& entire_output() const { return m_run_output; }
    bool is_successful() const { return last().is_successful(); }
    std::size_t num_steps() const { return m_run_output.size(); }
    TimePoint time() const { return last().time(); }
    std::size_t step_index() const { return last().step_index(); }


    void print() const { std::cout << to_string() << "\n"; }


    void print_detailed(std::size_t num_decimals = StepResult<T>::NUM_DECIMALS_DETAILED) const {
        std::cout << to_string_detailed(num_decimals) << "\n";
    }


    void print_all(std::optional<std::size_t> num_decimals = std::nullopt) const {
        for (auto& r : m_run_output) {
            r.print(false, num_decimals);
        }
    }


    std::string to_string(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        return last().to_string(compact, num_decimals);
    }


    std::string to_string_detailed(std::size_t num_decimals = StepResult<T>::NUM_DECIMALS_DETAILED) const {
        return to_string(false, num_decimals);
    }

private:
    Vec<StepResult<T>> m_run_output; // Invariant: m_run_output.size() > 0

    DomainType m_primary_domain;
};
}

#endif // TESTUTILS_RESULTS_H
