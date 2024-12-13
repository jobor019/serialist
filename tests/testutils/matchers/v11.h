#ifndef V11_H
#define V11_H

#include <catch2/catch_test_macros.hpp>

#include "results.h"
#include "core/collections/voices.h"
#include "matchers_common.h"
#include "param/string_serialization.h"

using namespace serialist;


namespace serialist::test::v11 {
// ==============================================================================================
// Matcher Classes
// ==============================================================================================


template<typename T, bool on_empty = false>
class GenericValueMatcher : public RunResultMatcher<T> {
public:
    explicit GenericValueMatcher(std::string expected_rendering, const ValueCondition<T>& f)
        : m_condition(f), m_expected_rendering(std::move(expected_rendering)) {}


    bool match_internal(const RunResult<T>& success_r) const override {
        m_evaluated_step = success_r.last();


        const auto& v = m_evaluated_step->voices();

        // Note: an empty-like value will always automatically pass or fail without evaluating condition
        if (is_empty(v)) {
            m_size_error = !on_empty; // an empty input is only an error if on_empty is false
            return on_empty;
        }

        if (!is_singular(v)) {
            m_size_error = true;
            return false;
        }

        if (!m_condition(v[0][0])) {
            return false;
        }

        return true;
    }


    std::string public_description() const override {
        if (m_size_error) {
            return "is not a single valued voices (actual: " + m_evaluated_step->to_string_detailed() + ")";
        }

        return "expected: " + m_expected_rendering + ", actual: " + m_evaluated_step->to_string_detailed();
    }

private:
    ValueCondition<T> m_condition;

    std::string m_expected_rendering;
    mutable bool m_size_error = false;
    mutable std::optional<StepResult<T> > m_evaluated_step;
};


template<typename T, typename Comparator>
GenericValueMatcher<T> make_comparator(const std::string& op, Comparator comp, const T& expected) {
    return GenericValueMatcher<T>(op + StringSerializer<T>::to_string(expected),
                                  [=](const T& v) { return comp(v, expected); });
}


// ==============================================================================================
// Factory Functions
// ==============================================================================================


template<typename T>
GenericValueMatcher<T> eq(const T& expected, const std::optional<std::string>& custom_rendering = std::nullopt) {
    return GenericValueMatcher<T>(custom_rendering.value_or(StringSerializer<T>::to_string(expected))
                                  , [expected](const T& v) {
                                      return v == expected;
                                  });
}


template<typename U>
GenericValueMatcher<Facet> eqf(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);

    auto f = static_cast<Facet>(expected);
    return eq<Facet>(f, Voices<Facet>::singular(f).to_string<>());
}


template<typename T>
GenericValueMatcher<T> lt(const T& expected) {
    return make_comparator<T>("< ", std::less<T>(), expected);
}


template<typename T>
GenericValueMatcher<T> le(const T& expected) {
    return make_comparator<T>("<= ", std::less_equal<T>(), expected);
}


template<typename T>
GenericValueMatcher<T> gt(const T& expected) {
    return make_comparator<T>("> ", std::greater<T>(), expected);
}


template<typename T>
GenericValueMatcher<T> ge(const T& expected) {
    return make_comparator<T>(">= ", std::greater_equal<T>(), expected);
}


// Specialized comparison functions for Facet
template<typename U>
GenericValueMatcher<Facet> ltf(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return lt<Facet>(static_cast<Facet>(expected));
}


template<typename U>
GenericValueMatcher<Facet> lef(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return le<Facet>(static_cast<Facet>(expected));
}


template<typename U>
GenericValueMatcher<Facet> gtf(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return gt<Facet>(static_cast<Facet>(expected));
}


template<typename U>
GenericValueMatcher<Facet> gef(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return ge<Facet>(static_cast<Facet>(expected));
}


/** Note: for float Facet comparison, use eqf instead. approx_eqf is only for cases where a custom epsilon is needed */
template<typename U>
GenericValueMatcher<Facet> approx_eqf(const U& expected, double epsilon, double epsilons_epsilon = EPSILON) {
    assert(epsilon > 0.0);
    assert(epsilons_epsilon >= 0.0);
    static_assert(utils::is_static_castable_v<U, Facet>);
    std::stringstream ss;
    ss << StringSerializer<U>::to_string(expected) << "±";
    if (epsilon < 1e-3) {
        ss << std::scientific << std::setprecision(0);
    }
    ss << epsilon;

    return GenericValueMatcher<Facet>(ss.str(), [=](const Facet& v) {
        return utils::equals(static_cast<double>(v), static_cast<double>(expected), epsilon + epsilons_epsilon);
    });
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
GenericValueMatcher<T> in_range(const T& low, const T& high, bool end_inclusive = false, bool start_inclusive = true) {
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

    return GenericValueMatcher<T>("in range", [=](const T& v) {
        return utils::in<T>(v, low, high, start_inclusive, end_inclusive);
    });
}


template<typename U>
GenericValueMatcher<Facet> in_rangef(const U& low, const U& high, bool end_inclusive = false,
                                     bool start_inclusive = true) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return in_range<Facet>(static_cast<Facet>(low), static_cast<Facet>(high), end_inclusive, start_inclusive);
}


