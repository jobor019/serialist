

#ifndef SERIALISTLOOPER_SLIDER_OBJECT_H
#define SERIALISTLOOPER_SLIDER_OBJECT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "node_component.h"
#include "variable.h"
#include "look_and_feel.h"


template<typename T>
class SliderObject : public GenerativeComponent
                     , private juce::ValueTree::Listener {
public:


    enum LabelPosition {
        left = 0
        , bottom = 1
    };


    SliderObject(const std::string& identifier
                 , ParameterHandler& parent
                 , T min = static_cast<T>(0)
                 , T max = static_cast<T>(127)
                 , T step = static_cast<T>(1)
                 , T initial = static_cast<T>(0)
                 , const juce::String& label = ""
                 , const int label_width = DimensionConstants::DEFAULT_LABEL_WIDTH
                 , const LabelPosition position = LabelPosition::bottom)
            : m_variable(static_cast<T>(initial), identifier, parent)
              , m_label({}, label)
              , m_label_width(label_width)
              , m_label_position(position) {
        static_assert(std::is_arithmetic_v<T>, "DataType must be arithmetic");

        m_label.setColour(juce::Label::textColourId, getLookAndFeel().findColour(Colors::bright_text_color));

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


    ~SliderObject() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    SliderObject(const SliderObject&) = delete;
    SliderObject& operator=(const SliderObject&) = delete;
    SliderObject(SliderObject&&) noexcept = default;
    SliderObject& operator=(SliderObject&&) noexcept = default;


    std::pair<int, int> dimensions() override {
        return {default_width(), default_height()};
    }


    int default_width() const {
        if (m_label.getText().isEmpty())
            return DimensionConstants::SLIDER_DEFAULT_WIDTH;

        if (m_label_position == LabelPosition::bottom) {
            return DimensionConstants::SLIDER_DEFAULT_WIDTH;
        } else {
            return DimensionConstants::SLIDER_DEFAULT_WIDTH + m_label_width;
        }
    }


    int default_height() const {
        if (m_label.getText().isEmpty())
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT;

        if (m_label_position == LabelPosition::bottom) {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT
                   + DimensionConstants::FONT_HEIGHT
                   + DimensionConstants::LABEL_BELOW_MARGINS;
        } else {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT;
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
                m_label.setJustificationType(juce::Justification::centred);
                m_label.setBounds(bounds.removeFromBottom(DimensionConstants::FONT_HEIGHT));
                bounds.removeFromBottom(DimensionConstants::LABEL_BELOW_MARGINS);

            } else {
                m_label.setJustificationType(juce::Justification::left);
                m_label.setBounds(bounds.removeFromLeft(m_label_width));
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
    int m_label_width;
    LabelPosition m_label_position;

    juce::Slider m_slider;


};

#endif //SERIALISTLOOPER_SLIDER_OBJECT_H
