

#ifndef SERIALIST_LOOPER_LOOPER_COMPONENT_H
#define SERIALIST_LOOPER_LOOPER_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "../looper.h"

template<typename T>
class LooperComponent : public juce::Component
                        , private juce::Slider::Listener {
public:
    explicit LooperComponent(std::shared_ptr<Looper<T>> looper, const std::string& name)
            : m_looper(looper)
            , m_name("name", name) {
        addAndMakeVisible(m_name);

        m_step_size.setRange(-8, 8, 0.1);
        m_step_size.setValue(1.0);
        m_step_size.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        m_step_size.addListener(this);

        addAndMakeVisible(m_step_size);

    }

private:

    void sliderValueChanged(juce::Slider* slider) override {
        if (slider == &m_step_size) {
            m_looper->set_step_size(slider->getValue());
            std::cout << "changed step size to " << slider->getValue() << "\n";
        }
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

//        g.setFont(juce::Font(16.0f));
//        g.setColour(juce::Colours::white);
//        g.drawText(m_name, getLocalBounds(), juce::Justification::centred, true);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        m_name.setBounds(bounds.removeFromLeft(100));
        m_step_size.setBounds(bounds.removeFromLeft(100));
    }

    std::shared_ptr<Looper<T>> m_looper;

    juce::Slider m_step_size;
    juce::Label m_name;
};

#endif //SERIALIST_LOOPER_LOOPER_COMPONENT_H
