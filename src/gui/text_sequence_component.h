

#ifndef SERIALISTLOOPER_TEXT_SEQUENCE_COMPONENT_H
#define SERIALISTLOOPER_TEXT_SEQUENCE_COMPONENT_H

#include "node_component.h"
#include "sequence.h"

template<typename T>
class TextSequenceComponent : public NodeComponent
                              , private juce::Label::Listener {
public:

    TextSequenceComponent(const std::string& id, ParameterHandler& parent)
            : m_sequence(id, parent) {
        value_input.setText(format_values(), juce::dontSendNotification);
        value_input.setEditable(true);
        value_input.addListener(this);
        addAndMakeVisible(value_input);

        value_input_success.setClickingTogglesState(false);
        value_input_success.setToggleState(false, juce::dontSendNotification);
        addAndMakeVisible(value_input_success);

    }


    Generative& get_generative() override {
        return m_sequence;
    }


private:
    void labelTextChanged(juce::Label* labelThatHasChanged) override {
        auto [rc, values] = parse_input(labelThatHasChanged->getText().toStdString());
        value_input_success.setToggleState(rc, juce::dontSendNotification);
        if (rc) {
            m_sequence.get_parameter_obj().reset_values(values);
            std::cout << "New values: ";
            for (auto& value: values) {
                std::cout << value << " ";
            }
            std::cout << "\n";
        } else {
            std::cout << "FAILED INPUT\n";
        }
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
        g.drawRect(value_input.getBounds(), 2);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(8);
        value_input_success.setBounds(bounds.removeFromRight(20));
        bounds.removeFromRight(4);
        value_input.setBounds(bounds);

    }


    std::pair<bool, std::vector<T>> parse_input(const std::string& input) {
        std::istringstream iss(input);
        std::vector<T> v;
        T d;
        while (iss >> d) {
            v.push_back(d);
        }
        if (!iss.eof()) {
            return {false, {}};
        }
        return {true, v};
    }


    std::string format_values() {
        std::stringstream ss;
        std::vector<T> values = m_sequence.get_parameter_obj().clone_values();
        for (auto& value: values) {
            ss << value << " ";
        }
        return ss.str();
    }


    Sequence<T> m_sequence;

    juce::Label value_input;
    juce::ToggleButton value_input_success;


};

#endif //SERIALISTLOOPER_TEXT_SEQUENCE_COMPONENT_H
