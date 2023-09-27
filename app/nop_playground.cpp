
#include <iostream>
#include "core/oscillator.h"
#include "core/unit_pulse.h"
#include "core/random_pulsator.h"

class OscillatorWrapper {
public:
private:
    ParameterHandler m_parameter_handler;

    // TRIGGER: TODO
    UnitPulse m_trigger{"", m_parameter_handler};
    Variable<Facet, Oscillator::Type> m_type{"", m_parameter_handler, Oscillator::Type::phasor};
    Variable<Facet, float> m_freq{"", m_parameter_handler, 0.25f};
    Variable<Facet, float> m_mul{"", m_parameter_handler, 1.0f};
    Variable<Facet, float> m_add{"", m_parameter_handler, 0.0f};
    Variable<Facet, float> m_duty{"", m_parameter_handler, 0.5f};
    Variable<Facet, float> m_curve{"", m_parameter_handler, 1.0f};
    Variable<Facet, bool> m_enabled{"", m_parameter_handler, true};
    Variable<Facet, int> m_num_voices{"", m_parameter_handler, 1};

    OscillatorNode m_oscillator{"", m_parameter_handler, &m_trigger, &m_type, &m_freq, &m_add, &m_mul
                                , &m_duty, &m_curve, &m_enabled, &m_num_voices};

    Variable<Facet, int> m_index{"", m_parameter_handler, 0};

};


int main() {
    RandomPulsator pulsator;

    double lb = 0.25;
    double ub = 1.0;

    for (int i = 0; i < 50; ++i) {
        auto time = static_cast<double>(i) / 10.0;
        if (auto trigger_time = pulsator.process(time, lb, ub)) {
            std::cout << "t=" << time << ": " << *trigger_time << std::endl;
        }
    }
}