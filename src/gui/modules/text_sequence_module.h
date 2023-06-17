

#ifndef SERIALISTLOOPER_TEXT_SEQUENCE_MODULE_H
#define SERIALISTLOOPER_TEXT_SEQUENCE_MODULE_H

#include "generative_component.h"
#include "sequence.h"

template<typename T>
class TextSequenceModule : public GenerativeComponent
                           , private juce::Label::Listener {
public:

    static const int INPUT_BOX_HEIGHT = static_cast<const int>(DC::SLIDER_DEFAULT_HEIGHT * 1.6);


    enum class Layout {
        full = 0
        , generator_internal = 1
    };


    explicit TextSequenceModule(Sequence<T>& sequence, Variable<bool>& enabled, Layout layout = Layout::full)
            : m_sequence(sequence)
              , m_header(sequence.get_identifier_as_string(), enabled)
              , m_layout(layout) {
        initialize_components();
    }


    static int width_of(Layout layout = Layout::full) {
        (void) layout;
        return 100;
    }


    static int height_of(Layout layout = Layout::full) {
        switch (layout) {
            case Layout::full:
                return HeaderWidget::height_of()
                       + 2 * DC::COMPONENT_UD_MARGINS
                       + INPUT_BOX_HEIGHT;
            case Layout::generator_internal:
                return INPUT_BOX_HEIGHT;
        }
    }


    Generative& get_generative() override { return m_sequence; }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        std::cout << "layout set to " << layout_id << "\n";
        resized();
    }


private:

    void initialize_components() {
        addAndMakeVisible(m_header);

        value_input.setText(format_values(), juce::dontSendNotification);
        value_input.setEditable(true);
        value_input.addListener(this);
        addAndMakeVisible(value_input);

        value_input_success.setClickingTogglesState(false);
        value_input_success.setToggleState(false, juce::dontSendNotification);
        addAndMakeVisible(value_input_success);
    }


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
        auto bounds = getLocalBounds();

        if (m_layout == Layout::full) {
            m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
            bounds.reduce(DC::COMPONENT_LR_MARGINS, DC::COMPONENT_UD_MARGINS);
        }

        value_input_success.setBounds(bounds.removeFromRight(bounds.getHeight()));
        bounds.removeFromRight(DC::OBJECT_X_MARGINS_ROW);
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


    Sequence<T>& m_sequence;

    HeaderWidget m_header;

    juce::Label value_input;
    juce::ToggleButton value_input_success;

    Layout m_layout;

};

#endif //SERIALISTLOOPER_TEXT_SEQUENCE_MODULE_H
