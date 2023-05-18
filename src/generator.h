

#ifndef SERIALIST_LOOPER_GENERATOR_H
#define SERIALIST_LOOPER_GENERATOR_H

#include <optional>

#include "phasor.h"
#include "mapping.h"
#include "generative.h"
#include "oscillator.h"
#include "interpolator.h"
#include "selector.h"
#include "parameter_policy.h"


template<typename T>
class Generator : public Node<T> {
public:

    explicit Generator(double step_size = 0.1
                       , double phase = 0.0
                       , Phasor::Mode mode = Phasor::Mode::stepped
                       , std::unique_ptr<Oscillator> oscillator = std::make_unique<Identity>()
                       , std::unique_ptr<Mapping<T>> mapping = nullptr
                       , std::unique_ptr<OldInterpolator<T>> interpolator = std::make_unique<ClipInterpolation<T>>()
                       , std::unique_ptr<SelectionPattern<T>> selector = std::make_unique<IdentitySelection<T>>()
                       , std::unique_ptr<Node<T>> output_add = nullptr
                       , std::unique_ptr<Node<T>> output_mul = nullptr
                       , std::unique_ptr<Node<double>> oscillator_add = nullptr
                       , std::unique_ptr<Node<double>> oscillator_mul = nullptr)
            : m_phasor{step_size, 1.0, phase, mode}
              , m_oscillator(std::move(oscillator))
              , m_mapping(std::move(mapping))
              , m_interpolator(std::move(interpolator))
              , m_selector(std::move(selector))
              , m_output_add(std::move(output_add))
              , m_output_mul(std::move(output_mul))
              , m_oscillator_add(std::move(oscillator_add))
              , m_oscillator_mul(std::move(oscillator_mul)) {
        if (!m_oscillator)
            throw std::runtime_error("An oscillator must be provided");
        if (!m_interpolator)
            throw std::runtime_error("An interpolator must be provided");
        if (!m_selector)
            throw std::runtime_error("A selector must be provided");

        if constexpr (!std::is_arithmetic_v<T>) {
            if (m_output_add)
                throw std::runtime_error("output_add can only be provided for arithmetic types T");
            if (m_output_mul)
                throw std::runtime_error("output_mul can only be provided for arithmetic types T");
            if (!m_mapping)
                throw std::runtime_error("A mapping must be provided for any non-arithmetic type T");
        }
    }


    explicit Generator(std::unique_ptr<Mapping<T>> mapping)
            : Generator(0.0
                        , 0.0
                        , Phasor::Mode::stepped
                        , std::make_unique<Identity>()
                        , std::move(mapping)) {
        set_step_size(1 / static_cast<double>(m_mapping->size()));
    }


    std::vector<T> process(const TimePoint& time) override {
        auto x = calculate_phasor_position(time);
        auto y = m_oscillator->process(x);

        if (!m_mapping) {               // may only occur if T is arithmetic
            return {static_cast<T>(y)};
        }

        auto output_candidates = m_interpolator->get(y, m_mapping.get());

        output_candidates = apply_output_add_mul(output_candidates, time);

        return m_selector->get(output_candidates);
    }


    void set_oscillator(std::unique_ptr<Oscillator> oscillator) {
        if (!oscillator)
            throw std::runtime_error("An oscillator must always be provided");

        m_oscillator = std::move(oscillator);
    }


