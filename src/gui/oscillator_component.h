#ifndef SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
#define SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "../oscillator.h" // TODO: Why can't this be relative?
#include "node_component.h"
#include "slider_object.h"
#include "toggle_button_object.h"
#include "header_component.h"
#include "combobox_object.h"
#include "oscillator_view.h"


class OscillatorComponent : public GenerativeComponent {
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
              , m_oscillator_view(m_oscillator)
              , m_oscillator_type(id + "::type", parent, {
                    {  "sin", Oscillator::Type::sin}
                    , {"sqr", Oscillator::Type::square}
                    , {"tri", Oscillator::Type::tri}
            }, Oscillator::Type::sin, "mode")
              , m_internal_freq(id + "::freq", parent, 0.0f, 10.0f, 0.125f, 0.5f, "freq")
              , m_internal_mul(id + "::mul", parent, 0.0f, 10.0f, 0.125f, 1.0, "mul")
              , m_internal_add(id + "::add", parent, 0.0f, 10.0f, 0.125f, 0.0f, "add")
              , m_internal_duty(id + "::duty", parent, 0.0f, 1.0f, 0.01f, 0.5f, "duty")
              , m_internal_curve(id + "::curve", parent, 0.0f, 0.0f, 0.0f, 0.0f, "curve")
              , m_header(id, parent)
              , m_layout(layout) {

        m_oscillator.set_type(dynamic_cast<Node<Oscillator::Type>*>(&m_oscillator_type.get_generative()));
        m_oscillator.set_freq(dynamic_cast<Node<float>*>(&m_internal_freq.get_generative()));
        m_oscillator.set_mul(dynamic_cast<Node<float>*>(&m_internal_mul.get_generative()));
        m_oscillator.set_add(dynamic_cast<Node<float>*>(&m_internal_add.get_generative()));
        m_oscillator.set_duty(dynamic_cast<Node<float>*>(&m_internal_duty.get_generative()));
        m_oscillator.set_curve(dynamic_cast<Node<float>*>(&m_internal_curve.get_generative()));

        m_oscillator.set_enabled(dynamic_cast<Node<bool>*>(&m_header.get_enabled().get_generative()));

        addAndMakeVisible(m_oscillator_view);

        addAndMakeVisible(m_oscillator_type);
        addAndMakeVisible(m_internal_freq);
        addAndMakeVisible(m_internal_mul);
        addAndMakeVisible(m_internal_add);
        addAndMakeVisible(m_internal_duty);
        addAndMakeVisible(m_internal_curve);

        addAndMakeVisible(m_header);
    }


    std::pair<int, int> dimensions() override {
        return {default_width(), default_height()};
    }


    int default_height() const {
//        if (m_layout == Layout::full) {
            return m_header.default_height()
                   + 2 * DimensionConstants::COMPONENT_UD_MARGINS
                   + m_oscillator_view.default_height()
                   + m_oscillator_type.default_height()
                   + 5 * m_internal_freq.default_height()
                   + 7 * DimensionConstants::OBJECT_Y_MARGINS_COLUMN;
//        } else if (m_layout == Layout::generator_internal) {
//            return 0;
//        }
    }


    int default_width() const {
//        if (m_layout == Layout::full) {
            return 2 * DimensionConstants::COMPONENT_LR_MARGINS + m_internal_freq.default_width();
//        }
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
        bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);

        m_oscillator_view.set_layout(OscillatorView::Layout::full);
        m_oscillator_view.setBounds(bounds.removeFromTop(m_oscillator_view.default_height()));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        m_oscillator_type.set_label_position(ComboBoxObject<Oscillator::Type>::LabelPosition::left);
        m_oscillator_type.setBounds(bounds.removeFromTop(m_oscillator_type.default_height()));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        m_internal_freq.set_label_position(SliderObject<float>::LabelPosition::left);
        m_internal_freq.setBounds(bounds.removeFromTop(m_internal_freq.default_height()));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        m_internal_mul.set_label_position(SliderObject<float>::LabelPosition::left);
        m_internal_mul.setBounds(bounds.removeFromTop(m_internal_mul.default_height()));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        m_internal_add.set_label_position(SliderObject<float>::LabelPosition::left);
        m_internal_add.setBounds(bounds.removeFromTop(m_internal_add.default_height()));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        m_internal_duty.set_label_position(SliderObject<float>::LabelPosition::left);
        m_internal_duty.setBounds(bounds.removeFromTop(m_internal_duty.default_height()));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        m_internal_curve.set_label_position(SliderObject<float>::LabelPosition::left);
        m_internal_curve.setBounds(bounds.removeFromTop(m_internal_curve.default_height()));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);
    }


    void generator_internal_layout() {
        auto bounds = getLocalBounds().reduced(5, 8);

        // TODO: Enabled must be ON by default
//        m_header.setBounds(bounds.removeFromTop(m_header.default_height()));
//        bounds.reduce(5, 8);

        auto col1 = bounds.removeFromLeft(bounds.proportionOfWidth(0.45f));

        m_oscillator_view.set_layout(OscillatorView::Layout::compact);
        m_oscillator_view.setBounds(col1.removeFromTop(m_oscillator_view.default_height()));
        col1.removeFromTop(FULL_LAYOUT_SPACING);

        m_oscillator_type.set_label_position(ComboBoxObject<Oscillator::Type>::LabelPosition::bottom);
        m_oscillator_type.setBounds(col1.removeFromLeft(col1.getWidth() / 2));
        col1.removeFromLeft(FULL_LAYOUT_SPACING);

        m_internal_freq.set_label_position(SliderObject<float>::LabelPosition::bottom);
        m_internal_freq.setBounds(col1);

        auto col2 = bounds.removeFromLeft(bounds.getWidth() / 2);
        m_internal_mul.set_label_position(SliderObject<float>::LabelPosition::bottom);
        m_internal_mul.setBounds(col2.removeFromTop(m_internal_mul.default_height()));
        col2.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_add.set_label_position(SliderObject<float>::LabelPosition::bottom);
        m_internal_add.setBounds(col2.removeFromTop(m_internal_add.default_height()));
        col2.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_duty.set_label_position(SliderObject<float>::LabelPosition::bottom);
        m_internal_duty.setBounds(bounds.removeFromTop(m_internal_duty.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);

        m_internal_curve.set_label_position(SliderObject<float>::LabelPosition::bottom);
        m_internal_curve.setBounds(bounds.removeFromTop(m_internal_curve.default_height()));
        bounds.removeFromTop(FULL_LAYOUT_SPACING);
    }


    Oscillator m_oscillator;

    OscillatorView m_oscillator_view;

    // TODO: typename InternalType rather than float
    ComboBoxObject<Oscillator::Type> m_oscillator_type;
    SliderObject<float> m_internal_freq;
    SliderObject<float> m_internal_mul;
    SliderObject<float> m_internal_add;
    SliderObject<float> m_internal_duty;
    SliderObject<float> m_internal_curve;

    HeaderComponent m_header;

    Layout m_layout = Layout::full;


};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
