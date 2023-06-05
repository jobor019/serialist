

#ifndef SERIALISTLOOPER_HEADER_WIDGET_H
#define SERIALISTLOOPER_HEADER_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "variable.h"
#include "toggle_button_widget.h"
#include "look_and_feel.h"

class HeaderWidget : public juce::Component
                     , private juce::Button::Listener {
public:

    explicit HeaderWidget(const std::string& public_name)
            : m_enabled(std::nullopt)
              , m_label({}, public_name)
              , m_stepped(std::nullopt)
              , m_minimized("-") {
        initialize_widgets();

    }


    HeaderWidget(const std::string& public_name
                 , Variable<bool>& enabled)
            : m_enabled(enabled)
              , m_label({}, public_name)
              , m_stepped(std::nullopt)
              , m_minimized("-") {
        initialize_widgets();
    }


    HeaderWidget(const std::string& public_name
                 , Variable<bool>& enabled
                 , Variable<bool>& stepped)
            : m_enabled(enabled)
              , m_label({}, public_name)
              , m_stepped(std::make_optional<ToggleButtonWidget>(stepped))
              , m_minimized("-") {
        initialize_widgets();
    }


    static int height_of() { return DimensionConstants::COMPONENT_HEADER_HEIGHT; }


    void paint(juce::Graphics& g) override {
        g.setColour(findColour(Colors::header_color));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
        g.setColour(findColour(Colors::component_border_color));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 1.0f);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(DimensionConstants::HEADER_INTERNAL_MARGINS);

        auto enabled_bounds = bounds.removeFromLeft(bounds.getHeight());
        if (m_enabled) {
            m_enabled->setBounds(enabled_bounds);
        }

        m_minimized.setBounds(bounds.removeFromRight(bounds.getHeight()));

        auto font = m_label.getFont();

        if (m_stepped) {
            bounds.removeFromRight(DimensionConstants::HEADER_INTERNAL_MARGINS);
            m_stepped->setBounds(bounds.removeFromRight(4 + font.getStringWidth(m_stepped->get_text())));
        }

        m_label.setBounds(bounds);
    }


    std::optional<ToggleButtonWidget>& get_enabled() { return m_enabled; }


    std::optional<ToggleButtonWidget>& get_stepped() { return m_stepped; }


private:

    void initialize_widgets() {
        m_label.setJustificationType(juce::Justification::centredLeft);

        if (m_enabled)
            addAndMakeVisible(*m_enabled);

        addAndMakeVisible(m_label);

        if (m_stepped)
            addAndMakeVisible(*m_stepped);

        addAndMakeVisible(m_minimized);
        m_minimized.addListener(this);
    }


    void buttonClicked(juce::Button*) override {
        std::cout << "minimize button not implemented\n";
    }


    std::optional<ToggleButtonWidget> m_enabled;
    juce::Label m_label;
    std::optional<ToggleButtonWidget> m_stepped;
    juce::ToggleButton m_minimized;

};

#endif //SERIALISTLOOPER_HEADER_WIDGET_H
