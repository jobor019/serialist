

#ifndef SERIALISTLOOPER_TOGGLE_BUTTON_OBJECT_H
#define SERIALISTLOOPER_TOGGLE_BUTTON_OBJECT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "node_component.h"
#include "variable.h"

class ToggleButtonObject : public GenerativeComponent
                           , private juce::ValueTree::Listener {
public:
    ToggleButtonObject(const std::string& identifier
                       , ParameterHandler& parent
                       , bool initial = false
                       , const std::string& on_text = ""
                       , const std::string& off_text = "")
            : m_variable(initial, identifier, parent)
              , m_on_text(on_text)
              , m_off_text(off_text) {

        m_button.onStateChange = [this]() { on_value_change(); };

        m_button.setToggleState(initial, juce::dontSendNotification);
        m_button.setButtonText(m_button.getToggleState() ? m_on_text : m_off_text);

        addAndMakeVisible(m_button);


        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~ToggleButtonObject() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    Generative& get_generative() override { return m_variable; }


    std::pair<int, int> dimensions() override {
        if (m_on_text.isEmpty() && m_off_text.isEmpty()) {
            return {DimensionConstants::SLIDER_DEFAULT_HEIGHT, DimensionConstants::SLIDER_DEFAULT_HEIGHT};
        }
        return {DimensionConstants::DEFAULT_LABEL_WIDTH, DimensionConstants::SLIDER_DEFAULT_HEIGHT};
    }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_button.setBounds(getLocalBounds());
    }


    const juce::String& get_text() const {
        return m_button.getButtonText();
    }


private:
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            m_button.setToggleState(m_variable.get_value(), juce::dontSendNotification);
        }
    }


    void on_value_change() {
        m_variable.set_value(m_button.getToggleState());
        m_button.setButtonText(m_button.getToggleState() ? m_on_text : m_off_text);
    }


    Variable<bool> m_variable;

    juce::ToggleButton m_button;

    const juce::String m_on_text;
    const juce::String m_off_text;


};

#endif //SERIALISTLOOPER_TOGGLE_BUTTON_OBJECT_H
