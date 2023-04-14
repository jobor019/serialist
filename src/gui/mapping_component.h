

#ifndef SERIALIST_LOOPER_MAPPING_COMPONENT_H
#define SERIALIST_LOOPER_MAPPING_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "../looper.h"
#include "../mapping.h"


// TODO: Note: this is not a generalized implementation that would work for any type of mapping.
//  Right now, it will only work for Looper since the Looper needs to update its Phasor and we therefore cannot
//  set the values directly in the MultiMapping. For this, a better solution is required.
template<typename T>
class TempMappingComponent : public juce::Component
                             , private juce::Label::Listener {
public:
    explicit TempMappingComponent(std::shared_ptr<Looper<T>> looper) : m_looper(looper) {
        value_input.setText("input values", juce::dontSendNotification);
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
            m_looper->set_mapping(MultiMapping<T>(values));
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


    std::shared_ptr<Looper<T>> m_looper;

    juce::Label value_input;
    juce::ToggleButton value_input_success;


};


// ==============================================================================================

template<typename T>
class TempInterpMappingComponent : public juce::Component
                             , private juce::Label::Listener {
public:
    explicit TempInterpMappingComponent(std::shared_ptr<Generator<T>> generator) : m_generator(generator) {
        value_input.setText("input values", juce::dontSendNotification);
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
            m_generator->set_mapping(SingleMapping<T>(values));
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


    std::shared_ptr<Generator<T>> m_generator;

    juce::Label value_input;
    juce::ToggleButton value_input_success;


};

#endif //SERIALIST_LOOPER_MAPPING_COMPONENT_H
