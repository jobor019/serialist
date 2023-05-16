#ifndef SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
#define SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "../new_oscillator.h" // TODO: Why can't this be relative?

class OscillatorComponent : public NodeComponent
                            , public juce::Timer {
public:
    enum class Layout {
        full
        , compact
        , internal
    };


    OscillatorComponent(const std::string& id, ParameterHandler& parent)
            : m_oscillator(id, parent)
              , m_internal_freq(id + "::freq", parent, 0.0f, 10.0f, 0.125f, 0.5f)
              , m_internal_mul(id + "::mul", parent, 0.0f, 10.0f, 0.125f, 1.0)
              , m_internal_add(id + "::add", parent, 0.0f, 10.0f, 0.125f, 0.0f)
              , m_internal_duty(id + "::duty", parent, 0.0f, 1.0f, 0.01f, 0.5f)
              , m_internal_curve(id + "::curve", parent, 0.0f, 0.0f, 0.0f, 0.0f)
              , m_enable_button(id + "::enabled", parent, true) {

        m_oscillator.set_freq(dynamic_cast<Node<float>*>(&m_internal_freq.get_generative()));
        m_oscillator.set_mul(dynamic_cast<Node<float>*>(&m_internal_mul.get_generative()));
        m_oscillator.set_add(dynamic_cast<Node<float>*>(&m_internal_add.get_generative()));
        m_oscillator.set_duty(dynamic_cast<Node<float>*>(&m_internal_duty.get_generative()));
        m_oscillator.set_curve(dynamic_cast<Node<float>*>(&m_internal_curve.get_generative()));
        m_oscillator.set_enabled(dynamic_cast<Node<bool>*>(&m_enable_button.get_generative()));


        addAndMakeVisible(m_internal_freq);
        addAndMakeVisible(m_internal_mul);
        addAndMakeVisible(m_internal_add);
        addAndMakeVisible(m_internal_duty);
        addAndMakeVisible(m_internal_curve);

        m_label.setText(id, juce::dontSendNotification);
        m_label.setEditable(false);
        m_label.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(m_label);

        addAndMakeVisible(m_enable_button);

        startTimerHz(33);
    }


    void timerCallback() override {
        auto queue = m_oscillator.get_output_history();
        if (!queue.empty()) {
            std::cout << "output: ";
            for (auto& e: queue) {
                std::cout << e << " ";
            }
            std::cout << "\n";
        }
    }


    Generative& get_generative() override {
        return m_oscillator;
    }


private:
    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
    }


    void resized() override {
        if (m_layout == Layout::full) {
            full_layout();
        }
    }


private:

    void full_layout() {
        auto bounds = getLocalBounds().reduced(8);

        auto header_bounds = bounds.removeFromTop(20);
        m_enable_button.setBounds(header_bounds.removeFromLeft(22));
        header_bounds.removeFromLeft(10);
        m_label.setBounds(header_bounds);

        bounds.removeFromTop(5);

        auto component_width = bounds.getWidth() / 5;
        auto spacing = component_width * 0.075;
        m_internal_freq.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_mul.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_add.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_duty.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
        m_internal_curve.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
    }

    NewOscillator m_oscillator;

    // TODO: InternalType
    SliderComponent<float> m_internal_freq;
    SliderComponent<float> m_internal_mul;
    SliderComponent<float> m_internal_add;
    SliderComponent<float> m_internal_duty;
    SliderComponent<float> m_internal_curve;

    ToggleButtonComponent m_enable_button;
    juce::Label m_label;

    Layout m_layout = Layout::full;


};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
