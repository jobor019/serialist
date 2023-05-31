

#ifndef SERIALISTLOOPER_MIDI_NOTE_SOURCE_COMPONENT_H
#define SERIALISTLOOPER_MIDI_NOTE_SOURCE_COMPONENT_H

#include "source.h"
#include "node_component.h"
#include "midi_config.h"
#include "slider_object.h"
#include "toggle_button_object.h"
#include "header_component.h"
#include "note_view.h"

class MidiNoteSourceComponent : public GenerativeComponent
                                , private juce::Timer {
public:
    MidiNoteSourceComponent(const std::string& id
                            , ParameterHandler& parent)
            : m_midi_source(id, parent)
              , m_header(id, parent)
              , m_visualizer(m_midi_source)
              , m_internal_onset(id + "::onset", parent, 0.125f, 4.0f, 0.125f, 1.0f, "onset")
              , m_internal_duration(id + "::duration", parent, 0.1f, 1.1f, 0.1f, 1.0f, "dur")
              , m_internal_pitch(id + "::pitch", parent, 2100, 10800, 100, 6000, "pitch")
              , m_internal_velocity(id + "::velocity", parent, 0, 127, 1, 100, "vel")
              , m_internal_channel(id + "::channel", parent, 1, 16, 1, 1, "ch") {

        m_midi_source.set_onset(dynamic_cast<Node<float>*>(&m_internal_onset.get_generative()));
        m_midi_source.set_duration(dynamic_cast<Node<float>*>(&m_internal_duration.get_generative()));
        m_midi_source.set_pitch(dynamic_cast<Node<int>*>(&m_internal_pitch.get_generative()));
        m_midi_source.set_velocity(dynamic_cast<Node<int>*>(&m_internal_velocity.get_generative()));
        m_midi_source.set_channel(dynamic_cast<Node<int>*>(&m_internal_channel.get_generative()));
        m_midi_source.set_enabled(dynamic_cast<Node<bool>*>(&m_header.get_enabled().get_generative()));

        addAndMakeVisible(m_header);
        addAndMakeVisible(m_visualizer);
        addAndMakeVisible(m_internal_onset);
        addAndMakeVisible(m_internal_duration);
        addAndMakeVisible(m_internal_pitch);
        addAndMakeVisible(m_internal_velocity);
        addAndMakeVisible(m_internal_channel);

        m_midi_source.set_midi_device(MidiConfig::get_instance().get_default_device_name());
        startTimer(25);

    }

    std::pair<int, int> dimensions() override {
        return {0, 0};
    }


    Generative& get_generative() override {
        return m_midi_source;
    }


    void paint(juce::Graphics& g) override {
        g.setColour(getLookAndFeel().findColour(Colors::component_background_color));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
        g.setColour(getLookAndFeel().findColour(Colors::component_border_color));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
    }


    void resized() override {
        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(m_header.default_height()));
        bounds.reduce(5, 8);

        m_visualizer.setBounds(bounds.removeFromTop(m_visualizer.default_height()));

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

    HeaderComponent m_header;

    NoteView m_visualizer;

    SliderObject<float> m_internal_onset;
    SliderObject<float> m_internal_duration;
    SliderObject<int> m_internal_pitch;
    SliderObject<int> m_internal_velocity;
    SliderObject<int> m_internal_channel;

//    ToggleButtonObject m_enable_button;
//    juce::Label m_label;
};


#endif //SERIALISTLOOPER_MIDI_NOTE_SOURCE_COMPONENT_H
