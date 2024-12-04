
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


}

#endif //MATCHERS_COMMON_H
