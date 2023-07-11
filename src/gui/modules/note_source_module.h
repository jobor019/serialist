

#ifndef SERIALISTLOOPER_NOTE_SOURCE_MODULE_H
#define SERIALISTLOOPER_NOTE_SOURCE_MODULE_H

#include "source.h"
#include "generative_component.h"
#include "midi_config.h"
#include "widgets/slider_widget.h"
#include "widgets/toggle_button_widget.h"
#include "widgets/header_widget.h"
#include "views/note_view.h"
#include "interaction_visualizations.h"

class NoteSourceModule : public GenerativeComponent {
public:

    static const int SLIDER_WIDTH = static_cast<int>(DimensionConstants::SLIDER_DEFAULT_WIDTH * 0.8);

    enum class Layout {
        full = 0
    };


    NoteSourceModule(MidiNoteSource& note_source
                     , Variable<float>& internal_onset
                     , Variable<float>& internal_duration
                     , Variable<int>& internal_pitch
                     , Variable<int>& internal_velocity
                     , Variable<int>& internal_channel
                     , Variable<bool>& internal_enabled
                     , Layout layout = Layout::full)
            : m_midi_source(note_source)
              , m_internal_onset(internal_onset, 0.125f, 4.0f, 0.125f
                                 , "onset", SliderWidget<float>::Layout::label_below)
              , m_internal_duration(internal_duration, 0.1f, 1.1f, 0.1f
                                    , "dur", SliderWidget<float>::Layout::label_below)
              , m_internal_pitch(internal_pitch, 2100, 10800, 100
                                 , "pitch", SliderWidget<int>::Layout::label_below)
              , m_internal_velocity(internal_velocity, 0, 127, 1
                                    , "vel", SliderWidget<int>::Layout::label_below)
              , m_internal_channel(internal_channel, 1, 16, 1
                                   , "ch", SliderWidget<int>::Layout::label_below)
              , m_header(note_source.get_identifier_as_string(), internal_enabled)
              , m_visualizer(m_midi_source)
              , m_highlight_manager(*this, &m_edit_state, ModuleEditState::default_module_highlights()) {
        (void) layout;

        addAndMakeVisible(m_header);
        addAndMakeVisible(m_visualizer);
        addAndMakeVisible(m_internal_onset);
        addAndMakeVisible(m_internal_duration);
        addAndMakeVisible(m_internal_pitch);
        addAndMakeVisible(m_internal_velocity);
        addAndMakeVisible(m_internal_channel);

        addAndMakeVisible(m_highlight_manager);
    }


    static int width_of(Layout layout = Layout::full) {
        (void) layout;

        return DimensionConstants::COMPONENT_LR_MARGINS
               + 5 * SLIDER_WIDTH
               + 4 * DimensionConstants::OBJECT_X_MARGINS_ROW;
    }


    static int height_of(Layout layout = Layout::full) {
        (void) layout;

        return HeaderWidget::height_of()
               + NoteView::height_of()
               + 2 * DimensionConstants::COMPONENT_UD_MARGINS
               + DimensionConstants::OBJECT_Y_MARGINS_COLUMN
               + SliderWidget<float>::height_of(SliderWidget<float>::Layout::label_below);
    }


    static std::string default_name() {
        return "source";
    }


    void set_layout(int) override {}


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

        m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
        bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);

        m_visualizer.setBounds(bounds.removeFromTop(NoteView::height_of()));

        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        m_internal_onset.setBounds(bounds.removeFromLeft(SLIDER_WIDTH));
        bounds.removeFromLeft(DimensionConstants::OBJECT_X_MARGINS_ROW);
        m_internal_duration.setBounds(bounds.removeFromLeft(SLIDER_WIDTH));
        bounds.removeFromLeft(DimensionConstants::OBJECT_X_MARGINS_ROW);
        m_internal_pitch.setBounds(bounds.removeFromLeft(SLIDER_WIDTH));
        bounds.removeFromLeft(DimensionConstants::OBJECT_X_MARGINS_ROW);
        m_internal_velocity.setBounds(bounds.removeFromLeft(SLIDER_WIDTH));
        bounds.removeFromLeft(DimensionConstants::OBJECT_X_MARGINS_ROW);
        m_internal_channel.setBounds(bounds.removeFromLeft(SLIDER_WIDTH));

        m_highlight_manager.setBounds(getLocalBounds());
    }


private:

    MidiNoteSource& m_midi_source;

    SliderWidget<float> m_internal_onset;
    SliderWidget<float> m_internal_duration;
    SliderWidget<int> m_internal_pitch;
    SliderWidget<int> m_internal_velocity;
    SliderWidget<int> m_internal_channel;

    HeaderWidget m_header;

    NoteView m_visualizer;

    ModuleEditState m_edit_state;
    EditHighlightManager m_highlight_manager;

};


#endif //SERIALISTLOOPER_NOTE_SOURCE_MODULE_H
