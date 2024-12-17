#ifndef MATCHERS_COMMON_H
#define MATCHERS_COMMON_H
#include <functional>

#include <core/algo/facet.h>
#include <core/collections/voices.h>

#include "condition.h"
#include "results.h"


namespace serialist::test {

// template<typename T>
// bool history_is_empty(const Vec<StepResult<T> >& results, bool includes_last_step) {
//     // Note: RunResult specifies that it will always contain at least one value (last output),
//     //       which may or may not be included here, depending on include_last_step value
//     if (includes_last_step) {
//         return results.size() <= 1;
//     }
//     return results.empty();
// }


// // ==============================================================================================
// // FUNCTION ALIASES
// // ==============================================================================================
//
// template<typename T>
// using ValueCondition = std::function<bool(const T&)>;
//
// template<typename T>
// using SizeCondition = std::function<bool(const Voices<T>&)>;
//
// template<typename T>
// using ConsecutiveCompare = std::function<bool(const T&, const T&)>;


// ==============================================================================================
// MATCHER BASE CLASS
// ==============================================================================================

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
            if (m_match_type == MatchType::last) {
                return match_last(arg);
            } else if (m_match_type == MatchType::all) {
                return match_all(arg);
            } else if (m_match_type == MatchType::any) {
                return match_any(arg);
            } else {
                throw test_error("Unknown match type");
            }
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
        //    could be used with a 'not'-condition, in which case outputting true would fail the matcher)
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
//
// template<typename T>
// class RunResultMatcher2 : public Catch::Matchers::MatcherBase<RunResult<T> > {
// public:
//     virtual std::string public_description() const = 0;
//     virtual bool match_internal(const RunResult<T>& success_r) const = 0;
//
//
//     bool match(const RunResult<T>& arg) const final {
//         if (!arg.is_successful()) {
//             return fail_assertion(arg.to_string());
//         }
//
//         return match_internal(arg);
//     }
//
// protected:
//     std::string describe() const final {
//         if (m_failure_string) {
//             return *m_failure_string;
//         }
//
//         return public_description();
//     }
//
//
//     /** @note Utility function for failure messages on assertions, i.e. failed configurations */
//     bool fail_assertion(std::string failure_string) const {
//         m_failure_string = std::move(failure_string);
//         return false;
//     }
//
//
//     /** @note Utility function for failure messages related to unsuccessful RunResult<T> */
//     bool fail_run(std::string failure_string) const {
//         m_failure_string = std::move(failure_string);
//         return false;
//     }
//
// private:
//     mutable std::optional<std::string> m_failure_string;
// };


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// template<typename T>
// class EmptyMatcher : public RunResultMatcher<T> {
// public:
//     bool match_internal(const RunResult<T>& success_r) const override {
//         m_evaluated_step = success_r.last();
//         return is_empty(success_r.last().voices());
//     }
//
//
//     std::string public_description() const override {
//         return "expected empty-like, actual: " + m_evaluated_step->to_string();
//     }
//
// private:
//     mutable std::optional<StepResult<T> > m_evaluated_step = std::nullopt;
// };
//
//
// // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// template<typename RunResultMatcherType, typename ValueType>
// class AllHistoryMatcher : public RunResultMatcher<ValueType> {
// public:
//     explicit AllHistoryMatcher(std::unique_ptr<RunResultMatcherType> internal_matcher
//                                , bool check_last_step = true
//                                , bool allow_no_history = false)
//         : m_internal_matcher(std::move(internal_matcher))
//           , m_check_last_step(check_last_step)
//           , m_allow_no_history(allow_no_history) {
//         static_assert(std::is_base_of_v<RunResultMatcher<ValueType>, RunResultMatcherType>);
//         assert(m_internal_matcher);
//     }
//
//
//     bool match_internal(const RunResult<ValueType>& success_r) const override {
//         const Vec<StepResult<ValueType> >& results =
//                 m_check_last_step ? success_r.entire_output() : success_r.history();
//
//         if (history_is_empty(results, m_check_last_step) && !m_allow_no_history) {
//             return RunResultMatcher<ValueType>::fail_assertion("Matcher expected history, but it was empty");
//         }
//
//         for (const auto& step: results) {
//             if (!m_internal_matcher->match(RunResult<ValueType>::dummy(step))) {
//                 return false;
//             }
//         }
//         return true;
//     }
//
//
//     std::string public_description() const override {
//         return m_internal_matcher->public_description();
//     }
//
// private:
//     std::unique_ptr<RunResultMatcherType> m_internal_matcher;
//     bool m_check_last_step;
//     bool m_allow_no_history;
// };
//
//
// // ==============================================================================================
// // COMMON FACTORY FUNCTIONS
// // ==============================================================================================
//
// template<typename T>
// EmptyMatcher<T> empty() {
//     return EmptyMatcher<T>();
// }
//
//
// inline EmptyMatcher<Facet> emptyf() {
//     return empty<Facet>();
// }
//
//
// template<typename T>
// AllHistoryMatcher<EmptyMatcher<T>, T> all_empty(bool check_output_step = false, bool allow_no_history = false) {
//     return AllHistoryMatcher<EmptyMatcher<T>, T>(std::make_unique<EmptyMatcher<T> >()
//                                                  , check_output_step
//                                                  , allow_no_history);
// }
//
//
// inline AllHistoryMatcher<EmptyMatcher<Facet>, Facet> all_emptyf(bool check_output_step = false
//                                                                 , bool allow_no_history = false) {
//     return all_empty<Facet>(check_output_step, allow_no_history);
// }
}

#endif //MATCHERS_COMMON_H
