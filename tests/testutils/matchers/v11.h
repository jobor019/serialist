#ifndef V11_H
#define V11_H

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "runner_results.h"
#include "core/collections/voices.h"
#include "matchers_common.h"
#include "param/string_serialization.h"


namespace serialist::test::v11 {
template<typename T>
class Empty : public Catch::Matchers::MatcherBase<Voices<T> > {
public:
    bool match(const Voices<T>& arg) const override {
        return arg.is_empty_like();
    }

protected:
    std::string describe() const override {
        return "is not empty";
    }
};


template<typename T>
class GenericMatcher : public Catch::Matchers::MatcherBase<RunResult<T> > {
public:
    explicit GenericMatcher(std::string expected_rendering, const Condition<T>& f)
        : m_condition(f), m_expected_rendering(std::move(expected_rendering)) {}


    bool match(const RunResult<T>& arg) const override {
        assert(arg.success());

        const auto& v = arg.output().voices;
        if (!is_singular(v)) {
            m_size_error = true;
            return false;
        }

        if (!m_condition(v[0][0])) {
            m_failure_step = arg.output();
            return false;
        }

        return true;
    }

protected:
    std::string describe() const override {
        if (m_size_error) {
            return "is not a single valued voices (actual: " + m_failure_step->to_string_compact() + ")";
        }

        return "expected: " + m_expected_rendering + ", actual: " + m_failure_step->to_string_compact();
    }

private:
    Condition<T> m_condition;

    std::string m_expected_rendering;
    mutable bool m_size_error = false;
    mutable std::optional<StepResult<T> > m_failure_step;
};


// ==============================================================================================


template<typename T>
GenericMatcher<T> eq(const T& expected) {
    return GenericMatcher<T>([expected](const T& v) {
        return v == expected;
    });
}


template<typename U>
GenericMatcher<Facet> eqf(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);

    return eq<Facet>(static_cast<Facet>(expected));
}


template<typename U>
GenericMatcher<Facet> approx_eqf(const U& expected, double epsilon = EPSILON) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    std::stringstream ss;
    ss << StringSerializer<U>::to_string(expected) << "±"
       << std::scientific << std::setprecision(0) << epsilon;

    return GenericMatcher<Facet>(ss.str(), [=](const Facet& v) {
        return utils::equals(static_cast<double>(v), static_cast<double>(expected), epsilon);
    });
}


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
class Empty : public Catch::Matchers::MatcherBase<RunResult<T> > {
public:
    bool match(const RunResult<T>& arg) const override {
        for (const auto& r: arg.history()) {
            if (!r.output.is_empty_like()) {
                m_failure_step = r;
                return false;
            }
        }
        return true;
    }

protected:
    std::string describe() const override {
        return "is not empty (failure: " + m_failure_step->to_string_compact() + ")";
    }

private:
    mutable std::optional<StepResult<T> > m_failure_step;
};


template<typename T>
class ComparePrevious : public Catch::Matchers::MatcherBase<RunResult<T> > {
public:
    explicit ComparePrevious(std::string keyword, const CompareFunc<T>& f)
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
            return "is not " + m_keyword + " previous (failure: " + m_failure_step->to_string_compact() + ")";
        }

        return "is not " + m_keyword + " previous (cur: " + m_failure_step->to_string_compact()
               + ", prev: " + m_prev_step->to_string_compact() + ")";
    }

private:
    // static bool validate_v11(const RunResult<T>& arg) {
    //     const auto& history = arg.history();
    //
    //     if (history.empty()) {
    //         return false;
    //     }
    //
    //     if (history.size() == 1) return history[0].output.is_empty_like();
    // }
    CompareFunc<T> m_func;
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
