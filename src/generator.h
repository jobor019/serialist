

#ifndef SERIALIST_LOOPER_GENERATOR_H
#define SERIALIST_LOOPER_GENERATOR_H

#include <optional>

#include "phasor.h"
#include "mapping.h"
#include "graph_node.h"
#include "oscillator.h"
#include "interpolator.h"
#include "selector.h"


template<typename T>
class Generator : public GraphNode<T> {
public:

    explicit Generator(double step_size = 0.1
                       , double phase = 0.0
                       , Phasor::Mode mode = Phasor::Mode::stepped
                       , std::unique_ptr<Oscillator> oscillator = std::make_unique<Identity>()
                       , std::unique_ptr<Mapping<T>> mapping = nullptr
                       , std::unique_ptr<Interpolator<T>> interpolator = std::make_unique<ClipInterpolator<T>>()
                       , std::unique_ptr<Selector<T>> selector = std::make_unique<IdentitySelector<T>>()
                       , std::unique_ptr<GraphNode<T>> output_add = nullptr
                       , std::unique_ptr<GraphNode<T>> output_mul = nullptr
                       , std::unique_ptr<GraphNode<double>> oscillator_add = nullptr
                       , std::unique_ptr<GraphNode<double>> oscillator_mul = nullptr)
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
            throw std::runtime_error("An oscillator must always be provided");
        if (!m_interpolator)
            throw std::runtime_error("An interpolator must always be provided");
        if (!m_selector)
            throw std::runtime_error("A selector must always be provided");

        if constexpr (!std::is_arithmetic_v<T>) {
            if (m_output_add)
                throw std::runtime_error("output_add can only be provided for arithmetic types T");
            if (m_output_mul)
                throw std::runtime_error("output_mul can only be provided for arithmetic types T");
            if (!m_mapping)
                throw std::runtime_error("A mapping must be provided for any non-arithmetic type T");
        }
    }


    std::vector<T> process(const TimePoint& time) override {
        auto x = calculate_phasor_position(time);
        auto y = m_oscillator->process(x);

        std::cout << y << "\n";

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


    void set_interpolator(std::unique_ptr<Interpolator<T>> interpolator) {
        if (!interpolator)
            throw std::runtime_error("An interpolator must always be provided");

        m_interpolator = std::move(interpolator);
    }


    void set_selector(std::unique_ptr<Selector<T>> selector) {
        if (!selector)
            throw std::runtime_error("A selector must always be provided");

        m_selector = std::move(selector);
    }


    void set_output_add(std::unique_ptr<GraphNode<T>> output_add) {
        if constexpr (!std::is_arithmetic_v<T>)
            throw std::runtime_error("output_mul can only be provided for arithmetic types T");
        m_output_add = std::move(output_add);
    }


    void set_output_mul(std::unique_ptr<GraphNode<T>> output_mul) {
        if constexpr (!std::is_arithmetic_v<T>)
            throw std::runtime_error("output_mul can only be provided for arithmetic types T");

        m_output_mul = std::move(output_mul);
    }


    void set_step_size(double step_size) { m_phasor.set_step_size(step_size); }

    void set_phase(double phase) { m_phasor.set_phase(phase, true); }

    void set_mode(Phasor::Mode mode) { m_phasor.set_mode(mode); }

    void set_phasor_add(std::unique_ptr<GraphNode<double>> phasor_add) { m_oscillator_add = std::move(phasor_add); }

    void set_phasor_mul(std::unique_ptr<GraphNode<double>> phasor_mul) { m_oscillator_mul = std::move(phasor_mul); }


    [[nodiscard]] Oscillator* get_oscillator() const { return m_oscillator.get(); }

    [[nodiscard]] Mapping<T>* get_mapping() const { return m_mapping.get(); }

    [[nodiscard]] Interpolator<T>* get_interpolator() const { return m_interpolator.get(); }

    [[nodiscard]] Selector<T>* get_a_selector() const { return m_selector.get(); }

    [[nodiscard]] GraphNode<T>* get_output_add() const { return m_output_add.get(); }

    [[nodiscard]] GraphNode<T>* get_output_mul() const { return m_output_mul.get(); }

    [[nodiscard]] GraphNode<double>* get_phasor_add() const { return m_oscillator_add.get(); }

    [[nodiscard]] GraphNode<double>* get_phasor_mul() const { return m_oscillator_mul.get(); }



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

        return x * phasor_mul + phasor_add;

    }

    std::vector<T> apply_output_add_mul(std::vector<T> elements, const TimePoint& time) {
        if constexpr(!std::is_arithmetic_v<T>) {
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

    std::unique_ptr<Interpolator<T>> m_interpolator;
    std::unique_ptr<Selector<T>> m_selector;

    std::unique_ptr<GraphNode<T>> m_output_add;
    std::unique_ptr<GraphNode<T>> m_output_mul;

    std::unique_ptr<GraphNode<double>> m_oscillator_add;
    std::unique_ptr<GraphNode<double>> m_oscillator_mul;

};


#endif //SERIALIST_LOOPER_GENERATOR_H
