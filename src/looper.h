

#ifndef SERIALIST_LOOPER_LOOPER_H
#define SERIALIST_LOOPER_LOOPER_H

#include "phasor.h"
#include "mapping.h"
#include "graph_node.h"

template<typename T>
class Looper : public GraphNode<T> {
public:
    explicit Looper() = default;

    explicit Looper(Mapping<T> mapping
                    , double step_size = 1.0
                    , double phase = 0.0
                    , Phasor::Mode mode = Phasor::Mode::stepped)
            : m_mapping(mapping)
              , m_phasor{step_size, static_cast<double>(mapping.size()), phase, mode} {}


    std::vector<T> process(const TimePoint& time) override {
        double x = m_phasor.process(time.get_tick());
        return m_mapping.get(static_cast<unsigned long>(std::floor(x)));
    }


    void add(MapElement<T> element, long index = -1) {
        m_mapping.add(std::move(element), index);

        // Increment phasor range by number of elements inserted
        m_phasor.set_max(m_phasor.get_max() + 1.0);

        long insertion_point = index;
        if (index < 0) {
            insertion_point += m_mapping.size();
        }

        // If inserting before current value, increment to avoid risk of repeating the same value twice
        auto current_phase = m_phasor.get_current_value();
        if (static_cast<double>(insertion_point) < current_phase) {
            current_phase += 1.0;
            m_phasor.set_phase(current_phase, false);
        }
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


private:
    Mapping<T> m_mapping;
    Phasor m_phasor;

};

#endif //SERIALIST_LOOPER_LOOPER_H
