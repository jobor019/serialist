#ifndef SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
#define SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "../oscillator.h" // TODO: Why can't this be relative?
#include "node_component.h"
#include "slider_component.h"
#include "toggle_button_component.h"
#include "header_component.h"


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
              , m_internal_freq(id + "::freq", parent, 0.0f, 10.0f, 0.125f, 0.5f, "freq")
              , m_internal_mul(id + "::mul", parent, 0.0f, 10.0f, 0.125f, 1.0, "mul")
              , m_internal_add(id + "::add", parent, 0.0f, 10.0f, 0.125f, 0.0f, "add")
              , m_internal_duty(id + "::duty", parent, 0.0f, 1.0f, 0.01f, 0.5f, "duty")
              , m_internal_curve(id + "::curve", parent, 0.0f, 0.0f, 0.0f, 0.0f, "curve")
              , m_header(id, parent)
//              , m_enable_button(id + "::enabled", parent, true)
              {

        m_oscillator.set_freq(dynamic_cast<Node<float>*>(&m_internal_freq.get_generative()));
        m_oscillator.set_mul(dynamic_cast<Node<float>*>(&m_internal_mul.get_generative()));
        m_oscillator.set_add(dynamic_cast<Node<float>*>(&m_internal_add.get_generative()));
        m_oscillator.set_duty(dynamic_cast<Node<float>*>(&m_internal_duty.get_generative()));
        m_oscillator.set_curve(dynamic_cast<Node<float>*>(&m_internal_curve.get_generative()));


//        m_oscillator.set_enabled(dynamic_cast<Node<bool>*>(&m_enable_button.get_generative()));
        m_oscillator.set_enabled(dynamic_cast<Node<bool>*>(&m_header.get_enabled().get_generative()));


        addAndMakeVisible(m_internal_freq);
        addAndMakeVisible(m_internal_mul);
        addAndMakeVisible(m_internal_add);
        addAndMakeVisible(m_internal_duty);
        addAndMakeVisible(m_internal_curve);

//        m_label.setText(id, juce::dontSendNotification);
//        m_label.setEditable(false);
//        m_label.setJustificationType(juce::Justification::centredLeft);
//        addAndMakeVisible(m_label);
//
//        addAndMakeVisible(m_enable_button);

        addAndMakeVisible(m_header);

        addAndMakeVisible(m_oscillator_view);
        m_oscillator_view.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_oscillator_view.setInterceptsMouseClicks(false, false);
        m_oscillator_view.setRange(0, 10.0);
        m_oscillator_view.setNumDecimalPlacesToDisplay(2);

        startTimerHz(33);
    }


    void timerCallback() override {
        auto queue = m_oscillator.get_output_history();
        if (!queue.empty()) {
            m_oscillator_view.setValue(queue.back(), juce::dontSendNotification);
        }
    }


    Generative& get_generative() override {
        return m_oscillator;
    }


private:
    void paint(juce::Graphics& g) override {
        g.setColour(getLookAndFeel().findColour(Colors::component_background_color));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
        getLookAndFeel().findColour(Colors::component_border_color);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 2.0f);
    }


    void resized() override {
        if (m_layout == Layout::full) {
            full_layout();
        }
    }


    void full_layout() {
        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(m_header.default_height()));
        bounds.reduce(5, 8);

        m_oscillator_view.setBounds(bounds.removeFromTop(20));

        auto spacing = 4;

        m_internal_freq.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_freq.setBounds(bounds.removeFromTop(m_internal_freq.default_height()));
        bounds.removeFromTop(spacing);

        m_internal_mul.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_mul.setBounds(bounds.removeFromTop(m_internal_mul.default_height()));
        bounds.removeFromTop(spacing);

        m_internal_add.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_add.setBounds(bounds.removeFromTop(m_internal_add.default_height()));
        bounds.removeFromTop(spacing);

        m_internal_duty.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_duty.setBounds(bounds.removeFromTop(m_internal_duty.default_height()));
        bounds.removeFromTop(spacing);

        m_internal_curve.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_curve.setBounds(bounds.removeFromTop(m_internal_curve.default_height()));
        bounds.removeFromTop(spacing);
//        auto component_width = bounds.getWidth() / 5;
//        auto spacing = component_width * 0.075;
//        m_internal_freq.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
//        m_internal_mul.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
//        m_internal_add.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
//        m_internal_duty.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
//        m_internal_curve.setBounds(bounds.removeFromLeft(component_width).reduced(static_cast<int>(spacing), 0));
    }

    Oscillator m_oscillator;

    // TODO: typename InternalType rather than float
    SliderComponent<float> m_internal_freq;
    SliderComponent<float> m_internal_mul;
    SliderComponent<float> m_internal_add;
    SliderComponent<float> m_internal_duty;
    SliderComponent<float> m_internal_curve;

    HeaderComponent m_header;

//    ToggleButtonComponent m_enable_button;
//    juce::Label m_label;

    juce::Slider m_oscillator_view;

    Layout m_layout = Layout::full;


};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
