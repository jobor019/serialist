
#ifndef SERIALISTLOOPER_INTERACTION_VISUALIZER_H
#define SERIALISTLOOPER_INTERACTION_VISUALIZER_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "state_handler.h"
#include "gui/state/interaction_visualizations.h"


class InteractionVisualizer : public juce::Component, public Stateful {
public:
    explicit InteractionVisualizer(Vec<std::unique_ptr<InteractionVisualization>> visualizations)
            : m_visualizations(std::move(visualizations)) {
        setInterceptsMouseClicks(false, false);
        for (const auto& v: m_visualizations) {
            addAndMakeVisible(*v);
        }
    }


    void resized() override {
        for (const auto& v: m_visualizations) {
            v->setBounds(getLocalBounds());
        }
    }


    void add(std::unique_ptr<InteractionVisualization> v) {
        addAndMakeVisible(*v);
        m_visualizations.append(std::move(v));
        resized();
    }


    void add(Vec<std::unique_ptr<InteractionVisualization>> visualizations) {
        for (auto& v: visualizations) {
            add(std::move(v));
        }
    }


    void state_changed(const State& active_state, const MouseState& mouse_state) override {
        for (const auto& v: m_visualizations) {
            v->update_state(active_state, mouse_state);
        }
        // no need to call resized: components are already added and visible.
    }


private:
    Vec<std::unique_ptr<InteractionVisualization>> m_visualizations;
};

#endif //SERIALISTLOOPER_INTERACTION_VISUALIZER_H
