#ifndef RUNNER_RESULTS_H
#define RUNNER_RESULTS_H

#include "generative.h"
#include "collections/vec.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>


using namespace serialist;

template<typename T>
class StepAssertion {
public:
    struct AssertEqualsVs {
        Voices<T> expected;
    };
    struct AssertEqualsV {
        Voice<T> expected;
    };
    struct AssertEquals {
        T expected;
    };
    struct AssertEmpty {};
    struct AssertNotEmpty {};

    using Type = std::variant<AssertEqualsVs, AssertEqualsV, AssertEquals, AssertEmpty, AssertNotEmpty>;


    explicit constexpr StepAssertion(Type type) : m_type(type) {}

    static StepAssertion assert_equals(const Voices<T>& expected) { return StepAssertion{AssertEqualsVs{expected}}; }
    static StepAssertion assert_equals(const Voice<T>& expected) { return StepAssertion{AssertEqualsV{expected}}; }
    static StepAssertion assert_equals(const T& expected) { return StepAssertion{AssertEquals{expected}}; }
    static StepAssertion assert_empty() { return StepAssertion{AssertEmpty{}}; }
    static StepAssertion assert_not_empty() { return StepAssertion{AssertNotEmpty{}}; }


    template<typename U>
    constexpr bool is() const { return std::holds_alternative<U>(m_type); }


    template<typename U>
    U as() const { return std::get<U>(m_type); }


    bool do_assert(const Voices<T>& actual) const {
        return std::visit([&actual](const auto& arg) {
            using U = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<U, AssertEqualsVs>) {
                return voices_assertion(arg.expected, actual);
            } else if constexpr (std::is_same_v<U, AssertEqualsV>) {
                return voice_assertion(arg.expected, actual);
            } else if constexpr (std::is_same_v<U, AssertEquals>) {
                return value_assertion(arg.expected, actual);
            } else if constexpr (std::is_same_v<U, AssertEmpty>) {
                return empty_assertion(actual);
            } else if constexpr (std::is_same_v<U, AssertNotEmpty>) {
                return !empty_assertion(actual);
            } else {
                return false; // No assertion for other types
            }
        }, m_type);
    }


    Type type() const { return m_type; }


    static bool voices_assertion(const Voices<T>& expected, const Voices<T>& actual) {
        return expected == actual; // TODO: I don't think this is a valid assertion, especially not for float values
    }


    static bool voice_assertion(const Voice<T>& expected, const Voices<T>& actual) {
        // TODO: We need to handle float comparison here!!
        return actual.size() == 1 && actual[0] == expected;
    }


    static bool value_assertion(const T& expected, const Voices<T>& actual) {
        // TODO: We need to handle float comparison here!!
        return actual.size() == 1 && actual[0].size() == 1 && actual[0][0] == expected;
    }


    static bool empty_assertion(const Voices<T>& actual) {
        return actual.is_empty_like();
    }


    Type m_type;
};


// ==============================================================================================

template<typename T>
class StepResult {
public:
    StepResult(const TimePoint& time, const Voices<T>& output, std::size_t step_index
               , bool success, DomainType primary_domain)
        : time(time), output(output), step_index(step_index), success(success), m_primary_domain(primary_domain) {}


    void print() {
        std::cout << to_string() << "\n";
    }


    std::string to_string() {
        return DomainTimePoint::from_time_point(time, m_primary_domain).to_string() + ": " + output.to_string();
    }


    std::string to_string_compact() {
        return DomainTimePoint::from_time_point(time, m_primary_domain).to_string() + ": (Voices with size " +
               std::to_string(output.size()) + ")";
    }


    TimePoint time;
    Voices<T> output;
    std::size_t step_index;
    bool success;

private:
    DomainType m_primary_domain;
};


// ==============================================================================================

template<typename T>
class RunResult {
public:
    RunResult(StepResult<T> output, Vec<StepResult<T> > output_history, bool success, DomainType primary_domain)
        : m_output(output), m_output_history(output_history), m_primary_domain(primary_domain), m_success(success) {}


    StepResult<T> operator*() const {
        if (m_success) {
            return m_output;
        } else {
            FAIL(to_string_compact());
            return m_output; // Should technically never happen when run inside a test
        }
    }


    StepResult<T>* operator->() {
        if (m_success) {
            return &m_output;
        }
        FAIL(to_string_compact());
        return nullptr;
    }


    void print() {
        std::cout << to_string() << "\n";
    }


    std::string to_string() {
        if (m_success) {
            return m_output.to_string();
        } else {
            return "Failed step assertion at: "
                   + DomainTimePoint::from_time_point(m_output.time, m_primary_domain).to_string()
                   + " (step " + std::to_string(m_output.step_index)
                   + "), output:\n"
                   + m_output.to_string();
        }
    };


    std::string to_string_compact() {
        if (m_success) {
            return m_output.to_string_compact();
        } else {
            return "Failed step assertion at: "
                   + DomainTimePoint::from_time_point(m_output.time, m_primary_domain).to_string()
                   + " (step " + std::to_string(m_output.step_index)
                   + ")";
        }
    }

    const Vec<StepResult<T>>& history() const { return m_output_history; }


    bool success() const { return m_success; }


    std::size_t num_steps() const { return m_output_history.size() + 1; }

private:
    StepResult<T> m_output;
    Vec<StepResult<T> > m_output_history;

    DomainType m_primary_domain;

    bool m_success;
};

#endif //RUNNER_RESULTS_H
