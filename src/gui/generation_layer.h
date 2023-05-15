

#ifndef SERIALISTLOOPER_GENERATION_LAYER_H
#define SERIALISTLOOPER_GENERATION_LAYER_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "parameter_policy.h"
#include "node_component.h"
#include "connector_component.h"
#include "source.h"

class GenerationLayer : public juce::Component
                        , public ParameterHandler {
public:

    GenerationLayer(juce::ValueTree& vt, juce::UndoManager& um)
            : ParameterHandler(vt, um) {

    }


    void process(const TimePoint& time) {
        std::lock_guard<std::mutex> lock(process_mutex);
        for (auto& source: m_sources) {
            source->process(time);
        }
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::aquamarine.withAlpha(0.6f));
    }


    void resized() override {
        std::cout << "GenerationLayer.resized() not implemented" << std::endl;
    }


    void add_component(std::unique_ptr<NodeComponent> component, juce::Rectangle<int> location) {
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


    void remove(NodeComponent* component) {
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
    NodeComponent* get_associated_component(Generative* connectable) {
        (void) connectable;
        throw std::runtime_error("not implemented"); // TODO
    }


    void update_connections() {
        throw std::runtime_error("not implemented"); // TODO
    }


    std::vector<std::unique_ptr<NodeComponent>> m_nodes;
    std::vector<Source*> m_sources;
    std::vector<ConnectorComponent> m_connectors;

    std::mutex process_mutex;

};

#endif //SERIALISTLOOPER_GENERATION_LAYER_H
