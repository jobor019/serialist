
#ifndef SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H
#define SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H

#include <interaction_visualizer.h>
#include "connectable_module.h"


enum class ActionTypes {
    move = 0
    , remove = 1
    , connect = 2
};


// ==============================================================================================

class BorderHighlight : public juce::Component {
public:
    explicit BorderHighlight(juce::Colour color, int border_width = 2)
            : m_color(color), m_border_width(border_width) {}


    void paint(juce::Graphics& g) override {
        g.setColour(m_color);
        g.drawRect(getLocalBounds(), m_border_width);
    }


private:
    juce::Colour m_color;
    int m_border_width;

};



// ==============================================================================================

class BorderAndFillHighlight : public juce::Component {
public:
    explicit BorderAndFillHighlight(std::optional<juce::Colour> border_color
                                    , std::optional<juce::Colour> fill_color
                                    , int border_width = 2)
            : m_border_color(border_color)
              , m_fill_color(fill_color)
              , m_border_width(border_width) {}


    void paint(juce::Graphics& g) override {
        if (m_fill_color) {
            g.setColour(*m_fill_color);
            g.fillRect(getLocalBounds());
        }

        if (m_border_color) {
            g.setColour(*m_border_color);
            g.drawRect(getLocalBounds(), m_border_width);

        }
    }


private:
    std::optional<juce::Colour> m_border_color;
    std::optional<juce::Colour> m_fill_color;
    int m_border_width;
};


// ==============================================================================================

class ConnectVisualization : public InteractionVisualization {
public:
    explicit ConnectVisualization(juce::Component& source)
            : InteractionVisualization(source)
              , m_source_if_connectable(dynamic_cast<ConnectableModule*>(&source)) {

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
            if (visibility_changed(initial_state, get_visibility()))
                resized();

            return;
        }

        m_mouseover_highlight.setVisible(mouse_is_over_component && active_action == nullptr);
        m_source_highlight.setVisible(active_action && &active_action->get_source() == &get_source_component());

        bool connectable = active_action
                           && m_source_if_connectable
                           && m_source_if_connectable->connectable_to(active_action->get_source());

        m_potential_target_highlight.setVisible(connectable);
        m_target_highlight.setVisible(connectable && mouse_is_over_component);

        if (visibility_changed(initial_state, get_visibility())) {
            resized();
        }
    }


    void resized() override {
        m_mouseover_highlight.setBounds(getLocalBounds());
        m_source_highlight.setBounds(getLocalBounds());
        m_target_highlight.setBounds(getLocalBounds());
        m_potential_target_highlight.setBounds(getLocalBounds());
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
        return std::any_of(visibility.begin(), visibility.end(), [](bool value) { return value; });
    }


    static bool visibility_changed(std::array<bool, 4> before, std::array<bool, 4> after) {
        return !std::equal(before.begin(), before.end(), after.begin());
    }


    BorderHighlight m_mouseover_highlight{juce::Colours::orchid};
    BorderHighlight m_source_highlight{juce::Colours::greenyellow};
    BorderAndFillHighlight m_target_highlight{std::nullopt, {juce::Colours::skyblue.withAlpha(0.5f)}};
    BorderHighlight m_potential_target_highlight{juce::Colours::chocolate};

    ConnectableModule* m_source_if_connectable;
};


// ==============================================================================================

class DisconnectVisualization : public InteractionVisualization {
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


    explicit DisconnectVisualization(juce::Component& source)
            : InteractionVisualization(source) {
        addChildComponent(m_mouseover_highlight);
    }


    void resized() override {
        m_mouseover_highlight.setBounds(getLocalBounds());
    }


    void update_state(bool mouse_is_over_component, Action*) override {
        bool is_visible = m_mouseover_highlight.isVisible();
        if (GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::DISCONNECT_KEY)
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
    TempHighlight m_mouseover_highlight{juce::Colours::deeppink.withAlpha(0.8f)};

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


// ==============================================================================================

class DeleteVisualization : public InteractionVisualization {
public:
    class TempHighlight : public juce::Component {
    public:
        explicit TempHighlight(juce::Colour color) : m_color(color) {}


        void paint(juce::Graphics& g) override {
            g.setColour(m_color);
            g.drawRect(getLocalBounds(), 3);
            g.setColour(juce::Colours::salmon.withAlpha(0.4f));
            g.fillRect(getLocalBounds());
        }


    private:
        juce::Colour m_color;

    };


    explicit DeleteVisualization(juce::Component& source)
            : InteractionVisualization(source) {
        addChildComponent(m_mouseover_highlight);
    }


    void resized() override {
        m_mouseover_highlight.setBounds(getLocalBounds());
    }


    void update_state(bool mouse_is_over_component, Action*) override {
        bool is_visible = m_mouseover_highlight.isVisible();
        if (GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::DELETE_KEY)
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
    TempHighlight m_mouseover_highlight{juce::Colours::firebrick};

};


#endif //SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H
