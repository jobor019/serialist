#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <juce_audio_devices/juce_audio_devices.h>
#include "../src/renderers.h"
#include "../src/events.h"

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
//        midi_output->sendMessageNow(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 127)); // channel 1
//    } else {
//        std::cout << "no midi output\n";
//    }

    MidiRenderer renderer;

    bool is_initialized = renderer.initialize(std::string("IAC Driver IAC1"));

    REQUIRE(is_initialized);

    MidiEvent m{-1, 6000, 127, 2};
    renderer.render(&m);


}


