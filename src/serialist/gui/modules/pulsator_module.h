
#ifndef SERIALISTLOOPER_PULSATOR_MODULE_H
#define SERIALISTLOOPER_PULSATOR_MODULE_H

#include "bases/module_stereotypes.h"
#include "slider_widget_LEGACY.h"
#include "core/generatives/auto_pulsator.h"
#include "socket_widget.h"
#include "header_widget.h"

class PulsatorModule : public NodeModuleBase<TriggerEvent> {
public:

    using SliderLayout = SliderWidget::Layout;

    enum class Layout {
        full
        , note_source_internal
    };


    PulsatorModule(Pulsator& pulsator
                   , Variable<Facet, float>& internal_trigger_interval
                   , Variable<Facet, float>& internal_duty_cycle
                   , Variable<Facet, bool>& internal_enabled
                   , Variable<Facet, float>& internal_num_voices
                   , Layout layout = Layout::full)
            : NodeBase<TriggerEvent>(pulsator, &internal_enabled, &internal_num_voices, layout == Layout::full)
              , m_interval_socket(pulsator.get_trigger_interval()
                                  , std::make_unique<SliderWidget>(internal_trigger_interval
                                                                   , 0.125, 4.0, 0.125, false
                                                                   , "interval", SliderLayout::label_left))
              , m_duty_socket(pulsator.get_duty_cycle()
                              , std::make_unique<SliderWidget>(internal_duty_cycle
                                                               , 0.1, 1.1, 0.1, false
                                                               , "dur", SliderWidget::Layout::label_left))
              , m_layout(layout) {

        addAndMakeVisible(m_interval_socket);
        addAndMakeVisible(m_duty_socket);
    }


    static int width_of(Layout layout = Layout::full) {
        if (layout == Layout::full) {
            return 2 * DC::COMPONENT_LR_MARGINS
                   + SliderWidget::default_width(SliderLayout::label_left, true);
        } else {
            return 2 * SliderWidget::default_width(SliderLayout::label_below, true)
                   + 1 * DC::OBJECT_X_MARGINS_ROW;
        }
    }


    static int height_of(Layout layout = Layout::full) {
        if (layout == Layout::full) {
            return HeaderWidget::height_of()
                   + 2 * DC::COMPONENT_UD_MARGINS
                     + 2 * SliderWidget::height_of(SliderWidget::Layout::label_left)
                   + DC::OBJECT_Y_MARGINS_COLUMN;

        } else {
            return SliderWidget::height_of(SliderWidget::Layout::label_left);
        }
    }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        resized();
    }


private:
    void on_resized(juce::Rectangle<int>& bounds) override {
        if (m_layout == Layout::full) {
            full_layout(bounds);
        } else if (m_layout == Layout::note_source_internal) {
            note_source_internal_layout(bounds);
        }
    }

    void full_layout(juce::Rectangle<int>& bounds) {
        auto slider_layout = SliderLayout::label_left;
        auto slider_height = SliderWidget::height_of(slider_layout);

        m_interval_socket.set_layout(static_cast<int>(slider_layout));
        m_duty_socket.set_layout(static_cast<int>(slider_layout));

        m_interval_socket.setBounds(bounds.removeFromTop(slider_height));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_duty_socket.setBounds(bounds.removeFromTop(slider_height));
    }

    void note_source_internal_layout(juce::Rectangle<int>& bounds) {
        auto slider_layout = SliderLayout::label_below;
//        auto slider_width = SliderWidget::default_width(slider_layout, true);
        auto slider_width = DC::SLIDER_COMPACT_WIDTH;

        m_interval_socket.set_layout(static_cast<int>(slider_layout));
        m_duty_socket.set_layout(static_cast<int>(slider_layout));

        m_interval_socket.setBounds(bounds.removeFromLeft(slider_width));
        bounds.removeFromLeft(DC::OBJECT_X_MARGINS_ROW);
        m_duty_socket.setBounds(bounds.removeFromLeft(slider_width));
    }

    SocketWidget<Facet> m_interval_socket;
    SocketWidget<Facet> m_duty_socket;

    Layout m_layout;

};

#endif //SERIALISTLOOPER_PULSATOR_MODULE_H
