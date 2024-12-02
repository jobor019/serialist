#ifndef V11_H
#define V11_H

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "runner_results.h"
#include "core/collections/voices.h"


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
} // namespace serialist::test::v11


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
using CompareFunc = std::function<bool(const T&, const T&)>;

template<typename T>
class ComparePrevious : public Catch::Matchers::MatcherBase<RunResult<T> > {
public:
    explicit ComparePrevious(std::string keyword, const CompareFunc<T>& f) : m_func(f), m_keyword(std::move(keyword)) {}


    bool match(const RunResult<T>& arg) const override {
        const auto& history = arg.history();

        if (history.empty()) {
            return false;
        }

        auto& fst = history[0].output;
        if (fst.size() != 1 || fst[0].size() != 1) {
            m_failure_step = history[0];
            return false;
        }

        for (int i = 1; i < history.size(); ++i) {
            auto& v = history[i].output;
            if (v.size() != 1 || v[0].size() != 1) {
                m_failure_step = history[i];
                return false;
            }

            if (!m_func(history[i - 1].output[0][0], history[i].output[0][0])) {
                m_failure_step = history[i];
                return false;
            }
        }
        return true;
    }

protected:
    std::string describe() const override {
        return "is not " + m_keyword + " previous (failure: " + m_failure_step->to_string_compact() + ")";
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

    mutable std::optional<StepResult<T> > m_failure_step; // Note: for empty vectors, this is still nullopt
};


template<typename T>
ComparePrevious<T> strictly_increasing() {
    return ComparePrevious<T>("greater than", [](const T& a, const T& b) {
        return b > a;
    });
}
}


#endif //V11_H