    void set_mapping(std::unique_ptr<Mapping<T>> mapping) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (!mapping) {
                throw std::runtime_error("A mapping must be provided for any non-arithmetic type T");
            }
        }

        m_mapping = std::move(mapping);
    }


    void set_interpolator(std::unique_ptr<OldInterpolator<T>> interpolator) {
        if (!interpolator)
            throw std::runtime_error("An interpolator must always be provided");

        m_interpolator = std::move(interpolator);
    }


    void set_selector(std::unique_ptr<SelectionPattern<T>> selector) {
        if (!selector)
            throw std::runtime_error("A selector must always be provided");

        m_selector = std::move(selector);
    }


    void set_output_add(std::unique_ptr<Node<T>> output_add) {
        if constexpr (!std::is_arithmetic_v<T>)
            throw std::runtime_error("output_mul can only be provided for arithmetic types T");
        m_output_add = std::move(output_add);
    }


    void set_output_mul(std::unique_ptr<Node<T>> output_mul) {
        if constexpr (!std::is_arithmetic_v<T>)
            throw std::runtime_error("output_mul can only be provided for arithmetic types T");

        m_output_mul = std::move(output_mul);
    }


    void set_step_size(double step_size) { m_phasor.set_step_size(step_size); }


    void set_phase(double phase) { m_phasor.set_phase(phase, true); }


    void set_mode(Phasor::Mode mode) { m_phasor.set_mode(mode); }


    void set_phasor_add(std::unique_ptr<Node<double>> phasor_add) { m_oscillator_add = std::move(phasor_add); }


    void set_phasor_mul(std::unique_ptr<Node<double>> phasor_mul) { m_oscillator_mul = std::move(phasor_mul); }


    [[nodiscard]] double get_step_size() const { return m_phasor.get_step_size(); }


    [[nodiscard]] Oscillator* get_oscillator() const { return m_oscillator.get(); }


    [[nodiscard]] Mapping<T>* get_mapping() const { return m_mapping.get(); }


    [[nodiscard]] OldInterpolator<T>* get_interpolator() const { return m_interpolator.get(); }


    [[nodiscard]] SelectionPattern<T>* get_a_selector() const { return m_selector.get(); }


    [[nodiscard]] Node<T>* get_output_add() const { return m_output_add.get(); }


    [[nodiscard]] Node<T>* get_output_mul() const { return m_output_mul.get(); }


    [[nodiscard]] Node<double>* get_phasor_add() const { return m_oscillator_add.get(); }


    [[nodiscard]] Node<double>* get_phasor_mul() const { return m_oscillator_mul.get(); }


    [[nodiscard]] double get_phasor_position() const { return m_x; }

// TODO: Update. Taken from Looper
//    void add_element(T element, long index = -1) {
//
//
//        m_mapping.add(std::move(element), index);
//
//        // Increment phasor range by number of elements inserted
//        m_phasor.set_max(m_phasor.get_max() + 1.0);
//
//        long insertion_point = index;
//        if (index < 0) {
//            insertion_point += m_mapping.size();
//        }
//
//        // If inserting before current value, increment to avoid risk of repeating the same value twice
//        auto current_phase = m_phasor.get_current_value();
//        if (static_cast<double>(insertion_point) < current_phase) {
//            current_phase += 1.0;
//            m_phasor.set_phase(current_phase, false);
//        }
//    }


private:

    double calculate_phasor_position(const TimePoint& time) {
        auto x = m_phasor.process(time.get_tick());

        auto phasor_add = 0.0;
        auto phasor_mul = 1.0;

        if (m_oscillator_add) {
            auto add = m_oscillator_add->process(time);
            if (!add.empty())
                phasor_add = add[0];
        }

        if (m_oscillator_mul) {
            auto mul = m_oscillator_mul->process(time);
            if (!mul.empty())
                phasor_mul = mul[0];
        }

        x = x * phasor_mul + phasor_add;
        m_x = x;

        return x;

    }


    std::vector<T> apply_output_add_mul(std::vector<T> elements, const TimePoint& time) {
        if constexpr (!std::is_arithmetic_v<T>) {
            return elements;
        }

        auto add = static_cast<T>(0);
        auto mul = static_cast<T>(1);

        if (m_output_add) {
            auto add_value = m_output_add->process(time);
            if (!add_value.empty())
                add = add_value[0];
        }

        if (m_output_mul) {
            auto mul_value = m_output_mul->process(time);
            if (!mul_value.empty())
                mul = mul_value[0];
        }

        for (auto& element: elements) {
            element = element * mul + add;
        }

        return elements;
    }


    Phasor m_phasor;
    std::unique_ptr<Oscillator> m_oscillator;
    std::unique_ptr<Mapping<T>> m_mapping;

    std::unique_ptr<OldInterpolator<T>> m_interpolator;
    std::unique_ptr<SelectionPattern<T>> m_selector;

    std::unique_ptr<Node<T>> m_output_add;
    std::unique_ptr<Node<T>> m_output_mul;

    std::unique_ptr<Node<double>> m_oscillator_add;
    std::unique_ptr<Node<double>> m_oscillator_mul;

    double m_x = 0.0;

};


#endif //SERIALIST_LOOPER_GENERATOR_H
