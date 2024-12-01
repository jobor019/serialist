#ifndef RUNNER_RESULTS_H
#define RUNNER_RESULTS_H

#include "generative.h"
#include "collections/vec.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>


using namespace serialist;

// TODO: This should probably have been implemented with dynamic polymorphism instead
template<typename T>
class StepOutput {
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
    struct Discard {};
    struct Output {};

    using Type = std::variant<AssertEqualsVs, AssertEqualsV, AssertEquals, AssertEmpty, Discard, Output>;


    explicit StepOutput(Type type) : m_type(type) {}

    static StepOutput assert_equals(const Voices<T>& expected) { return {AssertEqualsVs{expected}}; }
    static StepOutput assert_equals(const Voice<T>& expected) { return {AssertEqualsV{expected}}; }
    static StepOutput assert_equals(const T& expected) { return {AssertEquals{expected}}; }
    static StepOutput assert_empty() { return {AssertEmpty{}}; }
    static StepOutput discard() { return {Discard{}}; }
    static StepOutput output() { return {Output{}}; }


    template<typename U, typename = std::enable_if_t<std::is_same_v<U, Type>>>
    bool is() const { return std::holds_alternative<U>(m_type); }


    template<typename U>
    U as() const { return std::get<U>(m_type); }

    constexpr bool is_assertion() const {
        return is<AssertEqualsVs>() || is<AssertEqualsV>() || is<AssertEquals>() || is<AssertEmpty>();
    }

    bool do_assert(const Voices<T> actual) {
        if constexpr (is<AssertEqualsVs>()) {
            return voices_assertion(as<AssertEqualsVs>().expected, actual);
        } else if (is<AssertEqualsV>()) {
            return voice_assertion(as<AssertEqualsV>().expected, actual);
        } else if (is<AssertEquals>()) {
            return value_assertion(as<AssertEquals>().expected, actual);
        } else if (is<AssertEmpty>()) {
            return empty_assertion(actual);
        }

        return false;
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

    static bool empty_assertion(const Voices<T&> actual) {
        return actual.is_empty_like();
    }


    Type m_type;
};


// ==============================================================================================

template<typename T>
class StepResult {
public:
    StepResult(const TimePoint& time, const Voices<T>& output, bool success, DomainType primary_domain)
        : time(time), output(output), success(success), m_primary_domain(primary_domain) {}


    void print() {
        std::cout << to_string() << "\n";
    }


    std::string to_string() {
        return DomainTimePoint::from_time_point(time, m_primary_domain).to_string() + ": " + output.to_string();
    }


    std::string to_string_compact() {
        return DomainTimePoint::from_time_point(time, m_primary_domain).to_string() + ": (Voices with size " +
               output.size() + ")";
    }

    TimePoint time;
    Voices<T> output;
    bool success;

private:
    DomainType m_primary_domain;


};


// ==============================================================================================

template<typename T>
class RunResult {
public:
    RunResult(StepResult<T> output, Vec<StepResult<T> > output_history, bool success, DomainType primary_domain)
        : output(output), output_history(output_history), m_primary_domain(primary_domain), success(success) {}


    StepResult<T> operator*() const {
        if (success) {
            return output;
        } else {
            FAIL(to_string_compact());
            return output; // Should technically never happen when run inside a test
        }
    }


    void print() {
        std::cout << to_string() << "\n";
    }


    std::string to_string() {
        if (success) {
            return output.to_string();
        } else {
            return "Failed step assertion at: "
                   + DomainTimePoint::from_time_point(output.m_time, m_primary_domain).to_string()
                   + "\noutput:\n"
                   + output.to_string();
        }
    };


    std::string to_string_compact() {
        if (success) {
            return output.to_string_compact();
        } else {
            return "Failed step assertion at: "
                   + DomainTimePoint::from_time_point(output.m_time, m_primary_domain).to_string();
        }
    }

private:
    StepResult<T> output;
    Vec<StepResult<T> > output_history;

    DomainType m_primary_domain;

    bool success;
};

#endif //RUNNER_RESULTS_H
