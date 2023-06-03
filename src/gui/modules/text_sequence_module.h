

#ifndef SERIALISTLOOPER_TEXT_SEQUENCE_MODULE_H
#define SERIALISTLOOPER_TEXT_SEQUENCE_MODULE_H

#include "generative_component.h"
#include "sequence.h"

template<typename T>
class TextSequenceModule : public GenerativeComponent
                           , private juce::Label::Listener {
public:

    enum class Layout {
        full = 0
        , generator_internal = 1
    };


    TextSequenceModule(const std::string& id, ParameterHandler& parent, Layout layout = Layout::full)
            : m_header(id, parent)
              , m_sequence(id, parent)
              , m_layout(layout) {

        addAndMakeVisible(m_header);

        value_input.setText(format_values(), juce::dontSendNotification);
        value_input.setEditable(true);
        value_input.addListener(this);
        addAndMakeVisible(value_input);

        value_input_success.setClickingTogglesState(false);
        value_input_success.setToggleState(false, juce::dontSendNotification);
        addAndMakeVisible(value_input_success);

    }


    static int width_of(Layout layout) {
        (void) layout;
        return 100;
    }

    static int height_of(Layout layout) {
        (void) layout;
        return 100;
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
        if (m_layout == Layout::full) {
            auto bounds = getLocalBounds();
            m_header.setBounds(bounds.removeFromTop(m_header.default_height()));
            bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);
            value_input_success.setBounds(bounds.removeFromRight(DimensionConstants::SLIDER_DEFAULT_HEIGHT));
            bounds.removeFromRight(DimensionConstants::OBJECT_X_MARGINS_ROW);
            value_input.setBounds(bounds);

        }
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


    HeaderWidget m_header;

    Sequence<T> m_sequence;

    Layout m_layout;

    juce::Label value_input;
    juce::ToggleButton value_input_success;


};

#endif //SERIALISTLOOPER_TEXT_SEQUENCE_MODULE_H
