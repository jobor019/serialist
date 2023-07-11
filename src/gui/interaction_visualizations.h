
#ifndef SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H
#define SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H

#include <interaction_visualizer.h>

enum class ActionTypes {
    move = 0
    , remove = 1
    , connect = 2
};


class ConnectVisualization : public InteractionVisualization {
public:
    class TempHighlight : public juce::Component {
    public:
        explicit TempHighlight(juce::Colour color) : m_color(color) {}


        void paint(juce::Graphics& g) override {
            g.setColour(m_color);
            g.drawRect(getLocalBounds(), 3);
        }


    private:
        juce::Colour m_color;

    };


    explicit ConnectVisualization(juce::Component& source)
            : InteractionVisualization(source)
              , m_source_if_connectable(dynamic_cast<Connectable*>(&source)) {

        addChildComponent(m_mouseover_highlight);
        addChildComponent(m_source_highlight);
        addChildComponent(m_target_highlight);
        addChildComponent(m_potential_target_highlight);
    }


    void update_state(bool mouse_is_over_component, Action* active_action) override {
        auto initial_state = get_visibility();

        if (!GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::CONNECTOR_KEY)) {
            // no ongoing connection action
            hide_all();
        } else if (mouse_is_over_component && active_action == nullptr) {
            // connection key is down but no connection action started
            m_mouseover_highlight.setVisible(true);
        } else if (active_action && &active_action->get_source() == &get_source_component()) {
            // connection key is down and ongoing connection action originates from this component
            m_source_highlight.setVisible(true);
        } else if (active_action
                   && m_source_if_connectable
                   && m_source_if_connectable->connectable_to(active_action->get_source())) {
            // connection key is down and ongoing connection which could be connected to this target
            m_potential_target_highlight.setVisible(true);
            if (mouse_is_over_component) {
                m_target_highlight.setVisible(true);
            }
        }

        if (visibility_changed(initial_state, get_visibility())) {
            resized();
        }
    }


private:
    void hide_all() {
        m_mouseover_highlight.setVisible(false);
        m_source_highlight.setVisible(false);
        m_target_highlight.setVisible(false);
        m_potential_target_highlight.setVisible(false);
    }


    std::array<bool, 4> get_visibility() {
        return {m_mouseover_highlight.isVisible()
                , m_source_highlight.isVisible()
                , m_target_highlight.isVisible()
                , m_potential_target_highlight.isVisible()
        };
    }


    static bool any_is_visible(std::array<bool, 4> visibility) {
        std::any_of(visibility.begin(), visibility.end(), [](bool value) { return value; });
    }


    static bool visibility_changed(std::array<bool, 4> before, std::array<bool, 4> after) {
        return !std::equal(before.begin(), before.end(), after.begin());
    }


    TempHighlight m_mouseover_highlight{juce::Colours::orchid};
    TempHighlight m_source_highlight{juce::Colours::greenyellow};
    TempHighlight m_target_highlight{juce::Colours::lightsalmon};
    TempHighlight m_potential_target_highlight{juce::Colours::skyblue};

    Connectable* m_source_if_connectable;
};


// ==============================================================================================

class MoveVisualization : public InteractionVisualization {
public:
    class TempHighlight : public juce::Component {
    public:
        explicit TempHighlight(juce::Colour color) : m_color(color) {}


        void paint(juce::Graphics& g) override {
            g.setColour(m_color);
            g.drawRect(getLocalBounds(), 3);
        }


    private:
        juce::Colour m_color;

    };


    explicit MoveVisualization(juce::Component& source)
            : InteractionVisualization(source) {
        addChildComponent(m_mouseover_highlight);
    }


    void resized() override {
        m_mouseover_highlight.setBounds(getLocalBounds());
    }


    void update_state(bool mouse_is_over_component, Action*) override {
        bool is_visible = m_mouseover_highlight.isVisible();
        if (GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::MOVE_KEY)
            && mouse_is_over_component) {
            m_mouseover_highlight.setVisible(true);
        } else {
            m_mouseover_highlight.setVisible(false);
        }

        if (is_visible != m_mouseover_highlight.isVisible()) {
            resized();
        }
    }


private:
    TempHighlight m_mouseover_highlight{juce::Colours::limegreen};

};


#endif //SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H
