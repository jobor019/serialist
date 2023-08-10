#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "oscillator.h"

#include "generator.h"
#include "pulsator.h"

class OscillatorWrapper {
public:
    OscillatorWrapper()
            : osc_type("", handler, Oscillator::Type::phasor)
              , freq("", handler, 0.25f)
              , mul("", handler, 1.0f)
              , add("", handler, 0.0f)
              , duty("", handler, 0.5f)
              , curve("", handler, 1.0f)
              , enabled("", handler, true)
              , num_voices("", handler, 1)
              , oscillator("", handler, &osc_type, &freq, &add, &mul, &duty, &curve, &enabled, &num_voices) {}


    juce::UndoManager um;
    ParameterHandler handler{um};

    Variable<Facet, Oscillator::Type> osc_type;
    Variable<Facet, float> freq;
    Variable<Facet, float> mul;
    Variable<Facet, float> add;
    Variable<Facet, float> duty;
    Variable<Facet, float> curve;
    Variable<Facet, bool> enabled;
    Variable<Facet, int> num_voices;

    Oscillator oscillator;
};


TEST_CASE("Oscillator") {
    auto wrapper = OscillatorWrapper();
    auto& osc = wrapper.oscillator;

    wrapper.num_voices.set_value(4);

    auto t = TimePoint();

    for (int i = 0; i < 10; ++i) {
        std::cout << "- step " << i << "\n";
        auto s = osc.process(t);

        for (auto& e: s.vector()) {
            for (auto& ee: e.vector()) {
                std::cout << "    " << ee << "\n";
            }
        }
    }

    std::cout << "size: " << osc.get_connected().size() << "\n";
}


// ==============================================================================================

template<typename T>
class GeneratorWrapper {
public:
    GeneratorWrapper()
            : enabled("", handler, true)
              , num_voices("", handler, 1)
              , generator("", handler) {}


    juce::UndoManager um;
    ParameterHandler handler{um};

    Variable<Facet, bool> enabled;
    Variable<Facet, int> num_voices;

    Generator<T> generator;
};

TEST_CASE("Generator") {
    auto wrapper = GeneratorWrapper<Facet>();
    auto& generator = wrapper.generator;

    auto t = TimePoint();
    generator.process(t);
}


class PulsatorWrapper {
public:
    PulsatorWrapper()
            : interval("", handler, 1.0)
              , duty_cycle("", handler, 1.0)
              , enabled("", handler, true)
              , num_voices("", handler, 1)
              , pulsator("", handler, &interval, &duty_cycle, &enabled, &num_voices) {}


    juce::UndoManager um;
    ParameterHandler handler{um};


    Variable<Facet, float> interval;
    Variable<Facet, float> duty_cycle;
    Variable<Facet, float> enabled;
    Variable<Facet, float> num_voices;

    Pulsator pulsator;
};

TEST_CASE("Pulsator") {

    auto wrapper = PulsatorWrapper();
    auto& pulsator = wrapper.pulsator;
    wrapper.num_voices.set_value(10);
    wrapper.interval.set_value(0.5);
    wrapper.duty_cycle.set_value(0.5f);

    for (int i = 0; i < 10000; ++i) {
        auto t = i / 1000.0;

        auto v = pulsator.process(TimePoint(t));

        if (!v.is_empty_like()) {
            std::cout << "TICK " << t << ":  ";
        }


        for (auto& vv: v.vector()) {
            for (auto& val: vv.vector()) {
                if (val.get_type() == Trigger::Type::pulse) {
                    std::cout << "PULSE(" << val.get_id() << "), ";
                } else {
                    std::cout << "off(" << val.get_id() << "), ";
                }
            }
        }
        if (!v.is_empty_like())
            std::cout << "\n";
    }


}
