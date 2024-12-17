#ifndef TESTUTILS_CONDITION_H
#define TESTUTILS_CONDITION_H

#include "results.h"
#include "param/string_serialization.h"

using namespace serialist;
using namespace serialist::test;

// ==============================================================================================
// CONSTANTS
// ==============================================================================================

static constexpr double EPSILON = 1e-8;
static constexpr double EPSILON2 = 1e-15;


// ==============================================================================================
// UTILITIES
// ==============================================================================================

template<typename T>
Facet fcast(const T& t) {
    static_assert(utils::is_static_castable_v<T, Facet>, "Type T must be castable to Facet");
    return static_cast<Facet>(t);
}


template<typename T>
std::string serialize(const T& t) {
    return StringSerializer<T>::to_string(t);
}


template<typename T>
bool is_empty(const Voices<T>& v) { return v.is_empty_like(); }


template<typename T>
bool is_singular(const Voices<T>& v) { return v.size() == 1 && v[0].size() == 1; }


// Note: "maybe" indicate that the value is either singular or empty
template<typename T>
bool is_maybe_singular(const Voices<T>& v) { return is_empty(v) || is_singular(v); }


// ==============================================================================================


template<typename T>
class GenericCondition {
public:
    virtual ~GenericCondition() {}


    /**
     * @return std::nullopt if no comparison could be made (e.g. first value in a loop), otherwise the result
     * @throws test_error if the comparison fails
     */
    virtual std::optional<bool> matches_last(const Vec<StepResult<T> >& values) const = 0;

    /**
     * @param values values to compare
     * @param allow_no_comparison if false, will throw test_error if no comparison could be made
     *                            (e.g. change comparison on single-valued input)
     *
     * @return std::nullopt if comparison is successful, otherwise the index of the first non-failing step
     * @throws test_error if the comparison fails
     */
    virtual std::optional<std::size_t> matches_all(const Vec<StepResult<T> >& values, bool allow_no_comparison) const =
    0;

    /**
     * @param values values to compare
     * @param allow_no_comparison if false, will throw test_error if no comparison could be made
     *                            (e.g. change comparison on single-valued input)
     * @return index of the first matching step, otherwise std::nullopt if the comparison is **unsuccessful**
     * @throws test_error if the comparison fails
     */
    virtual std::optional<std::size_t> matches_any(const Vec<StepResult<T> >& values, bool allow_no_comparison) const =
    0;

    /** @return the offset(s) in relation to a given step that were used for comparison
     *          (typically 0 for value comparison or {0, -1} for change comparison) */
    virtual Vec<int> compared_step_offsets() const = 0;
};


// ==============================================================================================

template<typename T>
class GenericOutputComparison : public GenericCondition<T> {
public:
    std::optional<bool> matches_last(const Vec<StepResult<T> >& values) const final {
        if (values.empty()) {
            return std::nullopt;
        }

        return matches_internal(values[values.size() - 1]);
    }


    std::optional<std::size_t> matches_all(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison, [this](const StepResult<T>& step) {
            return !matches_internal(step); // Fail if any element matches
        });
    }


    std::optional<std::size_t> matches_any(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison, [this](const StepResult<T>& step) {
            return matches_internal(step); // Pass if any element matches
        });
    }


    Vec<int> compared_step_offsets() const override { return Vec<int>::singular(0); }

protected:
    /** @throws test_error if the comparison fails */
    virtual bool matches_internal(const StepResult<T>& current) const = 0;


    std::optional<std::size_t> matches_vector(const Vec<StepResult<T> >& values,
                                              bool allow_no_comparison,
                                              const std::function<bool(const StepResult<T>&)>& match_func) const {
        if (values.empty()) {
            if (allow_no_comparison) {
                return std::nullopt;
            }
            throw test_error("No input to compare");
        }

        for (std::size_t i = 0; i < values.size(); ++i) {
            if (match_func(values[i])) {
                return i;
            }
        }
        return std::nullopt;
    }
};


// ==============================================================================================

template<typename T>
class GenericChangeComparison : public GenericCondition<T> {
public:
    using ChangeFunc = std::function<bool(const StepResult<T>&, const StepResult<T>&)>;

    std::optional<bool> matches_last(const Vec<StepResult<T> >& values) const final {
        if (values.size() < 2) {
            return std::nullopt;
        }

        return matches_internal(values[values.size() - 2], values[values.size() - 1]);
    }


    std::optional<std::size_t> matches_all(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison
            , [this](const StepResult<T>& prev, const StepResult<T>& cur) {
            return !matches_internal(prev, cur); // Fail if any element matches
        });
    }


    std::optional<std::size_t> matches_any(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison
            , [this](const StepResult<T>& prev, const StepResult<T>& cur) {
            return matches_internal(prev, cur); // Pass if any element matches
        });
    }

protected:
    std::optional<std::size_t> matches_vector(const Vec<StepResult<T> >& values,
                                              bool allow_no_comparison,
                                              const ChangeFunc& match_func) const {
        if (values.size() < 2) {
            if (allow_no_comparison) {
                return std::nullopt;
            }
            throw test_error("Not enough input to compare");
        }

        for (std::size_t i = 1; i < values.size(); ++i) {
            if (match_func(values[i-1], values[i])) {
                return i;
            }
        }
        return std::nullopt;
    }

    Vec<int> compared_step_offsets() const override { return Vec{-1, 0}; }


protected:
    /** @throws test_error if the comparison fails */
    virtual bool matches_internal(const StepResult<T>& previous, const StepResult<T>& current) const = 0;
};


// ==============================================================================================

template<typename T>
class ValueComparison : public GenericOutputComparison<T> {
public:
    explicit ValueComparison(const std::function<bool(const T&)>& f) : m_f(f) {}

private:
    bool matches_internal(const StepResult<T>& current) const final {
        const auto& v = current.voices();

        if (v.is_empty_like()) throw test_error("Empty result: " + current.to_string() + ")");
        if (!is_singular(v)) throw test_error("Non-singular result: " + current.to_string() + ")");

        return m_f(v[0][0]);
    }


    std::function<bool(const T&)> m_f;
};


// ==============================================================================================

template<typename T>
class ValueChangeComparison : public GenericChangeComparison<T> {
public:
    using FuncType = std::function<bool(const T&, const T&)>;

    explicit ValueChangeComparison(const FuncType& f) : m_f(f) {}

private:
    bool matches_internal(const StepResult<T>& previous, const StepResult<T>& current) const final {
        const auto& p = previous.voices();
        const auto& c = current.voices();

        if (p.is_empty_like()) throw test_error("Empty result: " + previous.to_string() + ")");
        if (c.is_empty_like()) throw test_error("Empty result: " + current.to_string() + ")");

        if (!is_singular(p)) throw test_error("Non-singular result: " + previous.to_string() + ")");
        if (!is_singular(c)) throw test_error("Non-singular result: " + current.to_string() + ")");

        return m_f(p[0][0], c[0][0]);
    }


    FuncType m_f;
};

#endif //TESTUTILS_CONDITION_H
