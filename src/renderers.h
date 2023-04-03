

#ifndef SERIALIST_LOOPER_RENDERERS_H
#define SERIALIST_LOOPER_RENDERERS_H

#include <vector>

#include "events.h"
#include "exceptions.h"

#include <juce_audio_devices/juce_audio_devices.h>


class Renderer {
public:

    virtual void render(Event* event) = 0;


// Note: Problematic! What if one of the call fails?
//    /**
//     * @throws IOError if rendering fails
//     */
//    void render(std::vector<std::unique_ptr<Event>> m_events) {
//        for (auto& event: m_events) {
//            render(std::move(event));
//        }
//    }


};

class MidiRenderer : public Renderer {
public:

    /**
     * @throws: IOError if a device is already initialized and override is `false`
     */
    bool initialize(const juce::String& device_identifier, bool override = false) {
        if (is_initialized() && !override) {
            throw IOError("A MIDI device is already initialized");
        }

        for (auto& device: juce::MidiOutput::getAvailableDevices()) {
            if (device.identifier == device_identifier) {
                m_midi_output = juce::MidiOutput::openDevice(device.identifier);
            }
        }

        return is_initialized();
    }


    /**
     * @throws: IOError if a device is already initialized and override is `false`
     */
    bool initialize(int device_index, bool override = false) {
        // juce::Array::operator[] implements bounds checking, will return a
        //   default (empty) MidiDeviceInfo if out of bounds
        return initialize(get_devices()[device_index].identifier, override);
    }


    /**
     * @throws: IOError if a device is already initialized and override is `false`
     */
    bool initialize(const std::string& device_name, bool override = false) {
        for (const auto& device: get_devices()) {
            if (device.name == juce::String(device_name)) {
                return initialize(device.identifier, override);
            }
        }

        return false;
    }


    /**
     * @throws: IOError if device isn't initialized or if the event is of an invalid type
     */
    void render(Event* event) override {
        if (m_midi_output == nullptr) {
            throw IOError("Device is not initialized");
        }

        auto* midi_event = dynamic_cast<MidiEvent*>(event);

        if (midi_event) {
            m_midi_output->sendMessageNow(juce::MidiMessage::noteOn(midi_event->get_channel()
                                                                    , midi_event->get_note_number()
                                                                    , (juce::uint8) midi_event->get_velocity()));
        } else {
            throw IOError("Invalid event type");
        }
    }


    static juce::Array<juce::MidiDeviceInfo> get_devices() {
        return juce::MidiOutput::getAvailableDevices();
    }


    bool is_initialized() {
        return static_cast<bool>(m_midi_output);
    }


private:
    std::unique_ptr<juce::MidiOutput> m_midi_output;
};

#endif //SERIALIST_LOOPER_RENDERERS_H
