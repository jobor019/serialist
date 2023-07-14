

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
                                    , public GlobalKeyState::Listener
                                    , public juce::DragAndDropContainer {
public:

    using KeyCodes = ConfigurationLayerKeyboardShortcuts;


    struct ComponentAndBounds {
        std::unique_ptr<GenerativeComponent> component;
        juce::Rectangle<int> position;
    };

    class DummyMidiSourceHighlight : public juce::Component {
    public:
        explicit DummyMidiSourceHighlight(const juce::Point<int>& mouse_position) : position(mouse_position) {
            setWantsKeyboardFocus(false);
            setInterceptsMouseClicks(false, false);
        }


        void paint(juce::Graphics& g) override {
            g.setColour(juce::Colours::gold.withAlpha(0.6f));
            g.drawRect(getLocalBounds());
            g.setColour(juce::Colours::gold.withAlpha(0.2f));
            g.fillRect(getLocalBounds());
        }


        juce::Point<int> position;
        // TODO: for now just default EditHighlight implementation but should be a bitmap of the module in the future
    };


    explicit ConfigurationLayerComponent(ModularGenerator& modular_generator)
            : m_modular_generator(modular_generator) {
        GlobalKeyState::add_listener(*this);
        setWantsKeyboardFocus(false);
        setInterceptsMouseClicks(true, false);
    }


    ~ConfigurationLayerComponent() override {
        GlobalKeyState::remove_listener(*this);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::cadetblue);
    }


    void resized() override {
        for (auto& component_and_bounds: m_generative_components) {
            component_and_bounds.component->setBounds(component_and_bounds.position);
        }

        if (m_creation_highlight) {
            auto& pos = m_creation_highlight->position;
            m_creation_highlight->setBounds(pos.getX(), pos.getY(), GeneratorModule<int>::width_of(), GeneratorModule<
                    int>::height_of());
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


    void dragOperationStarted(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        if (auto* source = dragSourceDetails.sourceComponent.get()) {
            std::cout << "DRAGDARG\n";
            GlobalActionHandler::register_action(std::make_unique<Action>(static_cast<int>(ActionTypes::connect)
                                                                          , *source));
        }
    }


    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails&) override {
        std::cout << "DARGENDDD\n";
        GlobalActionHandler::terminate_ongoing_action();
    }


private:

    void mouseEnter(const juce::MouseEvent& event) override {
        if (event.originalComponent == this) {
            m_last_mouse_position = std::make_unique<juce::Point<int>>(event.getPosition());
            process_mouse_highlights();
        }
    }


    void mouseMove(const juce::MouseEvent& event) override {
        if (event.originalComponent == this) {
            m_last_mouse_position = std::make_unique<juce::Point<int>>(event.getPosition());
            process_mouse_highlights();
        }
    }


    void mouseExit(const juce::MouseEvent& event) override {
        if (event.originalComponent == this) {
            m_last_mouse_position = nullptr;
            process_mouse_highlights();
        }
    }


    void key_pressed() override {
        process_mouse_highlights();
    }


    void key_released() override {
        process_mouse_highlights();
    }


    void process_mouse_highlights() {
        if (!m_creation_highlight && !m_last_mouse_position) {
            return; // mouse is not over component
        }

        if (m_last_mouse_position && GlobalKeyState::is_down_exclusive(KeyCodes::NEW_GENERATOR_KEY)) {
            if (m_creation_highlight) {
                m_creation_highlight->position = *m_last_mouse_position;
            } else {
                m_creation_highlight = std::make_unique<DummyMidiSourceHighlight>(*m_last_mouse_position);
                addAndMakeVisible(*m_creation_highlight);
            }

        } else if (m_creation_highlight) {
            m_creation_highlight = nullptr;
        }

        resized();


    }


    void mouseDown(const juce::MouseEvent& event) override {
        (void) event;
    }


    void mouseUp(const juce::MouseEvent& event) override {
//        if (GlobalKeyState::is_down_exclusive(KeyCodes::CONNECTOR_KEY)) {
//            connect_component(event);

        if (GlobalKeyState::is_down(KeyCodes::DELETE_KEY)) {
            try_remove_component(event);

        } else {
            create_component(event);
        }
    }


//    void connect_component(const juce::MouseEvent& event) {
//        (void) event;
//
//    }


    void try_remove_component(const juce::MouseEvent& event) {
        auto* component_and_bounds = get_component_under_mouse(event);
        if (component_and_bounds != nullptr) {
            std::cout << "COMPONENT FOUND HEHE:: "
                      << component_and_bounds->component->get_generative().get_parameter_handler().get_identifier_as_string() << "\n";

            auto& generative = component_and_bounds->component->get_generative();

            m_generative_components.erase(
                    std::remove_if(
                            m_generative_components.begin()
                            , m_generative_components.end()
                            , [component_and_bounds](const ComponentAndBounds& c) { return &c == component_and_bounds; }
                    ), m_generative_components.end());

            m_modular_generator.remove_generative_and_children(generative);

        } else {
            std::cout << "no component found\n";
        }

        std::cout << "Size: " << m_modular_generator.size() << "\n";

    }


    void create_component(const juce::MouseEvent& event) {
        if (event.originalComponent != this)
            return;

        if (GlobalKeyState::any_is_down_exclusive(KeyCodes::NEW_GENERATOR_KEY
                                                  , KeyCodes::NEW_OSCILLATOR_KEY
                                                  , KeyCodes::NEW_MIDI_SOURCE_KEY)) {
            auto cng = ModuleFactory::new_from_key(GlobalKeyState::get_held_keys()[0], m_modular_generator);

            if (!cng)
                return;

            auto component = std::move(cng.value().component);
            auto generatives = std::move(cng.value().generatives);

            addAndMakeVisible(*component);
            component->addMouseListener(this, true);
            m_generative_components.push_back(
                    {std::move(component)
                     , {event.getPosition().getX(), event.getPosition().getY(), cng->width, cng->height}}
            );

            m_modular_generator.add(std::move(generatives));
            std::cout << m_generative_components.size() << "\n\n\n";
            resized();

            m_modular_generator.print_names();
        } else {
            return;
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

    std::unique_ptr<juce::Point<int>> m_last_mouse_position = nullptr;
    std::unique_ptr<DummyMidiSourceHighlight> m_creation_highlight = nullptr;




};

#endif //SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H
