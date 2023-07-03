

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
#include "editable.h"

class ConfigurationLayerComponent : public juce::Component
                                    , public juce::DragAndDropContainer {
public:

    using KeyCodes = ConfigurationLayerKeyboardShortcuts;

    struct ComponentAndBounds {
        std::unique_ptr<GenerativeComponent> component;
        juce::Rectangle<int> position;
    };


    explicit ConfigurationLayerComponent(ModularGenerator& modular_generator)
            : m_modular_generator(modular_generator) {
        setWantsKeyboardFocus(false);
        setInterceptsMouseClicks(true, false);

    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::cadetblue);
    }


    void resized() override {
        for (auto& component_and_bounds: m_generative_components) {
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


    void disconnect() {
        std::cout << "DISCONNECTING (DUMMY)\n";
    }


    void reposition() {
        throw std::runtime_error("not implemented"); // TODO
    }


private:

    void mouseDown(const juce::MouseEvent& event) override {
        (void) event;
    }


    void mouseUp(const juce::MouseEvent& event) override {
        if (GlobalKeyState::is_down_exclusive(KeyCodes::CONNECTOR_KEY)) {
            connect_component(event);

        } else if (GlobalKeyState::is_down(KeyCodes::DELETE_KEY)) {
            try_remove_component(event);

        } else {
            create_component(event);
        }
    }


    void connect_component(const juce::MouseEvent& event) {
        (void) event;

    }


    void try_remove_component(const juce::MouseEvent& event) {
        auto* c = get_component_under_mouse(event);
        if (c != nullptr) {
            std::cout << "COMPONENT FOUND HEHE:: " << c->component->get_generative().get_identifier_as_string() << "\n";
        } else {
            std::cout << "no component found\n";
        }
    }


    void create_component(const juce::MouseEvent& event) {
        if (GlobalKeyState::is_down_exclusive(KeyCodes::NEW_MIDI_SOURCE_KEY)) {
            auto [component, generatives] = ModuleFactory::new_generator<float>("generator", m_modular_generator);
            addAndMakeVisible(*component);
            m_generative_components.push_back(
                    {std::move(component), {event.getPosition().getX(), event.getPosition().getY(), GeneratorModule<
                            int>::width_of(), GeneratorModule<int>::height_of()}});
            m_modular_generator.add(std::move(generatives));
            std::cout << m_generative_components.size() << "\n\n\n";
            resized();
        } else if (GlobalKeyState::is_down_exclusive(KeyCodes::NEW_GENERATOR_KEY)) {
            std::cout << "CREATE NEW GENERATOR (dummy)\n";
        } else if (GlobalKeyState::is_down_exclusive(KeyCodes::NEW_OSCILLATOR_KEY)) {
            std::cout << "CREATE NEW OSCILLATOR (dummy)\n";
        } else {
            std::cout << "(click without modifier)\n";
        }

    }


    ComponentAndBounds* get_component_under_mouse(const juce::MouseEvent& event) {
        auto* component = getComponentAt(event.getEventRelativeTo(this).getPosition());

        if (!component || component == this)
            return nullptr;


        // getComponentAt may select a child at any depth, iterate up to direct children of this component
        while (component->getParentComponent() != this) {
            component = component->getParentComponent();
        }

        auto it = std::find_if(m_generative_components.begin()
                               , m_generative_components.end()
                               , [&](ComponentAndBounds& c) { return c.component.get() == component; });

        if (it != m_generative_components.end()) {
            return it.base();
        } else {
            return nullptr;
        }

    }


    ModularGenerator& m_modular_generator;

    std::vector<ComponentAndBounds> m_generative_components;
    std::vector<ConnectorComponent> m_connectors;

    // TODO: This should be a dummy bitmap created from each module, rather than just a blank highlight
    EditHighlight m_create_component_highlight;


};

#endif //SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H
