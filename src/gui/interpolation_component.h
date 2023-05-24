

#ifndef SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H
#define SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "interpolator.h"
#include "variable.h"
#include "node_component.h"

template<typename T>
class InterpolationStrategyComponent : public NodeComponent
        , juce::Slider::Listener
        , juce::ComboBox::Listener {
public:

    InterpolationStrategyComponent(const std::string& id, ParameterHandler& parent)
            : m_strategy(InterpolationStrategy<T>(), id, parent) {

        m_type.addItem("continuation", static_cast<int>(InterpolationStrategy<T>::Type::continuation));
        m_type.addItem("modulo", static_cast<int>(InterpolationStrategy<T>::Type::modulo));
        m_type.addItem("clip", static_cast<int>(InterpolationStrategy<T>::Type::clip));
        m_type.addItem("pass", static_cast<int>(InterpolationStrategy<T>::Type::pass));
        m_type.setSelectedId(static_cast<int>(m_strategy.get_value().get_type()), juce::dontSendNotification);
        m_type.addListener(this);
        addAndMakeVisible(m_type);

        m_pivot.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
        m_pivot.setValue(m_strategy.get_value().get_pivot(), juce::dontSendNotification);
        m_pivot.setNumDecimalPlacesToDisplay(std::is_integral_v<T> ? 0 : 2);
        m_pivot.addListener(this);
        addAndMakeVisible(m_pivot);

    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
    }


    void resized() override {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft(5);
        m_pivot.setBounds(bounds.removeFromLeft(60));
        bounds.removeFromLeft(5);
        bounds.removeFromRight(5);
        m_type.setBounds(bounds);
    }


    Generative& get_generative() override {
        return m_strategy;
    }


private:

    void sliderValueChanged(juce::Slider*) override {
        value_changed();
    }

    void comboBoxChanged(juce::ComboBox*) override {
        value_changed();
    }


    void value_changed() {
        typename InterpolationStrategy<T>::Type type{m_type.getSelectedId()};
        std::cout << "new value: " << static_cast<int>(type) << ":" << static_cast<T>(m_pivot.getValue()) << "\n";
        m_strategy.set_value(InterpolationStrategy<T>(type, static_cast<T>(m_pivot.getValue())));
    }


    Variable<InterpolationStrategy<T>> m_strategy;

    juce::ComboBox m_type;
    juce::Slider m_pivot;


};


#endif //SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H
