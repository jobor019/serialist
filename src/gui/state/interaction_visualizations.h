
#ifndef SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H
#define SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "gui/state/state.h"
#include "io/mouse_state.h"


class InteractionVisualization : public juce::Component {
public:
    explicit InteractionVisualization() {
        setInterceptsMouseClicks(false, false);
    }


    virtual void update_state(const State& active_state, const MouseState& mouse_state) = 0;
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

class FillHighlight : public juce::Component {
public:
    explicit FillHighlight(juce::Colour color) : m_color(color) {}


    void paint(juce::Graphics& g) override {
        g.setColour(m_color);
        g.fillRect(getLocalBounds());
    }


private:
    juce::Colour m_color;
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

class SingleVisualizationBase : public InteractionVisualization {
public:
    explicit SingleVisualizationBase(std::unique_ptr<juce::Component> component)
            : m_component(std::move(component)) {
        addChildComponent(*m_component);
    }


    virtual bool is_active(const State& active_state, const MouseState& mouse_state) const = 0;


    void resized() override {
        m_component->setBounds(getLocalBounds());
    }


    void update_state(const State& active_state, const MouseState& mouse_state) override {
        bool was_visible = m_component->isVisible();

        if (is_active(active_state, mouse_state)) {
            m_component->setVisible(true);
        } else {
            m_component->setVisible(false);
        }

        if (was_visible != m_component->isVisible()) {
            resized();
        }
    }


private:
    std::unique_ptr<juce::Component> m_component;

};


// ==============================================================================================

class MoveVisualization : public SingleVisualizationBase {
public:
    MoveVisualization() : SingleVisualizationBase(std::make_unique<BorderHighlight>(juce::Colours::limegreen)) {}


    bool is_active(const State& active_state, const MouseState& mouse_state) const override {
        return active_state.equals(ModuleIds::ConfigurationLayerComponent, StateIds::Configuration::Move)
               && mouse_state.is_over_component();
    }

};


// ==============================================================================================

class DeleteVisualization : public SingleVisualizationBase {
public:
    DeleteVisualization() : SingleVisualizationBase(std::make_unique<BorderHighlight>(juce::Colours::red)) {}


    bool is_active(const State& active_state, const MouseState& mouse_state) const override {
        return active_state.equals(ModuleIds::ConfigurationLayerComponent, StateIds::Configuration::Delete)
               && mouse_state.is_over_component();
    }
};

#endif //SERIALISTLOOPER_INTERACTION_VISUALIZATIONS_H
