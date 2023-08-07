#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "oscillator.h"

#include "generator.h"

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

        for (auto& e : s.vector()) {
            for (auto& ee : e.vector()) {
                std::cout << "    " << ee << "\n";
            }
        }
    }
}


// ==============================================================================================

template<typename T>
class GeneratorWrapper {
public:
    GeneratorWrapper()
            : enabled("", handler, true)
              ,num_voices("", handler, 1)
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