inline GenericValueMatcher<Facet> in_unit_rangef() {
    return in_rangef(0.0, 1.0, false, true);
}


template<typename U>
GenericValueMatcher<Facet> approx_in_rangef(const U& low, const U& high
                                            , double epsilon = EPSILON
                                            , bool end_inclusive = false, bool start_inclusive = true) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return in_range<Facet>(static_cast<Facet>(low - epsilon)
                           , static_cast<Facet>(high + epsilon)
                           , end_inclusive, start_inclusive);
}
}


// ==============================================================================================

namespace serialist::test::v11h {
// template<typename T>
// class AllHistoryMatcher : public RunResultMatcher<T> {
// public:
//     explicit AllHistoryMatcher(std::unique_ptr<v11::GenericValueMatcher<T>> internal_matcher)
//         : m_internal_matcher(std::move(internal_matcher)) {
//         assert(m_internal_matcher);
//     }
//
//
//     bool match(const RunResult<T>& arg) const override {
//         assert(arg.success());
//         assert(!arg.history().empty()); // Likely an error by the caller
//
//         for (const auto& step : arg.history()) {
//             if (!m_internal_matcher->match(RunResult<T>::dummy(step))) {
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
//
// private:
//     std::unique_ptr<v11::GenericValueMatcher<T>> m_internal_matcher;
// };

// TODO: This should be updated to closer match the signature of AllHistoryMatcher (allow_no_history, check_last_step, etc.)
template<typename T>
class ComparePrevious : public RunResultMatcher<T> {
public:
    explicit ComparePrevious(std::string keyword
                             , const ConsecutiveCompare<T>& f
                             , bool check_last_step = false
                             , bool allow_no_history = false)
        : m_func(f)
          , m_keyword(std::move(keyword))
          , m_check_last_step(check_last_step)
          , m_allow_no_history(allow_no_history) {}


    bool match_internal(const RunResult<T>& success_r) const override {
        const Vec<StepResult<T>>& results = m_check_last_step ? success_r.entire_output() : success_r.history();

        if (history_is_empty(results, m_check_last_step)) {
            return RunResultMatcher<T>::fail_assertion("Matcher expected history, but it was empty");
        }

        auto& fst = results[0].voices();
        if (fst.size() != 1 || fst[0].size() != 1) {
            m_failure_step = results[0];
            m_size_error = true;
            return false;
        }

        for (int i = 1; i < results.size(); ++i) {
            auto& v = results[i].voices();
            if (v.size() != 1 || v[0].size() != 1) {
                m_size_error = true;
                m_failure_step = results[i];
                return false;
            }

            if (!m_func(results[i - 1].voices()[0][0], results[i].voices()[0][0])) {
                m_failure_step = results[i];
                m_prev_step = results[i - 1];
                return false;
            }
        }
        return true;
    }


    std::string public_description() const override {
        if (!m_failure_step && !m_prev_step) {
            return "history is empty";
        }

        if (m_size_error) {
            return "is not a single valued voices (actual: " + m_failure_step->to_string() + ")";
        }

        if (!m_prev_step) {
            return "is not " + m_keyword + " previous. failure: " + m_failure_step->to_string();
        }

        return "is not " + m_keyword + " previous. cur: " + m_failure_step->to_string()
               + ", prev: " + m_prev_step->to_string();
    }

private:
    ConsecutiveCompare<T> m_func;
    std::string m_keyword;

    bool m_check_last_step;
    bool m_allow_no_history;

    mutable bool m_size_error = false;
    mutable std::optional<StepResult<T> > m_failure_step;
    mutable std::optional<StepResult<T> > m_prev_step;
};


template<typename T>
ComparePrevious<T> strictly_increasing() {
    return ComparePrevious<T>("greater than", [](const T& a, const T& b) {
        return b > a;
    });
}


inline ComparePrevious<Facet> strictly_increasingf() { return strictly_increasing<Facet>(); }


template<typename T>
ComparePrevious<T> strictly_decreasing() {
    return ComparePrevious<T>("less than", [](const T& a, const T& b) {
        return b < a;
    });
}


inline ComparePrevious<Facet> strictly_decreasingf() { return strictly_decreasing<Facet>(); }


template<typename T>
AllHistoryMatcher<v11::GenericValueMatcher<T>, T> all(v11::GenericValueMatcher<T>&& matcher
                                                      , bool check_last_step = true
                                                      , bool allow_no_history = false) {
    return AllHistoryMatcher<v11::GenericValueMatcher<T>, T>(
        std::make_unique<v11::GenericValueMatcher<T> >(std::move(matcher))
        , check_last_step
        , allow_no_history
    );
}


inline AllHistoryMatcher<v11::GenericValueMatcher<Facet>, Facet> allf(v11::GenericValueMatcher<Facet>&& matcher
                                                                      , bool check_last_step = true
                                                                      , bool allow_no_history = false) {
    return all<Facet>(std::move(matcher), check_last_step, allow_no_history);
}
}


#endif //V11_H
