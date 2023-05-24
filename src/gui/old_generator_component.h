

#ifndef SERIALIST_LOOPER_GENERATOR_COMPONENT_H
#define SERIALIST_LOOPER_GENERATOR_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "../generator.h"
#include "mapping_component.h"
#include "oscillator_component.h"

template<typename T>
class OldGeneratorComponent : public juce::Component
                              , private juce::Slider::Listener {
public:
    explicit OldGeneratorComponent(Generator<T>* generator, const std::string& name)
            : m_generator(generator)
              , m_name("name", name)
              , m_mapping(generator)
              , m_oscillator(generator) {
        addAndMakeVisible(m_name);

        m_step_size.setRange(-8, 8, 0.1);
        m_step_size.setValue(m_generator->get_step_size(), juce::dontSendNotification);
        m_step_size.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        m_step_size.addListener(this);
        addAndMakeVisible(m_step_size);

//        m_mul.setRange(0, 10, 0.1);
//        m_mul.setValue(1.0);
//        m_mul.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
//        m_mul.addListener(this);
//        addAndMakeVisible(m_mul);

        addAndMakeVisible(m_mapping);
        addAndMakeVisible(m_oscillator);
    }


private:
    void sliderValueChanged(juce::Slider* slider) override {
        if (slider == &m_step_size) {
            m_generator->set_step_size(slider->getValue());
            std::cout << "changed step size to " << slider->getValue() << "\n";
        } else if (slider == &m_mul) {
//            std::cout << "changed mul to " << slider->getValue() << "\n";
            std::cout << "Mul not implemented yet\n";
//            m_generator->get(slider->getValue());
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
//        m_mul.setBounds(bounds.removeFromLeft(100));
        m_oscillator.setBounds(bounds.removeFromRight(100));
        m_mapping.setBounds(bounds);
    }

    Generator<T>* m_generator;

    juce::Slider m_step_size;
    juce::Slider m_mul;
    juce::Slider m_add;
    juce::Label m_name;

    SingleMappingComponent<T> m_mapping;
    OscillatorComponent<T> m_oscillator;


};

#endif //SERIALIST_LOOPER_GENERATOR_COMPONENT_H
