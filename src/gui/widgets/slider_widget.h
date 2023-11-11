

#ifndef SERIALISTLOOPER_SLIDER_WIDGET_H
#define SERIALISTLOOPER_SLIDER_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "generative_component.h"
#include "core/generatives/variable.h"
#include "look_and_feel.h"

class SliderWidget : public GenerativeComponent
                     , private juce::ValueTree::Listener {
public:

    enum class Layout : int {
        label_left = 0
        , label_below = 1
    };


    explicit SliderWidget(Variable<Facet, float>& variable
                          , double min = 0.0f
                          , double max = 127.0f
                          , double step = 1.0f
                          , bool is_integral = false
                          , const juce::String& label = ""
                          , const Layout layout = Layout::label_below
                          , const int label_width = DimensionConstants::DEFAULT_LABEL_WIDTH)
            : m_variable(variable)
              , m_is_integral(is_integral)
              , m_label({}, label)
              , m_layout(layout)
              , m_label_width(label_width) {

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
    SliderWidget(SliderWidget&&) noexcept = delete;
    SliderWidget& operator=(SliderWidget&&) noexcept = delete;


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


    void initialize_slider(double min, double max, double step) {
        m_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_slider.onValueChange = [this]() { on_slider_value_change(); };
        m_slider.setRange(min, max, step);
        m_slider.setValue(static_cast<double>(m_variable.get_value()), juce::dontSendNotification);
        m_slider.setTextBoxIsEditable(false);
        if (!m_is_integral) {
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


    void on_slider_value_change() {
        if (m_is_integral)
            m_variable.try_set_value(static_cast<float>(std::round(m_slider.getValue())));
        else
            m_variable.try_set_value(static_cast<float>(m_slider.getValue()));
    }


    Variable<Facet, float>& m_variable;
    bool m_is_integral;

    juce::Label m_label;
    Layout m_layout;
    int m_label_width;


    juce::Slider m_slider;


};

#endif //SERIALISTLOOPER_SLIDER_WIDGET_H
