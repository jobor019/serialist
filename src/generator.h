

#ifndef SERIALIST_LOOPER_GENERATOR_H
#define SERIALIST_LOOPER_GENERATOR_H

#include <optional>

#include "phasor.h"
#include "mapping.h"
#include "graph_node.h"
#include "oscillator.h"


template<typename T>
class Generator : public GraphNode<T> {
public:

    Generator() = default;


    explicit Generator(std::shared_ptr<Oscillator> oscillator
                       , double step_size = 0.1
                       , double phase = 0.0
                       , double mul = 1.0
                       , Phasor::Mode mode = Phasor::Mode::stepped
                       , std::optional<InterpolationMapping<T> > mapping = std::nullopt)
            : m_oscillator(std::move(oscillator))
              , m_phasor{step_size, 1.0, phase, mode}
              , m_mapping(mapping)
              , m_mul(mul) {}


    std::vector<T> process(const TimePoint& time) override {
        auto x = m_phasor.process(time.get_tick());
        auto y = m_oscillator->process(x) * m_mul;
        if (m_mapping) {
            auto res = m_mapping->interpolate(y);
            if (!res) {
                return {};          // std::nullopt
            }
            return {res.value()};   // always only one value
        }

        return {static_cast<T>(y)};
    }


    void set_oscillator(std::shared_ptr<Oscillator>& oscillator) {
        m_oscillator = oscillator;
    }


    void set_step_size(double step_size) {
        m_phasor.set_step_size(step_size);
    }


    void set_phase(double phase) {
        m_phasor.set_phase(phase, true);
    }


    void set_mode(Phasor::Mode mode) {
        m_phasor.set_mode(mode);
    }

    void set_mapping(std::optional<InterpolationMapping<T> > mapping) {
        m_mapping = mapping;
    }


private:
    std::shared_ptr<Oscillator> m_oscillator;
    Phasor m_phasor;
    std::optional<InterpolationMapping<T> > m_mapping;

    double m_mul = 1.0;

};


#endif //SERIALIST_LOOPER_GENERATOR_H
