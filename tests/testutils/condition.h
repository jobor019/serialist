
#ifndef TESTUTILS_CONDITION_H
#define TESTUTILS_CONDITION_H

#include "results.h"
#include "matchers/matchers_common.h"

using namespace serialist;
using namespace serialist::test;

template<typename T>
class GenericCondition {
public:
    virtual ~GenericCondition() {}


    /**
     * @return std::nullopt if no comparison could be made (e.g. first value in a loop), otherwise the result
     */
    virtual std::optional<bool> matches(const Vec<StepResult<T>>& value) const = 0;
};


template<typename T>
class GenericCompareCurrent : public GenericCondition<T> {
public:
    std::optional<bool> matches(const Vec<StepResult<T>>& value) const final {
        if (value.empty()) {
            return std::nullopt;
        }

        return matches_current(value[value.size() - 1]);
    }

    virtual bool matches_current(const StepResult<T>& c) const = 0;
};

template<typename T>
class CompareValue : public GenericCompareCurrent<T> {
public:
    explicit CompareValue(const std::function<bool(const T&)>& f) : m_f(f) {}

private:
    bool matches_current(const StepResult<T>& c) const final {
        const auto& v = c.voices();

        if (v.is_empty_like()) {
            throw test_error("Empty result");
        }

        else if (!is_singular(v)) {
            throw test_error("Non-singular result");
        }

        return m_f(v[0][0]);
    }

    std::function<bool(const T&)> m_f;
};

#endif //TESTUTILS_CONDITION_H
