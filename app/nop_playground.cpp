
//#include <iostream>
//#include "core/oscillator.h"
//#include "core/unit_pulse.h"
//#include "core/random_pulsator.h"
//#include "core/sequence.h"
//#include "core/distributor.h"
#include "core/algo/voice/multi_voiced.h"

//class OscillatorWrapper {
//public:
//private:
//    ParameterHandler m_parameter_handler;
//
//    // TRIGGER: TODO
//    UnitPulse m_trigger{"", m_parameter_handler};
//    Variable<Facet, Oscillator::Type> m_type{"", m_parameter_handler, Oscillator::Type::phasor};
//    Variable<Facet, float> m_freq{"", m_parameter_handler, 0.25f};
//    Variable<Facet, float> m_mul{"", m_parameter_handler, 1.0f};
//    Variable<Facet, float> m_add{"", m_parameter_handler, 0.0f};
//    Variable<Facet, float> m_duty{"", m_parameter_handler, 0.5f};
//    Variable<Facet, float> m_curve{"", m_parameter_handler, 1.0f};
//    Variable<Facet, bool> m_enabled{"", m_parameter_handler, true};
//    Variable<Facet, int> m_num_voices{"", m_parameter_handler, 1};
//
//    OscillatorNode m_oscillator{"", m_parameter_handler, &m_trigger, &m_type, &m_freq, &m_add, &m_mul
//                                , &m_duty, &m_curve, &m_enabled, &m_num_voices};
//
//    Variable<Facet, int> m_index{"", m_parameter_handler, 0};
//
//};
//
//
//class RandomPulsatorWrapper {
//public:
//    ParameterHandler m_parameter_handler;
//
//    Sequence<Facet, bool> m_enabled{"", m_parameter_handler, true};
//    Variable<Facet, int> m_num_voices{"", m_parameter_handler, 1};
//
//    Sequence<Facet, float> m_lower_bound{"", m_parameter_handler, 0.5f};
//    Sequence<Facet, float> m_upper_bound{"", m_parameter_handler, 2.0f};
//
//    RandomPulsatorNode m_random_pulsator{"", m_parameter_handler, &m_lower_bound, &m_upper_bound
//                                         , &m_enabled, &m_num_voices};
//
//};

class MyFlushable : public Flushable<int> {
public:
    MyFlushable() = default;

    Voice<int> flush() override {
        return Voice<int>(1);
    }
};


int main() {
//    RandomPulsatorWrapper random_pulsator;
//    for (int i = 0; i < 10; ++i) {
//        random_pulsator.m_random_pulsator.update_time(TimePoint(i));
//        random_pulsator.m_random_pulsator.process();
//    }

//    Distributor distributor;


MultiVoiced<MyFlushable, int> v(8);

v.flush();





}