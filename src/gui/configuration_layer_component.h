

#ifndef SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H
#define SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "parameter_policy.h"
#include "generative_component.h"
#include "connector_component.h"
#include "source.h"
#include "modular_generator.h"
#include "key_state.h"
#include "keyboard_shortcuts.h"

class ConfigurationLayerComponent : public juce::Component
                                    , public juce::DragAndDropContainer {
public:

    struct ComponentAndBounds {
        std::unique_ptr<GenerativeComponent> component;
        juce::Rectangle<int> position;
    };

    using KeyCodes = ConfigurationLayerKeyboardShortcuts;


    explicit ConfigurationLayerComponent(ModularGenerator& modular_generator)
            : m_modular_generator(modular_generator) {
        setWantsKeyboardFocus(true);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::cadetblue);
    }


    void resized() override {
        for (auto& component_and_bounds : m_generative_components) {
            component_and_bounds.component->setBounds(component_and_bounds.position);
        }
    }


    void add_component(std::unique_ptr<GenerativeComponent> component, juce::Rectangle<int> location) {
        (void) component;
        (void) location;
//        std::lock_guard<std::mutex> lock(process_mutex);
//
//        if (std::find(m_nodes.begin(), m_nodes.end(), component) != m_nodes.end())
//            throw std::runtime_error("Cannot add a component twice");
//
//        if (auto* source = dynamic_cast<Source*>(&component->get_generative())) {
//            m_sources.emplace_back(source);
//        }
//
//        addAndMakeVisible(*component);
//        component->setBounds(location);
//
//        m_nodes.emplace_back(std::move(component));
    }


    void remove(GenerativeComponent* component) {
        (void) component;
//        std::lock_guard<std::mutex> lock(process_mutex);
        throw std::runtime_error("remove is not implemented"); // TODO
    }


    void connect() {
        std::cout << "CONNECTING (DUMMY)\n";
    }


    void disconnect() {
        std::cout << "DISCONNECTING (DUMMY)\n";
    }


    void reposition() {
        throw std::runtime_error("not implemented"); // TODO
    }


private:

    void mouseDown(const juce::MouseEvent& event) override {

    }


    void mouseUp(const juce::MouseEvent& event) override {
        if (m_key_state.is_down(KeyCodes::CONNECTOR_KEY)) {
            connect();
        } else {
            create_component(event);
        }
    }


    bool keyPressed(const juce::KeyPress& key) override {
        if (m_key_state.keypress(key)) {
            std::cout << "key pressed hoho\n";
        }
        return true;
    }


    bool keyStateChanged(bool isKeyDown) override {
        if (!isKeyDown && m_key_state.key_up_registered()) {
            std::cout << "key released\n";
        }
        return true;
    }


    void modifierKeysChanged(const juce::ModifierKeys& modifiers) override {
        if (m_key_state.modifiers(modifiers)) {
            std::cout << "modifiers changed\n";
        }
    }


    void create_component(const juce::MouseEvent& event) {
        if (m_key_state.is_down(KeyCodes::NEW_MIDI_SOURCE_KEY)) {
            auto [component, generatives] = ModuleFactory::new_generator<float>("generator", m_modular_generator);
            component->setAlpha(0.4f);
            addAndMakeVisible(*component);
            m_generative_components.push_back({std::move(component), {event.getPosition().getX(), event.getPosition().getY(), GeneratorModule<int>::width_of(), GeneratorModule<int>::height_of()}});
            m_modular_generator.add(std::move(generatives));
            std::cout << m_generative_components.size() << "\n\n\n";
            resized();
        } else if (m_key_state.is_down(KeyCodes::NEW_GENERATOR_KEY)) {
            std::cout << "CREATE NEW GENERATOR\n";
        } else if (m_key_state.is_down(KeyCodes::NEW_OSCILLATOR_KEY)) {
            std::cout << "CREATE NEW OSCILLATOR\n";
        } else {
            std::cout << "(click without modifier)\n";
        }

    }


    template<typename T>
    GenerativeComponent* get_associated_component(Generative* connectable) {
        (void) connectable;
        throw std::runtime_error("not implemented"); // TODO
    }


    void update_connections() {
        throw std::runtime_error("not implemented"); // TODO
    }


    ModularGenerator& m_modular_generator;

    KeyState m_key_state;

    std::vector<ComponentAndBounds> m_generative_components;
    std::vector<ConnectorComponent> m_connectors;


};

#endif //SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H
