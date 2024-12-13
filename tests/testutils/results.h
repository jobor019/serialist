#ifndef TESTUTILS_RESULTS_H
#define TESTUTILS_RESULTS_H

#include <core/policies/policies.h>
#include <core/generative.h>
#include <core/collections/vec.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "exceptions.h"


namespace serialist::test {
template<typename T>
class StepResult {
public:
    static constexpr std::size_t NUM_DECIMALS_DETAILED = 16;


    static StepResult success(const Voices<T>& voices
                              , const TimePoint& time
                              , std::size_t step_index
                              , DomainType primary_domain) {
        return StepResult(voices, time, step_index, primary_domain);
    }


    static StepResult failure(const std::string& err
                              , const TimePoint& time
                              , std::size_t step_index
                              , DomainType primary_domain) {
        return StepResult(err, time, step_index, primary_domain);
    }


    friend std::ostream& operator<<(std::ostream& os, const StepResult& obj) {
        os << obj.to_string();
        return os;
    }


    void print(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::cout << to_string(compact, num_decimals) << "\n";
    }


    void print_detailed(std::size_t num_decimals = NUM_DECIMALS_DETAILED) const {
        print(false, num_decimals);
    }


    std::string to_string(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        if (is_successful()) {
            return render_output(compact, num_decimals) + " " + render_step_info(compact, num_decimals);
        } else {
            return render_error() + " " + render_step_info(compact, num_decimals);
        }
    }


    std::string to_string_detailed(std::size_t num_decimals = NUM_DECIMALS_DETAILED) const {
        return to_string(false, num_decimals);
    }


    const Voices<T>& voices() const {
        if (is_successful()) {
            return std::get<Voices<T> >(m_value);
        } else {
            throw test_error(render_error());
        }
    }


    bool is_successful() const { return std::holds_alternative<Voices<T> >(m_value); }
    const TimePoint& time() const { return m_time; }
    std::size_t step_index() const { return m_step_index; }

private:
    StepResult(const std::variant<Voices<T>, std::string>& value
               , const TimePoint& time
               , std::size_t step_index
               , DomainType primary_domain)
        : m_value(value), m_time(time), m_step_index(step_index), m_primary_domain(primary_domain) {}


    std::string render_output(bool compact, std::optional<std::size_t> num_decimals = std::nullopt) const {
        const auto& v = voices();
        std::size_t num_elements = v.numel();
        if (!compact || num_elements < 10) {
            return v.to_string(num_decimals);
        } else {
            return "Voices(size="
                   + std::to_string(v.size())
                   + ", numel="
                   + std::to_string(num_elements) + ")";
        }
    }


    std::string render_error() const {
        return std::get<std::string>(m_value);
    }


    std::string render_step_info(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::stringstream ss;

        ss << " (i=" << m_step_index << ", t=";
        if (compact) {
            ss << DomainTimePoint::from_time_point(m_time, m_primary_domain).to_string_compact();
        } else {
            ss << DomainTimePoint::from_time_point(m_time, m_primary_domain).to_string(num_decimals);
        }
        ss << ")";
        return ss.str();
    }


    std::variant<Voices<T>, std::string> m_value; // Voices<T> if successful, otherwise error description
    TimePoint m_time;
    std::size_t m_step_index;
    DomainType m_primary_domain;
};


// ==============================================================================================

template<typename T>
class RunResult {
public:
    RunResult(const Vec<StepResult<T> >& run_output
              , DomainType primary_domain)
        : m_run_output(run_output)
          , m_primary_domain(primary_domain) {
        if (run_output.size() == 0) throw test_error("Empty run output");
    }


    static RunResult failure(const std::string& err
                             , const TimePoint& t = TimePoint{}
                             , std::size_t step_index = 0
                             , DomainType primary_domain = DomainType::ticks
                             , const Vec<StepResult<T> >& output_history = {}) {
        auto run_output = output_history.cloned();
        run_output.append(StepResult<T>::failure(err, t, step_index, primary_domain));
        return {run_output, primary_domain};
    }


    static RunResult dummy(const StepResult<T>& s) {
        return RunResult({s}, DomainType::ticks);
    }


    static RunResult dummy(const Voices<T>& v) {
        return dummy(StepResult<T>::success(v, TimePoint{}, 0, DomainType::ticks));
    }


    static RunResult dummy(const T& v, const Vec<T>& history = Vec<T>()) {
        auto h = Vec<StepResult<T> >::allocated(history.size() + 1);
        for (std::size_t i = 0; i < history.size(); ++i) {
            h.append(StepResult<T>::success(Voices<T>::singular(history[i]), TimePoint{}, i, DomainType::ticks));
        }

        h.append(StepResult<T>::success(Voices<T>::singular(v), TimePoint{}, history.size(), DomainType::ticks));
        return RunResult(h, DomainType::ticks);
    }

    friend std::ostream& operator<<(std::ostream& os, const RunResult& obj) {
        os << obj.to_string();
        return os;
    }


    void print() const {
        std::cout << to_string() << "\n";
    }


    void print_detailed(std::size_t num_decimals = StepResult<T>::NUM_DECIMALS_DETAILED) const {
        std::cout << to_string_detailed(num_decimals) << "\n";
    }


    std::string to_string(bool compact = false, std::optional<std::size_t> num_decimals = std::nullopt) const {
        return last().to_string(compact, num_decimals);
    }


    std::string to_string_detailed(std::size_t num_decimals = StepResult<T>::NUM_DECIMALS_DETAILED) const {
        return to_string(false, num_decimals);
    }


    Vec<StepResult<T>> history() const { return m_run_output.slice(0, -1); }

    StepResult<T> last() const { return *m_run_output.last(); }

    const Vec<StepResult<T> >& entire_output() const { return m_run_output; }


    bool is_successful() const { return last().is_successful(); }


    std::size_t num_steps() const { return m_run_output.size(); }

private:
    Vec<StepResult<T> > m_run_output; // Invariant: m_run_output.size() > 0

    DomainType m_primary_domain;
};
}

#endif // TESTUTILS_RESULTS_H
