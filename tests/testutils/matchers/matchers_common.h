#ifndef TESTUTILS_MATCHERS_COMMON_H
#define TESTUTILS_MATCHERS_COMMON_H

#include "core/types/facet.h"
#include <core/collections/voices.h>

#include "condition.h"
#include "event.h"
#include "results.h"


namespace serialist::test {
enum class MatchType {
    last, all, any
};


template<typename T>
class ResultMatcher : public Catch::Matchers::MatcherBase<RunResult<T>> {
public:
    ResultMatcher(std::unique_ptr<GenericCondition<T>> condition
                  , std::string matcher_description
                  , MatchType match_type = MatchType::last
                  , bool allow_no_comparison = false)
        : m_condition(std::move(condition))
        , m_matcher_description(std::move(matcher_description))
        , m_match_type(match_type)
        , m_allow_no_comparison(allow_no_comparison) {
        assert(m_condition);
    }


    bool match(const RunResult<T>& arg) const override {
        if (!arg.is_successful()) {
            m_failed_step_info = "Failed run: " + arg.to_string();
            m_skip_matcher_description = true;
            return false;
        }

        try {
            if (m_match_type == MatchType::last) { return match_last(arg); }
            if (m_match_type == MatchType::all) { return match_all(arg); }
            if (m_match_type == MatchType::any) { return match_any(arg); }
            throw test_error("Unknown match type");
        } catch (const test_error& e) {
            m_failed_step_info = e.what();
            return false;
        }
    }

protected:
    std::string describe() const override {
        if (m_skip_matcher_description) {
            return *m_failed_step_info;
        }

        return m_matcher_description + ". Actual: " + *m_failed_step_info;
    }

private:
    /** @throws test_error if comparison fails */
    bool match_last(const RunResult<T>& arg) const {
        auto rc = m_condition->matches_last(arg.entire_output());

        if (!rc.has_value() && !m_allow_no_comparison) {
            // No comparison was made (this should typically only happen for comparisons with previous value,
            //   which would require a RunResult of size 2. An empty RunResult would be ill-defined)
            m_failed_step_info = "Failed: Not enough values to compare, size=" + std::to_string(
                                     arg.entire_output().size());
            m_skip_matcher_description = true;
            return false;
        } else if (!*rc) {
            // Comparison was made and yielded false
            m_failed_step_info = format_failure(arg.num_steps() - 1, arg);
            return false;
        }
        // Comparison was made and yielded true. Note that we're still setting m_failed_step_info as the matched
        //    could be used with a 'not'-condition, in which case outputting true would fail the matcher
        m_failed_step_info = format_failure(arg.num_steps() - 1, arg);
        return true;
    }


    /** @throws test_error if comparison fails */
    bool match_all(const RunResult<T>& arg) const {
        auto failing_index = m_condition->matches_all(arg.entire_output(), m_allow_no_comparison);

        if (failing_index.has_value()) {
            m_failed_step_info = format_failure(*failing_index, arg);
            return false;
        }

        // Needed for 'not'-matcher
        m_failed_step_info = "all steps match condition";
        return true;
    }


    /** @throws test_error if comparison fails */
    bool match_any(const RunResult<T>& arg) const {
        auto succeeding_index = m_condition->matches_any(arg.entire_output(), m_allow_no_comparison);

        if (succeeding_index.has_value()) {
            // Needed for 'not'-matcher
            m_failed_step_info = format_failure(*succeeding_index, arg);
            return true;
        }

        m_failed_step_info = "no steps match condition";
        return false;
    }


    std::string format_failure(std::size_t step_index, const RunResult<T>& arg) const {
        auto offsets = m_condition->compared_step_offsets();
        if (offsets.empty()) {
            return arg.entire_output()[step_index].to_string();
        }

        int base_index = static_cast<int>(step_index);

        std::stringstream ss;
        for (std::size_t i = 0; i < offsets.size(); ++i) {
            std::size_t index = static_cast<std::size_t>(base_index + offsets[i]);
            ss << arg.entire_output()[index].to_string();
            if (i < offsets.size() - 1) ss << ", ";
        }
        return ss.str();
    }


    std::unique_ptr<GenericCondition<T>> m_condition;
    std::string m_matcher_description;
    MatchType m_match_type;
    bool m_allow_no_comparison;

