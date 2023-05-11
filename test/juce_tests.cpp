#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <juce_audio_devices/juce_audio_devices.h>
#include "../src/renderers.h"
#include "../src/events.h"
#include "../src/scheduler.h"
#include "../src/generation_graph.h"

TEST_CASE("rendering", "[midi, renderer]") {

//    juce::String device_name{"IAC Driver IAC1"};
//    std::unique_ptr<juce::MidiOutput> midi_output;
//
//    for (auto& device: juce::MidiOutput::getAvailableDevices()) {
//        std::cout << device.name << " " << device.identifier << "\n";
//        if (device.name == device_name) {
//            midi_output = juce::MidiOutput::openDevice(device.identifier);
//        }
//    }
//
//    if (midi_output) {
//        midi_output->sendMessageNow(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 127)); // m_channel 1
//    } else {
//        std::cout << "no midi output\n";
//    }

    MidiRenderer renderer;

    bool is_initialized = renderer.initialize(std::string("IAC Driver IAC1"));

    REQUIRE(is_initialized);

    MidiEvent m{-1, 6000, 127, 1};
    renderer.render(&m);


}

//TEST_CASE("midi graph", "[generation, integration]") {
//
//    Scheduler scheduler;
//
//
//    SimplisticMidiGraphV1 m{
//            std::make_unique<Looper<double>>(MultiMapping<double>{1.0, 1.0, 1.0, 2.0, 1.0}, 1.0, 0.0, Phasor::Mode::stepped)
//            , std::make_unique<Looper<double>>(MultiMapping<double>{1.0}, 1.0, 0.0, Phasor::Mode::stepped)
//            , std::make_unique<Looper<int>>(MultiMapping<int>{6000, 6200, 6400, 6700}, 1.0, 0.0, Phasor::Mode::stepped)
//            , std::make_unique<Looper<int>>(MultiMapping<int>{100}, 1.0, 0.0, Phasor::Mode::stepped)
//            , std::make_unique<Looper<int>>(MultiMapping<int>{1}, 1.0, 0.0, Phasor::Mode::stepped)};
//
//    double t = 0.0;
//    for (int i = 0; i < 10; ++i) {
//        auto events = m.process(TimePoint(t));
//
//        for (auto& event: events) {
//            if (auto trigger_event = dynamic_cast<TriggerEvent*>(event.get())) {
//                std::cout << "trigger: @" << trigger_event->get_time() << std::endl;
//                t = event->get_time();
//            } else if (auto note_event = dynamic_cast<MidiEvent*>(event.get())) {
//
//                std::cout << "note:    @" << note_event->get_time() << " (" << note_event->get_note_number() << ", "
//                          << note_event->get_velocity() << ", " << note_event->get_channel() << ")\n";
//            }
//            scheduler.add_event(std::move(event));
//        }
//    }
//
//    MidiRenderer renderer;
//    bool is_initialized = renderer.initialize(std::string("IAC Driver IAC1"));
//
//    REQUIRE(is_initialized);
//
//    double time = 0;
//    while (!scheduler.is_empty()) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(1));
//
//        auto events = scheduler.get_events(time);
//
//        for (auto& event: events) {
//            if (auto note_event = dynamic_cast<MidiEvent*>(event.get())) {
//                renderer.render(note_event);
//            }
//        }
//
//
//        time += 0.001;
//    }
//
//
//}
