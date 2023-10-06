
//#include <iostream>
//#include "core/oscillator.h"
//#include "core/unit_pulse.h"
//#include "core/random_pulsator.h"
//#include "core/sequence.h"
//#include "core/distributor.h"
#include "core/algo/voice/multi_voiced.h"
#include "core/algo/pitch/notes.h"
#include "core/events.h"

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


class A {
public:
    explicit A(const std::string& name) : m_name(name) {}

    void print() const {
        std::cout << m_name << std::endl;
    }

private:
    std::string m_name;
};


void ff(const Voices<int>& t) {
    auto& z = t[2];
    z.print();
}


void fff(const Vec<A>& a) {
    a[2].print();
}

void applish(const Vec<bool>& a) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i]) {
            std::cout << i << std::endl;
        }
    }
}


int main() {
//    auto a = Voices<int>::transposed(Vec<int>{1, 2, 3, 4});
//    a[2].print();
//    ff(a);
//    Vec<A> v{A{"a"}, A{"b"}, A{"c"}};
//    fff(v);

    Vec mask = {true, false, true, false, true};
    applish(mask);
}