    mutable bool m_skip_matcher_description = false;
    mutable std::optional<std::string> m_failed_step_info = std::nullopt;
};


// ==============================================================================================

class TimePointMatcher : public Catch::Matchers::MatcherBase<TimePoint> {
public:
    explicit TimePointMatcher(const std::optional<double>& tick = std::nullopt
                              , const std::optional<double>& absolute_beat = std::nullopt
                              , const std::optional<double>& relative_beat = std::nullopt
                              , const std::optional<double>& bar = std::nullopt
                              , const std::optional<Meter>& meter = std::nullopt
                              , const std::optional<double>& tempo = std::nullopt
                              , double epsilon = EPSILON)
        : m_tick(tick)
        , m_tempo(tempo)
        , m_absolute_beat(absolute_beat)
        , m_relative_beat(relative_beat)
        , m_bar(bar)
        , m_meter(meter)
        , m_epsilon(epsilon) {}


    explicit TimePointMatcher(const TimePoint& t, bool match_meter = false, bool match_tempo = false)
        : TimePointMatcher(t.get_tick(), t.get_absolute_beat(), t.get_relative_beat(), t.get_bar()
                           , match_meter ? std::optional(t.get_meter()) : std::nullopt
                           , match_tempo ? std::optional(t.get_tempo()) : std::nullopt) {}


    static TimePointMatcher zero(bool match_meter = false, bool match_tempo = false) {
        return TimePointMatcher(TimePoint::zero(), match_meter, match_tempo);
    }


    TimePointMatcher& with_tick(double tick) {
        m_tick = tick;
        return *this;
    }


    TimePointMatcher& with_tempo(double tempo) {
        m_tempo = tempo;
        return *this;
    }


    TimePointMatcher& with_absolute_beat(double absolute_beat) {
        m_absolute_beat = absolute_beat;
        return *this;
    }


    TimePointMatcher& with_relative_beat(double relative_beat) {
        m_relative_beat = relative_beat;
        return *this;
    }


    TimePointMatcher& with_bar(double bar) {
        m_bar = bar;
        return *this;
    }


    TimePointMatcher& with_meter(const Meter& meter) {
        m_meter = meter;
        return *this;
    }


    TimePointMatcher& with_epsilon(double epsilon) {
        m_epsilon = epsilon;
        return *this;
    }


    TimePointMatcher& with(const DomainType& type, double value) {
        switch (type) {
            case DomainType::ticks: return with_tick(value);
            case DomainType::beats: return with_absolute_beat(value);
            case DomainType::bars: return with_bar(value);
            default: throw test_error("unsupported domain type");
        }
    }


    bool match(const TimePoint& arg) const override {
        std::stringstream ss;

        bool matches = true;

        matches = matches_internal(ss, "tick", m_tick, arg.get_tick(), true) && matches;
        matches = matches_internal(ss, "tempo", m_tempo, arg.get_tempo(), matches) && matches;
        matches = matches_internal(ss, "absolute_beat", m_absolute_beat, arg.get_absolute_beat(), matches) && matches;
        matches = matches_internal(ss, "relative_beat", m_relative_beat, arg.get_relative_beat(), matches) && matches;
        matches = matches_internal(ss, "bar", m_bar, arg.get_bar(), matches) && matches;

        if (m_meter && *m_meter != arg.get_meter()) {
            if (!matches) ss << ", ";
            ss << "meter: " << *m_meter << " != " << arg.get_meter();
            matches = false;
        }

        if (matches) {
            m_description = "TimePoint matches all conditions";
        } else {
            m_description = ss.str();
        }

        return matches;
    }

protected:
    std::string describe() const override {
        return m_description;
    }

private:
    bool matches_internal(std::stringstream& ss
                          , const std::string& str
                          , const std::optional<double>& expected
                          , double actual
                          , bool is_first_arg) const {
        if (expected && !utils::equals(*expected, actual, m_epsilon)) {
            if (!is_first_arg) ss << ", ";
            ss << str << ": " << *expected << " (± " << m_epsilon << ") != " << actual;
            return false;
        }
        return true;
    }


    std::optional<double> m_tick;
    std::optional<double> m_tempo;
    std::optional<double> m_absolute_beat;
    std::optional<double> m_relative_beat;
    std::optional<double> m_bar;

    std::optional<Meter> m_meter;

    double m_epsilon;

    mutable std::string m_description = "";
};

}

#endif //TESTUTILS_MATCHERS_COMMON_H
