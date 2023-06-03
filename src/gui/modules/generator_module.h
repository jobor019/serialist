

#ifndef SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
#define SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H

#include "generative_component.h"
#include "parameter_policy.h"
#include "generator.h"
#include "oscillator_module.h"
#include "text_sequence_module.h"
#include "interpolation_module.h"

template<typename T>
class GeneratorModule : public GenerativeComponent {
public:
    enum class Layout {
        full
        , compact
        , internal
    };


    GeneratorModule(const std::string& id, ParameterHandler& parent)
            : m_generator(id, parent)
              , m_header(id, parent)
              , m_internal_oscillator(id + "::oscillator", parent, OscillatorModule::Layout::generator_internal)
              , m_interpolator(id + "::interpolator", parent)
              , m_internal_sequence(id + "::sequence", parent) {

        // TODO: Set enabled, step
        m_generator.set_cursor(dynamic_cast<Node<double>*>(&m_internal_oscillator.get_generative()));
        m_generator.set_interpolation_strategy(
                dynamic_cast<Node<InterpolationStrategy<T>>*>(&m_interpolator.get_generative()));
        m_generator.set_map(dynamic_cast<Sequence<T>*>(&m_internal_sequence.get_generative()));

        addAndMakeVisible(m_header);
        addAndMakeVisible(m_internal_oscillator);
        addAndMakeVisible(m_interpolator);
        addAndMakeVisible(m_internal_sequence);
    }

    static int width_of(Layout layout) {

    }

    static int height_of(Layout layout) {

    }

    std::pair<int, int> dimensions() override {
        return {0, 0};
    }


    Generative& get_generative() override {
        return m_generator;
    }


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
        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(m_header.default_height()));
        bounds.reduce(5, 8);

        m_internal_oscillator.setBounds(bounds.removeFromTop(70));
        bounds.removeFromTop(5);
        m_internal_sequence.setBounds(bounds.removeFromTop(40));
        bounds.removeFromTop(5);
        m_interpolator.setBounds(bounds.removeFromTop(40));
    }


    Generator<T> m_generator;

    HeaderWidget m_header;

    OscillatorModule m_internal_oscillator;
    InterpolationModule<T> m_interpolator;
    TextSequenceModule<T> m_internal_sequence; // TODO: Replace with generic SequenceComponent

    Layout m_layout = Layout::full;
};

#endif //SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
