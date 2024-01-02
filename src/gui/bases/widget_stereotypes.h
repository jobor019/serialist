
#ifndef SERIALISTLOOPER_WIDGET_STEREOTYPES_H
#define SERIALISTLOOPER_WIDGET_STEREOTYPES_H

#include "keyboard_shortcuts.h"
#include "state/generative_component.h"
#include "state/state_handler.h"
#include "state/interaction_visualizations.h"
#include "state/interaction_visualizer.h"

class WidgetStereotypeFuncs {
public:
    using KeyCodes = ConfigurationLayerKeyboardShortcuts;


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return {}; // TODO: Add Connect / Disconnect
    }


    static Vec<TriggerableState> default_states() {
        return Vec<TriggerableState>(
                TriggerableState{std::make_unique<NoKeyCondition>(), States::Default}
        ); // TODO: Add Connect / Disconnect
    }

};


// ==============================================================================================


class WidgetBase : public GenerativeComponent, public Stateful {
public:

    explicit WidgetBase(Generative& main_generative
                        , StateHandler& parent_state_handler
                        , Vec<
            std::unique_ptr<InteractionVisualization>> visualizations = WidgetStereotypeFuncs::default_visualizations()
                        , Vec<TriggerableState> states = WidgetStereotypeFuncs::default_states()
                        , const State& initial_state = States::Default)
            : m_main_generative(main_generative)
              , m_interaction_visualizer(std::move(visualizations))
              , m_state_handler(&parent_state_handler
                                , *this
                                , {*this, m_interaction_visualizer}
                                , std::move(states)
                                , initial_state) {

        setComponentID(m_main_generative.get_parameter_handler().get_id());
        addAndMakeVisible(m_interaction_visualizer);
    }


    Generative& get_generative() override {
        return m_main_generative;
    }


    void resized() final {
        auto bounds = getLocalBounds();
        on_resized(bounds);
        m_interaction_visualizer.setBounds(getLocalBounds());
    }


    StateHandler& get_state_handler() {
        return m_state_handler;
    }


    InteractionVisualizer& get_visualizer() {
        return m_interaction_visualizer;
    }


protected:
    virtual void on_resized(juce::Rectangle<int>& bounds) = 0;


private:
    Generative& m_main_generative;

    InteractionVisualizer m_interaction_visualizer;
    StateHandler m_state_handler;


};


#endif //SERIALISTLOOPER_WIDGET_STEREOTYPES_H
