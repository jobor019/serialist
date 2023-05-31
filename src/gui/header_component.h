

#ifndef SERIALISTLOOPER_HEADER_COMPONENT_H
#define SERIALISTLOOPER_HEADER_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "variable.h"
#include "toggle_button_object.h"
#include "look_and_feel.h"

class HeaderComponent : public juce::Component
                        , private juce::Button::Listener {
public:

    explicit HeaderComponent(const std::string& id
                             , ParameterHandler& parent
                             , bool initial = true
                             , bool stepped = true)
            : m_enabled(id + "::enabled", parent, initial)
              , m_label({}, id)
              , m_stepped(id + "::stepped", parent, stepped, "step", "time")
              , m_minimized("-") {
        m_label.setText(id, juce::dontSendNotification);
        m_label.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(m_enabled);
        addAndMakeVisible(m_label);
        addAndMakeVisible(m_stepped);
        addAndMakeVisible(m_minimized);

        m_minimized.addListener(this);
    }


    int default_height() const {
        return DimensionConstants::COMPONENT_HEADER_HEIGHT;
    }


    void paint(juce::Graphics& g) override {
        g.setColour(findColour(Colors::header_color));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
        g.setColour(findColour(Colors::component_border_color));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 1.0f);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(DimensionConstants::HEADER_INTERNAL_MARGINS);
        m_enabled.setBounds(bounds.removeFromLeft(bounds.getHeight()));
        m_minimized.setBounds(bounds.removeFromRight(bounds.getHeight()));

        auto font = m_label.getFont();

        bounds.removeFromRight(DimensionConstants::HEADER_INTERNAL_MARGINS);
        m_stepped.setBounds(bounds.removeFromRight(4 + font.getStringWidth(m_stepped.get_text())));

        m_label.setBounds(bounds);
    }


    ToggleButtonObject& get_enabled() { return m_enabled; }


    ToggleButtonObject& get_stepped() { return m_stepped; }


private:
    void buttonClicked(juce::Button*) override {
        std::cout << "minimize button not implemented\n";
    }


    ToggleButtonObject m_enabled;
    juce::Label m_label;

    ToggleButtonObject m_stepped;

    juce::ToggleButton m_minimized;

};

#endif //SERIALISTLOOPER_HEADER_COMPONENT_H
