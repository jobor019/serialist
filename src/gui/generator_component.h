

#ifndef SERIALIST_LOOPER_GENERATOR_COMPONENT_H
#define SERIALIST_LOOPER_GENERATOR_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "../generator.h"
#include "mapping_component.h"

template<typename T>
class GeneratorComponent : public juce::Component
                        , private juce::Slider::Listener {
public:
    explicit GeneratorComponent(std::shared_ptr<Generator<T> > generator, const std::string& name)
            : m_generator(std::move(generator))
              , m_name("name", name)
              , m_mapping_component(generator) {
        addAndMakeVisible(m_name);

        m_step_size.setRange(-8, 8, 0.1);
        m_step_size.setValue(1.0);
        m_step_size.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        m_step_size.addListener(this);
        addAndMakeVisible(m_step_size);

        m_mul.setRange(0, 10, 0.1);
        m_mul.setValue(1.0);
        m_mul.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        m_mul.addListener(this);
        addAndMakeVisible(m_mul);

        addAndMakeVisible(m_mapping_component);

    }


private:

    void sliderValueChanged(juce::Slider* slider) override {
        if (slider == &m_step_size) {
            m_generator->set_step_size(slider->getValue());
            std::cout << "changed step size to " << slider->getValue() << "\n";
        } else if (slider == &m_mul) {
            std::cout << "changed mul to " << slider->getValue() << "\n";
            m_generator->set_mul(slider->getValue());
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
        m_mul.setBounds(bounds.removeFromLeft(100));
        m_mapping_component.setBounds(bounds);
    }

    std::shared_ptr<Generator<T>> m_generator;

    juce::Slider m_step_size;
    juce::Slider m_mul;
    juce::Label m_name;

    TempInterpMappingComponent<T> m_mapping_component;

};

#endif //SERIALIST_LOOPER_GENERATOR_COMPONENT_H
