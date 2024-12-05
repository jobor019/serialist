#ifndef MATCHERS_COMMON_H
#define MATCHERS_COMMON_H
#include <functional>

#include <core/algo/facet.h>
#include <core/collections/voices.h>
#include "runner_results.h"


namespace serialist::test {
// ==============================================================================================
// CONSTANTS
// ==============================================================================================


static constexpr double EPSILON = 1e-8;


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

protected:
    std::string describe() const final {
        return public_description();
    }
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
class EmptyMatcher : public RunResultMatcher<T> {
public:
    bool match(const RunResult<T>& arg) const override {
        assert(arg.success());

        m_evaluated_step = arg.output();
        return is_empty(arg.output().voices);
    }


    std::string public_description() const override {
        return "expected empty-like, actual: " + m_evaluated_step->to_string_compact();
    }

private:
    mutable std::optional<StepResult<T> > m_evaluated_step = std::nullopt;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename RunResultMatcherType, typename ValueType>
class AllHistoryMatcher : public RunResultMatcher<ValueType> {
public:
    explicit AllHistoryMatcher(std::unique_ptr<RunResultMatcherType> internal_matcher
                               , bool check_output_step = false
                               , bool allow_no_history = false)
        : m_internal_matcher(std::move(internal_matcher))
          , m_check_output_step(check_output_step)
          , m_allow_no_history(allow_no_history) {
        static_assert(std::is_base_of_v<RunResultMatcher<ValueType>, RunResultMatcherType>);
        assert(m_internal_matcher);
    }


    bool match(const RunResult<ValueType>& arg) const override {
        assert(arg.success());
        assert(m_allow_no_history || !arg.history().empty());

        for (const auto& step: arg.history()) {
            if (!m_internal_matcher->match(RunResult<ValueType>::dummy(step))) {
                return false;
            }
        }

        if (m_check_output_step && !m_internal_matcher->match(RunResult<ValueType>::dummy(arg.output()))) {
            return false;
        }
        return true;
    }


    std::string public_description() const override {
        return m_internal_matcher->public_description();
    }

private:
    std::unique_ptr<RunResultMatcherType> m_internal_matcher;
    bool m_check_output_step;
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
