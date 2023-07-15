

#ifndef SERIALISTLOOPER_TOGGLE_BUTTON_WIDGET_H
#define SERIALISTLOOPER_TOGGLE_BUTTON_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "generative_component.h"
#include "variable.h"

class ToggleButtonWidget : public GenerativeComponent
                           , private juce::ValueTree::Listener {
public:
    explicit ToggleButtonWidget(Variable<bool>& variable
                                , const std::string& on_text = ""
                                , const std::string& off_text = "")
            : m_variable(variable)
              , m_on_text(on_text)
              , m_off_text(off_text) {

        setComponentID(variable.get_parameter_handler().get_id());
        initialize_button();

        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~ToggleButtonWidget() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    Generative& get_generative() override { return m_variable; }


    void set_layout(int) override { /* no layouts defined */ }


    static int height_of() { return DimensionConstants::SLIDER_DEFAULT_HEIGHT; }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_button.setBounds(getLocalBounds());
    }


    const juce::String& get_text() const {
        return m_button.getButtonText();
    }


private:
    void initialize_button() {
        bool is_on = m_variable.get_value();

        m_button.setToggleState(is_on, juce::dontSendNotification);
        m_button.setButtonText(is_on ? m_on_text : m_off_text);
        addAndMakeVisible(m_button);

        m_button.onStateChange = [this]() { on_value_change(); };
    }


    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            std::cout << "TREE    : " << treeWhosePropertyHasChanged.toXmlString() << std::endl;
            std::cout << "PROPERTY: " << property.toString() << std::endl;
            std::cout << "VALUE   : " << treeWhosePropertyHasChanged.getProperty(property).toString() << std::endl;
            m_button.setToggleState(m_variable.get_value(), juce::dontSendNotification);
        }
    }


    void on_value_change() {
        m_variable.set_value(m_button.getToggleState());
        m_button.setButtonText(m_button.getToggleState() ? m_on_text : m_off_text);
    }


    Variable<bool>& m_variable;

    juce::ToggleButton m_button;

    const juce::String m_on_text;
    const juce::String m_off_text;


};

#endif //SERIALISTLOOPER_TOGGLE_BUTTON_WIDGET_H
