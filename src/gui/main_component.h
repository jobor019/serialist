#ifndef SERIALIST_LOOPER_MAIN_COMPONENT_H
#define SERIALIST_LOOPER_MAIN_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "node_component.h"
#include "source.h"
#include "connector_component.h"
#include "transport.h"
#include "scheduler.h"
#include "io/renderers.h"
#include "parameter_policy.h"
#include "generation_layer.h"
#include "midi_note_source_component.h"
#include "oscillator_component.h"


class MainComponent : public juce::Component
                      , private juce::HighResolutionTimer {

public:
    MainComponent()
            : m_value_tree(m_vt_identifier)
              , m_generation_layer(m_value_tree, m_undo_manager) {

        addAndMakeVisible(m_generation_layer);

        m_generation_layer.add_component(std::make_unique<MidiNoteSourceComponent>("midi1", m_generation_layer)
                                         , {50, 50, 220, 75});

        m_generation_layer.add_component(std::make_unique<MidiNoteSourceComponent>("midi2", m_generation_layer)
                                         , {330, 50, 220, 75});

        m_generation_layer.add_component(std::make_unique<OscillatorComponent>("osc1", m_generation_layer)
                                         , {50, 150, 100, 100});


        std::cout << m_value_tree.toXmlString() << std::endl;

        m_transport.start();
        startTimer(1);
        setSize(600, 400);

    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello World!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
        m_generation_layer.setBounds(getLocalBounds());
    }


private:

    void hiResTimerCallback() override {
        auto& time = m_transport.update_time();
        m_generation_layer.process(time);
    }


    void start() {
        throw std::runtime_error("not implemented"); // TODO
    }


    void stop() {
        throw std::runtime_error("not implemented"); // TODO
    }


    const juce::Identifier m_vt_identifier = "root";

    juce::ValueTree m_value_tree;
    juce::UndoManager m_undo_manager;

    Transport m_transport;
    GenerationLayer m_generation_layer;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#endif //SERIALIST_LOOPER_MAIN_COMPONENT_H