

#ifndef SERIALISTLOOPER_MIDI_NOTE_SOURCE_COMPONENT_H
#define SERIALISTLOOPER_MIDI_NOTE_SOURCE_COMPONENT_H

#include "source.h"
#include "node_component.h"
#include "midi_config.h"
#include "slider_component.h"
#include "toggle_button_component.h"

class MidiNoteSourceComponent : public NodeComponent
                                , private juce::Timer {
public:
    MidiNoteSourceComponent(const std::string& id
                            , ParameterHandler& parent)
            : m_midi_source(id, parent)
              , m_internal_onset(id + "::onset", parent, 0.125f, 4.0f, 0.125f, 1.0f)
              , m_internal_duration(id + "::duration", parent, 0.1f, 1.1f, 0.1f, 1.0f)
              , m_internal_pitch(id + "::pitch", parent, 2100, 10800, 100, 6000)
              , m_internal_velocity(id + "::velocity", parent, 0, 127, 1, 100)
              , m_internal_channel(id + "::channel", parent, 1, 16, 1, 1)
              , m_enable_button(id + "::enabled", parent, true) {

        m_midi_source.set_onset(dynamic_cast<Node<float>*>(&m_internal_onset.get_generative()));
        m_midi_source.set_duration(dynamic_cast<Node<float>*>(&m_internal_duration.get_generative()));
        m_midi_source.set_pitch(dynamic_cast<Node<int>*>(&m_internal_pitch.get_generative()));
        m_midi_source.set_velocity(dynamic_cast<Node<int>*>(&m_internal_velocity.get_generative()));
        m_midi_source.set_channel(dynamic_cast<Node<int>*>(&m_internal_channel.get_generative()));
        m_midi_source.set_enabled(dynamic_cast<Node<bool>*>(&m_enable_button.get_generative()));

        addAndMakeVisible(m_internal_onset);
        addAndMakeVisible(m_internal_duration);
        addAndMakeVisible(m_internal_pitch);
        addAndMakeVisible(m_internal_velocity);
        addAndMakeVisible(m_internal_channel);

        m_label.setText(id, juce::dontSendNotification);
        m_label.setEditable(false);
        m_label.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(m_label);

        addAndMakeVisible(m_enable_button);

        m_midi_source.set_midi_device(MidiConfig::get_instance().get_default_device_name());
        startTimer(25);

    }


    Generative& get_generative() override {
        return m_midi_source;
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(8);

        auto header_bounds = bounds.removeFromTop(20);
        m_enable_button.setBounds(header_bounds.removeFromLeft(22));
        header_bounds.removeFromLeft(10);
        m_label.setBounds(header_bounds);

        bounds.removeFromTop(5);

        auto component_width = bounds.getWidth() / 5;
        auto spacing = component_width * 0.075;
        m_internal_onset.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_duration.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_pitch.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_velocity.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_channel.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
    }


private:
    void timerCallback() override {
        if (!m_midi_source.get_played_notes().empty())
            std::cout << "MidiNoteSourceComponent: played note\n";
    }


    MidiNoteSource m_midi_source;




    SliderComponent<float> m_internal_onset;
    SliderComponent<float> m_internal_duration;
    SliderComponent<int> m_internal_pitch;
    SliderComponent<int> m_internal_velocity;
    SliderComponent<int> m_internal_channel;

    ToggleButtonComponent m_enable_button;
    juce::Label m_label;
};


#endif //SERIALISTLOOPER_MIDI_NOTE_SOURCE_COMPONENT_H
