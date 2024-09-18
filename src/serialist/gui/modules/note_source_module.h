

#ifndef SERIALISTLOOPER_NOTE_SOURCE_MODULE_H
#define SERIALISTLOOPER_NOTE_SOURCE_MODULE_H

#include "core/generatives/note_source_LEGACY.h"
#include "state/generative_component.h"
#include "midi_config.h"
#include "widgets/slider_widget_LEGACY.h"
#include "widgets/toggle_button_widget.h"
#include "widgets/header_widget.h"
#include "views/note_view.h"
#include "interaction_visualizations_LEGACY.h"
#include "pulsator_module.h"

namespace serialist {

class NoteSourceModule : public RootModuleBase {
public:

    enum class Layout {
        full = 0
    };


    NoteSourceModule(NoteSource& note_source
                     , std::unique_ptr<PulsatorModule> internal_trigger
                     , Variable<Facet, float>& internal_pitch
                     , Variable<Facet, float>& internal_velocity
                     , Variable<Facet, float>& internal_channel
                     , Variable<Facet, bool>& internal_enabled
                     , Variable<Facet, float>& internal_num_voices
                     , Layout layout = Layout::full)
            : RootModuleBase(note_source, &internal_enabled, &internal_num_voices)
              , m_internal_trigger(note_source.get_trigger_pulse(), std::move(internal_trigger))
              , m_internal_pitch(note_source.get_pitch(), std::make_unique<SliderWidget>(
                    internal_pitch, 2100, 10800, 100, true, "pitch", SliderWidget::Layout::label_below))
              , m_internal_velocity(note_source.get_velocity(), std::make_unique<SliderWidget>(
                    internal_velocity, 0, 127, 1, true, "vel", SliderWidget::Layout::label_below))
              , m_internal_channel(note_source.get_channel(), std::make_unique<SliderWidget>(
                    internal_channel, 1, 16, 1, true, "ch", SliderWidget::Layout::label_below))
              , m_visualizer(note_source) {
        (void) layout;

        addAndMakeVisible(m_internal_trigger);
        addAndMakeVisible(m_internal_pitch);
        addAndMakeVisible(m_internal_velocity);
        addAndMakeVisible(m_internal_channel);

        addAndMakeVisible(m_visualizer);
    }


    static int width_of(Layout layout = Layout::full) {
        (void) layout;

        return DC::COMPONENT_LR_MARGINS
               + 5 * DC::SLIDER_COMPACT_WIDTH
               + 4 * DC::OBJECT_X_MARGINS_ROW;
    }


    static int height_of(Layout layout = Layout::full) {
        (void) layout;

        return HeaderWidget::height_of()
               + NoteView::height_of()
               + 2 * DC::COMPONENT_UD_MARGINS
               + DC::OBJECT_Y_MARGINS_COLUMN
               + SliderWidget::height_of(SliderWidget::Layout::label_below);
    }


    void set_layout(int) override {}


    void on_resized(juce::Rectangle<int>& bounds) override {
        m_visualizer.setBounds(bounds.removeFromTop(NoteView::height_of()));

        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);

        m_internal_trigger.setBounds(bounds.removeFromLeft(2 * DC::SLIDER_COMPACT_WIDTH + DC::OBJECT_X_MARGINS_ROW));
        bounds.removeFromLeft(DC::OBJECT_X_MARGINS_ROW);
        m_internal_pitch.setBounds(bounds.removeFromLeft(DC::SLIDER_COMPACT_WIDTH));
        bounds.removeFromLeft(DC::OBJECT_X_MARGINS_ROW);
        m_internal_velocity.setBounds(bounds.removeFromLeft(DC::SLIDER_COMPACT_WIDTH));
        bounds.removeFromLeft(DC::OBJECT_X_MARGINS_ROW);
        m_internal_channel.setBounds(bounds.removeFromLeft(DC::SLIDER_COMPACT_WIDTH));

    }


private:
    SocketWidget<TriggerEvent> m_internal_trigger;

    SocketWidget<Facet> m_internal_pitch;
    SocketWidget<Facet> m_internal_velocity;
    SocketWidget<Facet> m_internal_channel;

    NoteView m_visualizer;

};

namespace serialist {


#endif //SERIALISTLOOPER_NOTE_SOURCE_MODULE_H
