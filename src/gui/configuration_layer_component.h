

#ifndef SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H
#define SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "parameter_policy.h"
#include "generative_component.h"
#include "connector_component.h"
#include "source.h"

class ConfigurationLayerComponent : public juce::Component {
public:

    ConfigurationLayerComponent(ModularGenerator& modular_generator) : m_modular_generator(modular_generator) {}


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::cadetblue);
    }


    void resized() override {
        std::cout << "ConfigurationLayerComponent.resized() not implemented" << std::endl;
    }


    void add_component(std::unique_ptr<GenerativeComponent> component, juce::Rectangle<int> location) {
        std::lock_guard<std::mutex> lock(process_mutex);

        if (std::find(m_nodes.begin(), m_nodes.end(), component) != m_nodes.end())
            throw std::runtime_error("Cannot add a component twice");

        if (auto* source = dynamic_cast<Source*>(&component->get_generative())) {
            m_sources.emplace_back(source);
        }

        addAndMakeVisible(*component);
        component->setBounds(location);

        m_nodes.emplace_back(std::move(component));
    }


    void remove(GenerativeComponent* component) {
        (void) component;
        std::lock_guard<std::mutex> lock(process_mutex);
        throw std::runtime_error("remove is not implemented"); // TODO
    }


    void connect() {
        throw std::runtime_error("not implemented"); // TODO
    }


    void disconnect() {
        throw std::runtime_error("not implemented"); // TODO
    }


    void reposition() {
        throw std::runtime_error("not implemented"); // TODO
    }


private:
    template<typename T>
    GenerativeComponent* get_associated_component(Generative* connectable) {
        (void) connectable;
        throw std::runtime_error("not implemented"); // TODO
    }


    void update_connections() {
        throw std::runtime_error("not implemented"); // TODO
    }



    std::vector<ConnectorObject> m_connectors;



};

#endif //SERIALISTLOOPER_CONFIGURATION_LAYER_COMPONENT_H
