

#ifndef SERIALIST_LOOPER_GENERATOR_COMPONENT_H
#define SERIALIST_LOOPER_GENERATOR_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "../generator.h"
#include "mapping_component.h"

template<typename T>
class GeneratorComponent : public juce::Component
                        , private juce::Slider::Listener {
public:
    explicit GeneratorComponent(std::shared_ptr<Looper<T>> looper, const std::string& name)
            : m_looper(looper)
              , m_name("name", name)
              , m_mapping_component(looper) {
        addAndMakeVisible(m_name);

        m_step_size.setRange(-8, 8, 0.1);
        m_step_size.setValue(1.0);
        m_step_size.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        m_step_size.addListener(this);

        addAndMakeVisible(m_step_size);

        addAndMakeVisible(m_mapping_component);

    }

#endif //SERIALIST_LOOPER_GENERATOR_COMPONENT_H
