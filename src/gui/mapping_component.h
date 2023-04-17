

#ifndef SERIALIST_LOOPER_MAPPING_COMPONENT_H
#define SERIALIST_LOOPER_MAPPING_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "../looper.h"
#include "../mapping.h"


template<typename T>
class SingleMappingComponent : public juce::Component
                               , private juce::Label::Listener {
public:
    explicit SingleMappingComponent(Generator<T>* generator) : m_generator(generator) {
        value_input.setText(format_values(), juce::dontSendNotification);
        value_input.setEditable(true);
        value_input.addListener(this);
        addAndMakeVisible(value_input);

        value_input_success.setClickingTogglesState(false);
        value_input_success.setToggleState(false, juce::dontSendNotification);
        addAndMakeVisible(value_input_success);
    }


private:
    void labelTextChanged(juce::Label* labelThatHasChanged) override {
        auto [rc, values] = parse_input(labelThatHasChanged->getText().toStdString());
        value_input_success.setToggleState(rc, juce::dontSendNotification);
        if (rc) {
            m_generator->set_mapping(std::make_unique<Mapping<T>>(values));
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
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    void resized() override {
        auto bounds = getLocalBounds();
        value_input_success.setBounds(bounds.removeFromRight(20));
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
        for (auto& value: m_generator->get_mapping()->get_values()) {
            if (value.empty())
                ss << "X ";
            else
                ss << value.at(0) << " ";
        }
        return ss.str();
    }


    Generator<T>* m_generator;

    juce::Label value_input;
    juce::ToggleButton value_input_success;


};


#endif //SERIALIST_LOOPER_MAPPING_COMPONENT_H
