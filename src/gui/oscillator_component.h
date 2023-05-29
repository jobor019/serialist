#ifndef SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
#define SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "../oscillator.h" // TODO: Why can't this be relative?
#include "node_component.h"
#include "slider_component.h"
#include "toggle_button_component.h"
#include "header_component.h"
#include "combobox_component.h"


class OscillatorComponent : public NodeComponent
                            , public juce::Timer {
public:

    static const int FULL_LAYOUT_SPACING = 4;

    enum class Layout {
        full
        , generator_internal
    };


    int get_default_width() {
        if (m_layout == Layout::full) {
            return FULL_LAYOUT_SPACING * 6 + m_header.default_height() +
                   6 * m_internal_freq.default_height(); // TODO: missing oscillatorview
        }
        return -1;
    }


    OscillatorComponent(const std::string& id, ParameterHandler& parent, Layout layout = Layout::full)
            : m_oscillator(id, parent)
              , m_oscillator_type(id + "::type", parent, {
                    {  "sin", Oscillator::Type::sin}
                    , {"sqr", Oscillator::Type::square}
                    , {"tri", Oscillator::Type::tri}
            }, Oscillator::Type::sin, "mode", 0.35f)
              , m_internal_freq(id + "::freq", parent, 0.0f, 10.0f, 0.125f, 0.5f, "freq", 0.35f)
              , m_internal_mul(id + "::mul", parent, 0.0f, 10.0f, 0.125f, 1.0, "mul", 0.35f)
              , m_internal_add(id + "::add", parent, 0.0f, 10.0f, 0.125f, 0.0f, "add", 0.35f)
              , m_internal_duty(id + "::duty", parent, 0.0f, 1.0f, 0.01f, 0.5f, "duty", 0.35f)
              , m_internal_curve(id + "::curve", parent, 0.0f, 0.0f, 0.0f, 0.0f, "curve", 0.35f)
              , m_header(id, parent)
              , m_layout(layout) {

        m_oscillator.set_type(dynamic_cast<Node<Oscillator::Type>*>(&m_oscillator_type.get_generative()));
        m_oscillator.set_freq(dynamic_cast<Node<float>*>(&m_internal_freq.get_generative()));
        m_oscillator.set_mul(dynamic_cast<Node<float>*>(&m_internal_mul.get_generative()));
        m_oscillator.set_add(dynamic_cast<Node<float>*>(&m_internal_add.get_generative()));
        m_oscillator.set_duty(dynamic_cast<Node<float>*>(&m_internal_duty.get_generative()));
        m_oscillator.set_curve(dynamic_cast<Node<float>*>(&m_internal_curve.get_generative()));

        m_oscillator.set_enabled(dynamic_cast<Node<bool>*>(&m_header.get_enabled().get_generative()));


        addAndMakeVisible(m_oscillator_type);
        addAndMakeVisible(m_internal_freq);
        addAndMakeVisible(m_internal_mul);
        addAndMakeVisible(m_internal_add);
        addAndMakeVisible(m_internal_duty);
        addAndMakeVisible(m_internal_curve);

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
        g.setColour(getLookAndFeel().findColour(Colors::component_border_color));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
    }


    void resized() override {
        if (m_layout == Layout::full) {
            full_layout();
        } else if (m_layout == Layout::generator_internal) {
            generator_internal_layout();
        }
    }


    void full_layout() {
        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(m_header.default_height()));
        bounds.reduce(5, 8);

        m_oscillator_view.setBounds(bounds.removeFromTop(20));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_oscillator_type.set_label_position(ComboBoxComponent<Oscillator::Type>::LabelPosition::left);
        m_oscillator_type.setBounds(bounds.removeFromTop(m_oscillator_type.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_freq.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_freq.setBounds(bounds.removeFromTop(m_internal_freq.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_mul.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_mul.setBounds(bounds.removeFromTop(m_internal_mul.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_add.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_add.setBounds(bounds.removeFromTop(m_internal_add.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_duty.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_duty.setBounds(bounds.removeFromTop(m_internal_duty.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_curve.set_label_position(SliderComponent<float>::LabelPosition::left);
        m_internal_curve.setBounds(bounds.removeFromTop(m_internal_curve.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);
    }


    void generator_internal_layout() {
        auto bounds = getLocalBounds().reduced(5, 8);

        // TODO: Enabled must be ON by default
//        m_header.setBounds(bounds.removeFromTop(m_header.default_height()));
//        bounds.reduce(5, 8);

        auto col1 = bounds.removeFromLeft(getWidth() / 2);

        m_oscillator_view.setBounds(col1.removeFromTop(20));
        col1.removeFromTop(FULL_LAYOUT_SPACING);

        m_oscillator_type.set_label_position(ComboBoxComponent<Oscillator::Type>::LabelPosition::bottom);
        m_oscillator_type.setBounds(col1.removeFromLeft(col1.getWidth() / 2));
        col1.removeFromLeft(FULL_LAYOUT_SPACING);

        m_internal_freq.set_label_position(SliderComponent<float>::LabelPosition::bottom);
        m_internal_freq.setBounds(col1);

        auto col2 = bounds.removeFromLeft(bounds.getWidth() / 2);
        m_internal_mul.set_label_position(SliderComponent<float>::LabelPosition::bottom);
        m_internal_mul.setBounds(col2.removeFromTop(m_internal_mul.default_height()));
        col2.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_add.set_label_position(SliderComponent<float>::LabelPosition::bottom);
        m_internal_add.setBounds(col2.removeFromTop(m_internal_add.default_height()));
        col2.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_duty.set_label_position(SliderComponent<float>::LabelPosition::bottom);
        m_internal_duty.setBounds(bounds.removeFromTop(m_internal_duty.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_curve.set_label_position(SliderComponent<float>::LabelPosition::bottom);
        m_internal_curve.setBounds(bounds.removeFromTop(m_internal_curve.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);
    }


    Oscillator m_oscillator;

    // TODO: typename InternalType rather than float
    ComboBoxComponent<Oscillator::Type> m_oscillator_type;
    SliderComponent<float> m_internal_freq;
    SliderComponent<float> m_internal_mul;
    SliderComponent<float> m_internal_add;
    SliderComponent<float> m_internal_duty;
    SliderComponent<float> m_internal_curve;

    HeaderComponent m_header;

    juce::Slider m_oscillator_view;

    Layout m_layout = Layout::full;


};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
