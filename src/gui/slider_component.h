

#ifndef SERIALISTLOOPER_SLIDER_COMPONENT_H
#define SERIALISTLOOPER_SLIDER_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "node_component.h"
#include "variable.h"


template<typename T>
class SliderComponent : public NodeComponent
                        , private juce::ValueTree::Listener {
public:

    static const int DEFAULT_SLIDER_HEIGHT_PX = 14;

    enum LabelPosition {
        left = 0
        , bottom = 1
    };


    SliderComponent(const std::string& identifier
                    , ParameterHandler& parent
                    , T min = static_cast<T>(0)
                    , T max = static_cast<T>(127)
                    , T step = static_cast<T>(1)
                    , T initial = static_cast<T>(0)
                    , const juce::String& label = ""
                    , LabelPosition label_position = LabelPosition::bottom)
            : m_variable(static_cast<T>(initial), identifier, parent)
              , m_label({}, label)
              , m_label_position(label_position) {
        static_assert(std::is_arithmetic_v<T>, "T must be arithmetic");

        m_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_slider.onValueChange = [this]() { on_slider_value_change(); };
        m_slider.setRange(min, max, step);
        m_slider.setValue(static_cast<double>(initial), juce::dontSendNotification);
        m_slider.setTextBoxIsEditable(false);
        if (!std::is_integral_v<T>) {
            m_slider.setNumDecimalPlacesToDisplay(2);
        }

        addAndMakeVisible(m_slider);

        if (m_label_position == LabelPosition::bottom)
            m_label.setJustificationType(juce::Justification::centred);
        else
            m_label.setJustificationType(juce::Justification::left);

        addAndMakeVisible(m_label);

        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~SliderComponent() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    SliderComponent(const SliderComponent&) = delete;
    SliderComponent& operator=(const SliderComponent&) = delete;
    SliderComponent(SliderComponent&&) noexcept = default;
    SliderComponent& operator=(SliderComponent&&) noexcept = default;


    int default_height() {
        if (m_label.getText().isEmpty())
            return DEFAULT_SLIDER_HEIGHT_PX;

        if (m_label_position == LabelPosition::bottom) {
            return static_cast<int>(DEFAULT_SLIDER_HEIGHT_PX
                                    + getLookAndFeel().getLabelFont(m_label).getHeight() + 2.0);
        } else {
            return DEFAULT_SLIDER_HEIGHT_PX;
        }
    }


    void set_range(T min, T max, T step = static_cast<T>(1)) {
        m_slider.setRange(static_cast<double>(min), static_cast<double>(max), static_cast<double>(step));
    }


    void set_label_position(LabelPosition label_position) {
        m_label_position = label_position;
        resized();
    }


    Generative& get_generative() override { return m_variable; }


    void paint(juce::Graphics&) override {}


    void resized() override {
        auto bounds = getLocalBounds();

        if (m_label.getText().isNotEmpty()) {

            if (m_label_position == LabelPosition::bottom) {
                m_label.setBounds(bounds.removeFromBottom(static_cast<int>(getLookAndFeel()
                                                                                   .getLabelFont(m_label)
                                                                                   .getHeight() + 2.0f)));
            } else {
                m_label.setBounds(bounds.removeFromLeft(
                        static_cast<int>(getLookAndFeel()
                                                 .getLabelFont(m_label)
                                                 .getStringWidth(m_label.getText()) + 2.0f)));
            }
        }
        m_slider.setBounds(bounds);
    }


private:
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            m_slider.setValue(m_variable.get_value(), juce::dontSendNotification);
        }
    }


    template<typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
    void on_slider_value_change() {
        m_variable.set_value(static_cast<T>(std::round(m_slider.getValue())));
    }


    template<typename U = T, std::enable_if_t<!std::is_integral_v<U>, int> = 0>
    void on_slider_value_change() {
        m_variable.set_value(static_cast<T>(m_slider.getValue()));
    }


    Variable<T> m_variable;

    juce::Label m_label;
    LabelPosition m_label_position;

    juce::Slider m_slider;


};

#endif //SERIALISTLOOPER_SLIDER_COMPONENT_H
