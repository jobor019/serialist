

#ifndef SERIALISTLOOPER_TOGGLE_BUTTON_COMPONENT_H
#define SERIALISTLOOPER_TOGGLE_BUTTON_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "node_component.h"
#include "variable.h"

class ToggleButtonComponent : public NodeComponent
                              , private juce::ValueTree::Listener {
public:
    ToggleButtonComponent(const std::string& identifier
                          , ParameterHandler& parent
                          , bool initial = false)
            : m_variable(initial, identifier, parent) {

        m_button.onStateChange = [this]() { on_value_change(); };
        m_button.setToggleState(initial, juce::dontSendNotification);

        addAndMakeVisible(m_button);


        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~ToggleButtonComponent() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }



    Generative& get_generative() override { return m_variable; }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_button.setBounds(getLocalBounds());
    }


private:
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            m_button.setToggleState(m_variable.get_value(), juce::dontSendNotification);
        }
    }


    void on_value_change() {
        std::cout << "NEW VALUE::: " << m_button.getToggleState() << "\n";
        m_variable.set_value(m_button.getToggleState());
    }


    Variable<bool> m_variable;

    juce::ToggleButton m_button;


};

#endif //SERIALISTLOOPER_TOGGLE_BUTTON_COMPONENT_H
