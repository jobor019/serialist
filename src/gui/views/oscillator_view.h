

#ifndef SERIALISTLOOPER_OSCILLATOR_VIEW_H
#define SERIALISTLOOPER_OSCILLATOR_VIEW_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "oscillator.h"

class OscillatorView : public juce::Component
                       , private juce::Timer {
public:


    enum class Layout {
        full = 0
        , compact = 1
    };


    explicit OscillatorView(Oscillator& oscillator, Layout layout = Layout::full)
            : m_oscillator(oscillator)
              , m_layout(layout) {
        addAndMakeVisible(m_view);
        m_view.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_view.setInterceptsMouseClicks(false, false);
        m_view.setRange(0, 10.0);
        m_view.setNumDecimalPlacesToDisplay(2);
        startTimer(25);
    }


    static int height_of(Layout layout) {
        if (layout == Layout::compact) {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT;
        } else {
            return 2 * DimensionConstants::SLIDER_DEFAULT_HEIGHT;
        }
    }

    void paint(juce::Graphics&) override {}


    void resized() override {
        m_view.setBounds(getLocalBounds());
    }


    void set_layout(Layout layout) {
        m_layout = layout;
        resized();
    }


private:
    void timerCallback() override {
        auto queue = m_oscillator.get_output_history();
        if (!queue.empty()) {
            m_view.setValue(queue.back(), juce::dontSendNotification);
        }
    }


    Oscillator& m_oscillator;

    Layout m_layout;

    juce::Slider m_view;

};


#endif //SERIALISTLOOPER_OSCILLATOR_VIEW_H
