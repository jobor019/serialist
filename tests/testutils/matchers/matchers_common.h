#ifndef MATCHERS_COMMON_H
#define MATCHERS_COMMON_H
#include <functional>

#include <core/algo/facet.h>
#include <core/collections/voices.h>
#include "results.h"


namespace serialist::test {


// ==============================================================================================
// CONSTANTS
// ==============================================================================================


static constexpr double EPSILON = 1e-8;
static constexpr double EPSILON2 = 1e-15;


// ==============================================================================================
// SIZE CHECKS
// ==============================================================================================

template<typename T>
bool is_empty(const Voices<T>& v) { return v.is_empty_like(); }


template<typename T>
bool is_singular(const Voices<T>& v) { return v.size() == 1 && v[0].size() == 1; }


// Note: "maybe" indicate that the value is either singular or empty
template<typename T>
bool is_maybe_singular(const Voices<T>& v) { return is_empty(v) || is_singular(v); }

template<typename T>
bool history_is_empty(const Vec<StepResult<T>>& results, bool includes_last_step) {
    // Note: RunResult specifies that it will always contain at least one value (last output),
    //       which may or may not be included here, depending on include_last_step value
    if (includes_last_step) {
        return results.size() <= 1;
    }
    return results.empty();
}


// ==============================================================================================
// FUNCTION ALIASES
// ==============================================================================================

template<typename T>
using ValueCondition = std::function<bool(const T&)>;

template<typename T>
using SizeCondition = std::function<bool(const Voices<T>&)>;

template<typename T>
using ConsecutiveCompare = std::function<bool(const T&, const T&)>;


// ==============================================================================================
// MATCHER BASE CLASSES
// ==============================================================================================

template<typename T>
class RunResultMatcher : public Catch::Matchers::MatcherBase<RunResult<T> > {
public:
    virtual std::string public_description() const = 0;
    virtual bool match_internal(const RunResult<T>& success_r) const = 0;


    bool match(const RunResult<T>& arg) const final {
        if (!arg.is_successful()) {
            return fail_assertion(arg.to_string());
        }

        return match_internal(arg);
    }

protected:
    std::string describe() const final {
        if (m_failure_string) {
            return *m_failure_string;
        }

        return public_description();
    }

    /** @note Utility function for failure messages on assertions, i.e. failed configurations */
    bool fail_assertion(std::string failure_string) const {
        m_failure_string = std::move(failure_string);
        return false;
    }

    /** @note Utility function for failure messages related to unsuccessful RunResult<T> */
    bool fail_run(std::string failure_string) const {
        m_failure_string = std::move(failure_string);
        return false;
    }



private:
    mutable std::optional<std::string> m_failure_string;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
class EmptyMatcher : public RunResultMatcher<T> {
public:
    bool match_internal(const RunResult<T>& success_r) const override {
        m_evaluated_step = success_r.last();
        return is_empty(success_r.last().voices());
    }


    std::string public_description() const override {
        return "expected empty-like, actual: " + m_evaluated_step->to_string();
    }

private:
    mutable std::optional<StepResult<T> > m_evaluated_step = std::nullopt;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename RunResultMatcherType, typename ValueType>
class AllHistoryMatcher : public RunResultMatcher<ValueType> {
public:
    explicit AllHistoryMatcher(std::unique_ptr<RunResultMatcherType> internal_matcher
                               , bool check_last_step = true
                               , bool allow_no_history = false)
        : m_internal_matcher(std::move(internal_matcher))
          , m_check_last_step(check_last_step)
          , m_allow_no_history(allow_no_history) {
        static_assert(std::is_base_of_v<RunResultMatcher<ValueType>, RunResultMatcherType>);
        assert(m_internal_matcher);
    }


    bool match_internal(const RunResult<ValueType>& success_r) const override {
        const Vec<StepResult<ValueType>>& results = m_check_last_step ? success_r.entire_output() : success_r.history();

        if (history_is_empty(results, m_check_last_step) && !m_allow_no_history) {
            return RunResultMatcher<ValueType>::fail_assertion("Matcher expected history, but it was empty");
        }

        for (const auto& step: results) {
            if (!m_internal_matcher->match(RunResult<ValueType>::dummy(step))) {
                return false;
            }
        }
        return true;
    }


    std::string public_description() const override {
        return m_internal_matcher->public_description();
    }

private:
    std::unique_ptr<RunResultMatcherType> m_internal_matcher;
    bool m_check_last_step;
    bool m_allow_no_history;
};


// ==============================================================================================
// COMMON FACTORY FUNCTIONS
// ==============================================================================================

template<typename T>
EmptyMatcher<T> empty() {
    return EmptyMatcher<T>();
}


inline EmptyMatcher<Facet> emptyf() {
    return empty<Facet>();
}


template<typename T>
AllHistoryMatcher<EmptyMatcher<T>, T> all_empty(bool check_output_step = false, bool allow_no_history = false) {
    return AllHistoryMatcher<EmptyMatcher<T>, T>(std::make_unique<EmptyMatcher<T> >()
                                                 , check_output_step
                                                 , allow_no_history);
}


inline AllHistoryMatcher<EmptyMatcher<Facet>, Facet> all_emptyf(bool check_output_step = false
                                                                , bool allow_no_history = false) {
    return all_empty<Facet>(check_output_step, allow_no_history);
}
}

#endif //MATCHERS_COMMON_H
