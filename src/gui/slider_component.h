

#ifndef SERIALISTLOOPER_SLIDER_COMPONENT_H
#define SERIALISTLOOPER_SLIDER_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "node_component.h"
#include "variable.h"


template<typename T>
class SliderComponent : public NodeComponent
                        , private juce::ValueTree::Listener {
public:
    SliderComponent(const std::string& identifier
                    , ParameterHandler& parent
                    , T min = static_cast<T>(0)
                    , T max = static_cast<T>(127)
                    , T step = static_cast<T>(1)
                    , T initial = static_cast<T>(0))
            : m_variable(static_cast<T>(initial), identifier, parent) {
        static_assert(std::is_arithmetic_v<T>, "T must be arithmetic");

        m_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
        m_slider.onValueChange = [this](){ on_slider_value_change(); };
        m_slider.setRange(min, max, step);
        m_slider.setValue(static_cast<double>(initial), juce::dontSendNotification);
        m_slider.setTextBoxIsEditable(false);
        if (!std::is_integral_v<T>) {
            m_slider.setNumDecimalPlacesToDisplay(2);
        }

        addAndMakeVisible(m_slider);


        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~SliderComponent() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    SliderComponent(const SliderComponent&) = delete;
    SliderComponent& operator=(const SliderComponent&) = delete;
    SliderComponent(SliderComponent&&) noexcept = default;
    SliderComponent& operator=(SliderComponent&&) noexcept = default;

    void set_range(T min, T max, T step = static_cast<T>(1)) {
        m_slider.setRange(static_cast<double>(min), static_cast<double>(max), static_cast<double>(step));
    }


    Generative& get_generative() override { return m_variable; }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_slider.setBounds(getLocalBounds());
    }


private:
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            m_slider.setValue(m_variable.get_value(), juce::dontSendNotification);
        }
    }


    template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
    void on_slider_value_change() {
        m_variable.set_value(static_cast<T>(std::round(m_slider.getValue())));
    }

    template <typename U = T, std::enable_if_t<!std::is_integral_v<U>, int> = 0>
    void on_slider_value_change() {
        m_variable.set_value(static_cast<T>(m_slider.getValue()));
    }


    Variable<T> m_variable;

    juce::Slider m_slider;


};

#endif //SERIALISTLOOPER_SLIDER_COMPONENT_H
