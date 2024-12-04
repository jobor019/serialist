#ifndef V11_H
#define V11_H

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "runner_results.h"
#include "core/collections/voices.h"
#include "matchers_common.h"
#include "param/string_serialization.h"


namespace serialist::test::v11 {
// ==============================================================================================
// Matcher Classes
// ==============================================================================================


template<typename T, bool on_empty = false>
class GenericMatcher : public RunResultMatcher<T> {
public:
    explicit GenericMatcher(std::string expected_rendering, const ValueCondition<T>& f)
        : m_condition(f), m_expected_rendering(std::move(expected_rendering)) {}


    bool match(const RunResult<T>& arg) const override {
        assert(arg.success());

        m_evaluated_step = arg.output();

        const auto& v = arg.output().voices;

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
            return "is not a single valued voices (actual: " + m_evaluated_step->to_string_compact() + ")";
        }

        return "expected: " + m_expected_rendering + ", actual: " + m_evaluated_step->to_string_compact();
    }

private:
    ValueCondition<T> m_condition;

    std::string m_expected_rendering;
    mutable bool m_size_error = false;
    mutable std::optional<StepResult<T> > m_evaluated_step;
};


template<typename T, typename Comparator>
GenericMatcher<T> make_comparator(const std::string& op, Comparator comp, const T& expected) {
    return GenericMatcher<T>(op + StringSerializer<T>::to_string(expected),
                             [=](const T& v) { return comp(v, expected); });
}


// ==============================================================================================
// Factory Functions
// ==============================================================================================


template<typename T>
GenericMatcher<T> eq(const T& expected, const std::optional<std::string>& custom_rendering = std::nullopt) {
    return GenericMatcher<T>(custom_rendering.value_or(StringSerializer<T>::to_string(expected))
                             , [expected](const T& v) {
                                 return v == expected;
                             });
}


template<typename U>
GenericMatcher<Facet> eqf(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);

    auto f = static_cast<Facet>(expected);
    return eq<Facet>(f, Voices<Facet>::singular(f).to_string<>());
}


template<typename T>
GenericMatcher<T> lt(const T& expected) {
    return make_comparator<T>("< ", std::less<T>(), expected);
}


template<typename T>
GenericMatcher<T> le(const T& expected) {
    return make_comparator<T>("<= ", std::less_equal<T>(), expected);
}


template<typename T>
GenericMatcher<T> gt(const T& expected) {
    return make_comparator<T>("> ", std::greater<T>(), expected);
}


template<typename T>
GenericMatcher<T> ge(const T& expected) {
    return make_comparator<T>(">= ", std::greater_equal<T>(), expected);
}


// Specialized comparison functions for Facet
template<typename U>
GenericMatcher<Facet> ltf(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return lt<Facet>(static_cast<Facet>(expected));
}


template<typename U>
GenericMatcher<Facet> lef(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return le<Facet>(static_cast<Facet>(expected));
}


template<typename U>
GenericMatcher<Facet> gtf(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return gt<Facet>(static_cast<Facet>(expected));
}


template<typename U>
GenericMatcher<Facet> gef(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return ge<Facet>(static_cast<Facet>(expected));
}


/** Note: for float Facet comparison, use eqf instead. approx_eqf is only for cases where a custom epsilon is needed */
template<typename U>
GenericMatcher<Facet> approx_eqf(const U& expected, double epsilon) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    std::stringstream ss;
    ss << StringSerializer<U>::to_string(expected) << "±"
            << std::scientific << std::setprecision(0) << epsilon;

    return GenericMatcher<Facet>(ss.str(), [=](const Facet& v) {
        return utils::equals(static_cast<double>(v), static_cast<double>(expected), epsilon);
    });
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
GenericMatcher<T> in_range(const T& low, const T& high, bool end_inclusive = false, bool start_inclusive = true) {
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

    return GenericMatcher<T>("in range", [=](const T& v) {
        bool lower_check = start_inclusive ? (v >= low) : (v > low);
        bool upper_check = end_inclusive ? (v <= high) : (v < high);
        return lower_check && upper_check;
    });
}


template<typename U>
GenericMatcher<Facet> in_rangef(const U& low, const U& high, bool end_inclusive = false, bool start_inclusive = true) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return in_range<Facet>(static_cast<Facet>(low), static_cast<Facet>(high), end_inclusive, start_inclusive);
}


template<typename U>
GenericMatcher<Facet> approx_in_rangef(const U& low, const U& high
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

template<typename T>
class ComparePrevious : public Catch::Matchers::MatcherBase<RunResult<T> > {
public:
    explicit ComparePrevious(std::string keyword, const ConsecutiveCompare<T>& f)
        : m_func(f), m_keyword(std::move(keyword)) {}


    bool match(const RunResult<T>& arg) const override {
        const auto& history = arg.history();

        if (history.empty()) {
            return false;
        }

        auto& fst = history[0].voices;
        if (fst.size() != 1 || fst[0].size() != 1) {
            m_failure_step = history[0];
            m_size_error = true;
            return false;
        }

        for (int i = 1; i < history.size(); ++i) {
            auto& v = history[i].voices;
            if (v.size() != 1 || v[0].size() != 1) {
                m_size_error = true;
                m_failure_step = history[i];
                return false;
            }

            if (!m_func(history[i - 1].voices[0][0], history[i].voices[0][0])) {
                m_failure_step = history[i];
                m_prev_step = history[i - 1];
                return false;
            }
        }
        return true;
    }

protected:
    std::string describe() const override {
        if (!m_failure_step && !m_prev_step) {
            // This is most likely an error on the users side, which we don't want to accidentally pass
            return "history is empty";
        }

        if (m_size_error) {
            return "is not a single valued voices (actual: " + m_failure_step->to_string_compact() + ")";
        }

        if (!m_prev_step) {
            return "is not " + m_keyword + " previous. failure: " + m_failure_step->to_string_compact();
        }

        return "is not " + m_keyword + " previous. cur: " + m_failure_step->to_string_compact()
               + ", prev: " + m_prev_step->to_string_compact();
    }

private:
    ConsecutiveCompare<T> m_func;
    std::string m_keyword;

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


template<typename T>
ComparePrevious<T> strictly_decreasing() {
    return ComparePrevious<T>("less than", [](const T& a, const T& b) {
        return b < a;
    });
}



}


#endif //V11_H
