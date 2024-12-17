#ifndef MATCHERS_COMMON_H
#define MATCHERS_COMMON_H

#include <core/algo/facet.h>
#include <core/collections/voices.h>

#include "condition.h"
#include "results.h"


namespace serialist::test {


enum class MatchType {
    last, all, any
};


template<typename T>
class ResultMatcher : public Catch::Matchers::MatcherBase<RunResult<T> > {
public:
    ResultMatcher(std::unique_ptr<GenericCondition<T> > condition
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
            m_failed_step_info = "Failed: Not enough values to compare, size=" + std::to_string(arg.entire_output().size());
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


    std::unique_ptr<GenericCondition<T> > m_condition;
    std::string m_matcher_description;
    MatchType m_match_type;
    bool m_allow_no_comparison;

    mutable bool m_skip_matcher_description = false;
    mutable std::optional<std::string> m_failed_step_info = std::nullopt;
};

}

#endif //MATCHERS_COMMON_H
