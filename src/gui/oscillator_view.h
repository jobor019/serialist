

#ifndef SERIALISTLOOPER_OSCILLATOR_VIEW_H
#define SERIALISTLOOPER_OSCILLATOR_VIEW_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "oscillator.h"

class OscillatorView : public juce::Component
//                       , public Dimensioned
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

//    std::pair<int, int> dimensions() override {
//        int height;
//        if (m_layout == Layout::compact) {
//            height = DimensionConstants::SLIDER_DEFAULT_HEIGHT;
//        } else {
//            height =
//        }
//    }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_view.setBounds(getLocalBounds());
    }


    int default_height() const {
        if (m_layout == Layout::full)
            return 22;
        else
            return 14;
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
