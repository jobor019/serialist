#ifndef SERIALIST_LOOPER_MAIN_COMPONENT_H
#define SERIALIST_LOOPER_MAIN_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "state/generative_component.h"
#include "core/generatives/note_source_LEGACY.h"
#include "bases/connector_component.h"
#include "core/algo/temporal/transport.h"
#include "core/collections/scheduler.h"
#include "io/renderers.h"
#include "core/param/parameter_policy.h"
#include "configuration_layer_component.h"
#include "modules/note_source_module.h"
#include "modules/oscillator_module.h"
#include "modules/generator_module.h"


class MainComponent : public MainKeyboardFocusComponent
                      , private juce::HighResolutionTimer {

public:
    MainComponent()
            : m_value_tree(m_vt_identifier)
              , m_generation_layer(m_value_tree, m_undo_manager) {

        SerialistLookAndFeel::setup_look_and_feel_colors(m_lnf);
        juce::Desktop::getInstance().setDefaultLookAndFeel(&m_lnf);

        addAndMakeVisible(m_generation_layer);

        m_generation_layer.add_component(std::make_unique<NoteSourceModule>("midi1", m_generation_layer)
                                         , {50, 50, 220, 105});

        m_generation_layer.add_component(std::make_unique<NoteSourceModule>("midi2", m_generation_layer)
                                         , {330, 50, 220, 105});

        m_generation_layer.add_component(std::make_unique<OscillatorModule>("osc1", m_generation_layer)
                                         , {330, 200, 100, 200});

        m_generation_layer.add_component(std::make_unique<GeneratorModule<int>>("pitch", m_generation_layer)
                                         , {50, 200, 180, 210});


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

    SerialistLookAndFeel m_lnf;

    const juce::Identifier m_vt_identifier = "root";

    juce::ValueTree m_value_tree;
    juce::UndoManager m_undo_manager;

    Transport m_transport;
    ConfigurationLayerComponent m_generation_layer;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#endif //SERIALIST_LOOPER_MAIN_COMPONENT_H