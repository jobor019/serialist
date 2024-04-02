#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>



#include "core/generatives/auto_pulsator.h"
#include "core/generatives/oscillator.h"
//#include "core/generator.h"
#include "core/generatives/note_source_LEGACY.h"
#include "midi_config.h"
#include "core/generatives/note_source_LEGACY.h"
//#include "midi_config.h"

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
    Variable<Facet, bool> enabled;
    Variable<Facet, int> num_voices;

    Pulsator pulsator;
};

TEST_CASE("Pulsator") {

    auto wrapper = PulsatorWrapper();
    auto& pulsator = wrapper.pulsator;
    wrapper.num_voices.set_value(3);
    wrapper.interval.set_value(0.5);
//    wrapper.duty_cycle.try_set_value(0.5f);

    for (int i = 0; i < 10000; ++i) {
        auto t = i / 1000.0;

        pulsator.update_time(TimePoint(t));
        auto v = pulsator.process();

        if (!v.is_empty_like()) {
            std::cout << "TICK " << t << ":  ";
        }


        for (auto& vv: v.vector()) {
            for (auto& val: vv.vector()) {
                if (val.get_type() == TriggerEvent::Type::pulse_on) {
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


class OscillatorWrapper {
public:
    OscillatorWrapper()
            : osc_type("", handler, OscillatorNode::Type::phasor)
              , freq("", handler, 0.25f)
              , mul("", handler, 1.0f)
              , add("", handler, 0.0f)
              , duty("", handler, 0.5f)
              , curve("", handler, 1.0f)
              , enabled("", handler, true)
              , num_voices("", handler, 1)
              , oscillator("", handler, &pulsator_wrapper.pulsator, &osc_type, &freq, &add, &mul, &duty, &curve, &enabled, &num_voices) {
        pulsator_wrapper.interval.set_value(1.0f);
    }


    juce::UndoManager um;
    ParameterHandler handler{um};

    Variable<Facet, OscillatorNode::Type> osc_type;



    Variable<Facet, float> freq;
    Variable<Facet, float> mul;
    Variable<Facet, float> add;
    Variable<Facet, float> duty;
    Variable<Facet, float> curve;
    Variable<Facet, bool> enabled;
    Variable<Facet, int> num_voices;

    PulsatorWrapper pulsator_wrapper;



    OscillatorNode oscillator;
};


TEST_CASE("Oscillator") {
    auto wrapper = OscillatorWrapper();
    auto& osc = wrapper.oscillator;
    auto& pulsator = wrapper.pulsator_wrapper.pulsator;

    wrapper.num_voices.set_value(4);

    for (int i = 0; i < 20; ++i) {
        auto t = TimePoint(i / 2.0);
        std::cout << "- tick: " << t.get_tick() << "\n";

        osc.update_time(t);
        pulsator.update_time(t);
        auto s = osc.process();

        for (auto& e: s.vector()) {
            for (auto& ee: e.vector()) {
                std::cout << "    " << ee << "\n";
            }
        }
    }

    std::cout << "size: " << osc.get_connected().size() << "\n";
}


// ==============================================================================================

//template<typename T>
//class GeneratorWrapper {
//public:
//    GeneratorWrapper()
//            : enabled("", handler, true)
//              , num_voices("", handler, 1)
//              , generator("", handler) {}
//
//
//    juce::UndoManager um;
//    ParameterHandler handler{um};
//
//    Variable<Facet, bool> enabled;
//    Variable<Facet, int> num_voices;
//
//    Generator<T> generator;
//};
//
//TEST_CASE("Generator") {
//    auto wrapper = GeneratorWrapper<Facet>();
//    auto& generator = wrapper.generator;
//
//    auto t = TimePoint();
//    generator.process();
//}




class NoteSourceWrapper {
public:
    NoteSourceWrapper()
            : pitch("", handler, 6000)
              , velocity("", handler, 100)
              , channel("", handler, 1)
              , enabled("", handler, true)
              , num_voices("", handler, 1)
              , note_source("", handler, &pulsator_wrapper.pulsator, &pitch, &velocity, &channel, &enabled, &num_voices) {}


    juce::UndoManager um;
    ParameterHandler handler{um};


    PulsatorWrapper pulsator_wrapper;
    Variable<Facet, float> pitch;
    Variable<Facet, float> velocity;
    Variable<Facet, float> channel;
    Variable<Facet, bool> enabled;
    Variable<Facet, float> num_voices;

    NoteSource note_source;
};


TEST_CASE("Note Root") {
    auto wrapper = NoteSourceWrapper();



    auto& note_source = wrapper.note_source;
    note_source.set_midi_device(MidiConfig::get_instance().get_default_device_name());

    auto& pulsator_wrapper = wrapper.pulsator_wrapper;
    pulsator_wrapper.interval.set_value(1.0);
    pulsator_wrapper.interval.set_value(1.0);



    for (int i = 0; i < 10000; ++i) {
        auto t = i / 1000.0;

        note_source.update_time(TimePoint(t));
        note_source.process();
    }

    for (auto& note : note_source.get_played_notes()) {
        note.print();
    }


}
