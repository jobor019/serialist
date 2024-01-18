
#ifndef SERIALISTLOOPER_INTERACTION_VISUALIZER_H
#define SERIALISTLOOPER_INTERACTION_VISUALIZER_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "input_handler.h"


class AlphaMask {
public:
    explicit AlphaMask(juce::Component& component) : m_component(component) {}


    void set_alpha(float alpha) {
        m_component.setAlpha(juce::jlimit(0.0f, 1.0f, alpha));
    }


    void reset() {
        set_alpha(1.0f);
    }


private:
    juce::Component& m_component;
};


// ==============================================================================================

class InteractionVisualization : public juce::Component {
public:
    explicit InteractionVisualization() {
        setInterceptsMouseClicks(false, false);
    }


    virtual void state_changed(InputMode* active_mode, int state, AlphaMask& alpha) = 0;
};


// ==============================================================================================

class InteractionVisualizer : public juce::Component, public InputHandler::Listener {
public:
    explicit InteractionVisualizer(juce::Component& parent
                                   , Vec<std::unique_ptr<InteractionVisualization>> visualizations)
            : m_alpha_mask(parent) {
        setInterceptsMouseClicks(false, false);
        add(std::move(visualizations));
    }

    void mouseMove(const juce::MouseEvent &) override {
        std::cout << "IV Move\n";
    }


    void state_changed(InputMode* active_mode, int state) override {
        for (const auto& v: m_visualizations) {
            v->state_changed(active_mode, state, m_alpha_mask);
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
    }


    void add(Vec<std::unique_ptr<InteractionVisualization>> visualizations) {
        for (auto& v: visualizations) {
            add(std::move(v));
        }
    }


private:
    Vec<std::unique_ptr<InteractionVisualization>> m_visualizations;
    AlphaMask m_alpha_mask;
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

#endif //SERIALISTLOOPER_INTERACTION_VISUALIZER_H
