

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

    enum class MapMode {
        cont, mod, clip, pass
    };

    explicit Generator() = default;

    explicit Generator(std::shared_ptr<Oscillator>& oscillator
                       , double step_size = 1.0
                       , double phase = 0.0
                       , Phasor::Mode mode = Phasor::Mode::stepped
                       , std::optional<Mapping<T>> mapping = std::nullopt
                       , MapMode map_mode = MapMode::cont)
            : m_oscillator(oscillator)
              , m_phasor{step_size, static_cast<double>(mapping.size()), phase, mode}
              , m_mapping(mapping)
              , m_map_mode(map_mode) {}

    std::vector<T> process(const TimePoint& time) override {
        throw std::runtime_error("not implemented");
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

    void set_mapping(Mapping<T> mapping) {
        m_mapping = mapping;
    }

    void set_map_mode(MapMode map_mode) {
        m_map_mode = map_mode;
    }


private:

    std::shared_ptr<Oscillator> m_oscillator;
    std::optional<Mapping<T>> m_mapping;
    Phasor m_phasor;
    MapMode m_map_mode;


}


#endif //SERIALIST_LOOPER_GENERATOR_H
