

#ifndef SERIALISTLOOPER_SLIDER_WIDGET_H
#define SERIALISTLOOPER_SLIDER_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "generative_component.h"
#include "variable.h"
#include "look_and_feel.h"


template<typename T>
class SliderWidget : public GenerativeComponent
                     , private juce::ValueTree::Listener {
public:

    enum class Layout : int {
        label_left = 0
        , label_below = 1
    };


    explicit SliderWidget(Variable<T>& variable
                          , T min = static_cast<T>(0)
                          , T max = static_cast<T>(127)
                          , T step = static_cast<T>(1)
                          , const juce::String& label = ""
                          , const Layout layout = Layout::label_below
                          , const int label_width = DimensionConstants::DEFAULT_LABEL_WIDTH)
            : m_variable(variable)
              , m_label({}, label)
              , m_layout(layout)
              , m_label_width(label_width){

        static_assert(std::is_arithmetic_v<T>, "DataType must be arithmetic");

        setComponentID(variable.get_parameter_handler().get_id());

        initialize_slider(min, max, step);
        initialize_label();

        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~SliderWidget() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    SliderWidget(const SliderWidget&) = delete;
    SliderWidget& operator=(const SliderWidget&) = delete;
    SliderWidget(SliderWidget&&) noexcept = default;
    SliderWidget& operator=(SliderWidget&&) noexcept = default;


    static int height_of(Layout layout) {
        if (layout == Layout::label_below) {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT
                   + DimensionConstants::FONT_HEIGHT
                   + DimensionConstants::LABEL_BELOW_MARGINS;
        } else {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT;
        }
    }


    static int default_width(Layout layout, bool has_label) {
        if (!has_label)
            return DimensionConstants::SLIDER_DEFAULT_WIDTH;

        if (layout == Layout::label_below) {
            return DimensionConstants::SLIDER_DEFAULT_WIDTH;
        } else {
            return DimensionConstants::SLIDER_DEFAULT_WIDTH + DimensionConstants::DEFAULT_LABEL_WIDTH;
        }
    }


    Generative& get_generative() override { return m_variable; }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        resized();
    }


    void paint(juce::Graphics&) override {}


    void resized() override {
        auto bounds = getLocalBounds();

        if (m_label.getText().isNotEmpty()) {

            if (m_layout == Layout::label_below) {
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
    void initialize_label() {
        m_label.setColour(juce::Label::textColourId, getLookAndFeel().findColour(Colors::bright_text_color));
        addAndMakeVisible(m_label);

        if (m_layout == Layout::label_below)
            m_label.setJustificationType(juce::Justification::centred);
        else
            m_label.setJustificationType(juce::Justification::left);

    }


    void initialize_slider(T min, T max, T step) {
        m_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_slider.onValueChange = [this]() { on_slider_value_change(); };
        m_slider.setRange(min, max, step);
        m_slider.setValue(static_cast<double>(m_variable.get_value()), juce::dontSendNotification);
        m_slider.setTextBoxIsEditable(false);
        if (!std::is_integral_v<T>) {
            m_slider.setNumDecimalPlacesToDisplay(2);
        }

        addAndMakeVisible(m_slider);
    }


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


    Variable<T>& m_variable;

    juce::Label m_label;
    Layout m_layout;
    int m_label_width;

    juce::Slider m_slider;


};

#endif //SERIALISTLOOPER_SLIDER_WIDGET_H
