
#ifndef SERIALISTLOOPER_PULSATOR_MODULE_H
#define SERIALISTLOOPER_PULSATOR_MODULE_H

#include "module_bases.h"
#include "slider_widget.h"
#include "pulsator.h"
#include "socket_widget.h"
#include "header_widget.h"

class PulsatorModule : public NodeBase<Trigger> {
public:

    using SliderLayout = SliderWidget::Layout;

    enum class Layout {
        full
        , note_source
    };


    PulsatorModule(Pulsator& pulsator
                   , Variable<Facet, float>& internal_trigger_interval
                   , Variable<Facet, float>& internal_duty_cycle
                   , Variable<Facet, bool>& internal_enabled
                   , Variable<Facet, float>& internal_num_voices
                   , Layout layout = Layout::full)
            : NodeBase<Trigger>(pulsator, &internal_enabled, &internal_num_voices)
              , m_interval_socket(pulsator.get_trigger_interval()
                                  , std::make_unique<SliderWidget>(internal_trigger_interval
                                                                   , 0.125, 4.0, 0.125, false
                                                                   , "interval", SliderLayout::label_below))
              , m_duty_socket(pulsator.get_duty_cycle()
                              , std::make_unique<SliderWidget>(internal_duty_cycle
                                                               , 0.1, 1.1, 0.1, false
                                                               , "dur", SliderWidget::Layout::label_below))
              , m_layout(layout) {

        addAndMakeVisible(m_interval_socket);
        addAndMakeVisible(m_duty_socket);
    }


    static int width_of(Layout layout = Layout::full) {
        (void) layout;
        return DimensionConstants::COMPONENT_LR_MARGINS
               + 2 * DimensionConstants::SLIDER_DEFAULT_WIDTH
               + 1 * DimensionConstants::OBJECT_X_MARGINS_ROW;
    }


    static int height_of(Layout layout = Layout::full) {
        (void) layout;

        return HeaderWidget::height_of()
               + 2 * DimensionConstants::COMPONENT_UD_MARGINS
               + DimensionConstants::OBJECT_Y_MARGINS_COLUMN
               + SliderWidget::height_of(SliderWidget::Layout::label_below);
    }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        resized();
    }


private:
    void on_resized(juce::Rectangle<int>& bounds) override {
        (void) bounds;
        if (m_layout == Layout::full) {
            throw std::runtime_error("Not implementedPulsator bounds"); // TODO

        }
    }

    SocketWidget<Facet> m_interval_socket;
    SocketWidget<Facet> m_duty_socket;

    Layout m_layout;

};

#endif //SERIALISTLOOPER_PULSATOR_MODULE_H
