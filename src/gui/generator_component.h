

#ifndef SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
#define SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H

#include "node_component.h"
#include "parameter_policy.h"
#include "generator.h"

template<typename T>
class GeneratorComponent : NodeComponent {
public:
    enum class Layout {
        full
        , compact
        , internal
    };


    GeneratorComponent(const std::string& id, ParameterHandler& parent)
    : m_generator(id, parent)
    , m_internal_oscillator(id + "::oscillator", parent)
    , m_internal_sequence(id + "::sequence", parent) {
        m_generator.set_cursor(m_internal_oscillator);
        m_generator.set

        addAndMakeVisible(m_generator);
        addAndMakeVisible(m_internal_oscillator);
        addAndMakeVisible(m_internal_sequence);
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
    Generator<T> m_generator;
    OscillatorComponent m_internal_oscillator;
    TextSequenceComponent<T> m_internal_sequence; // TODO: Replace with generic SequenceComponent

    Layout m_layout = Layout::full;
};

#endif //SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
