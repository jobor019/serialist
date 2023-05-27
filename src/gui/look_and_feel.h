

#ifndef SERIALISTLOOPER_LOOK_AND_FEEL_H
#define SERIALISTLOOPER_LOOK_AND_FEEL_H

#include <juce_gui_extra/juce_gui_extra.h>

enum Colors {
    object_on = 0xf000001
    , object_off = 0xf000002
    , outline_mouseover = 0xf000003
    , object_border_color = 0xf000004
    , text_color = 0xf000004

    , component_border_color = 0xf000006

    , header_color = 0xf000005

};

class SerialistLookAndFeel : public juce::LookAndFeel_V4 {
public:

    static inline const juce::Font default_font = juce::Font("Arial", 12.0f, juce::Font::FontStyleFlags::plain);


    static void setup_look_and_feel_colors(juce::LookAndFeel& lnf) {
        lnf.setColour(Colors::object_on, juce::Colour(0xffffb532));
        lnf.setColour(Colors::object_off, juce::Colour(0xff5a5a5a));
        lnf.setColour(Colors::outline_mouseover, juce::Colour(0xfff5deb3));
        lnf.setColour(Colors::object_border_color, juce::Colour(0xff212121));
        lnf.setColour(Colors::text_color, juce::Colour(0xff000000));

        lnf.setColour(Colors::component_border_color, juce::Colour(0xffcee5e8));

        lnf.setColour(Colors::header_color, juce::Colour(0xff68b74d));

        lnf.setColour(juce::Label::ColourIds::textColourId, juce::Colour(0xff000000));

    }


    juce::Font getLabelFont(juce::Label&) override {
        return default_font;
    }


    juce::Font getTextButtonFont(juce::TextButton&, int) override {
        return default_font;
    }


    void drawToggleButton(juce::Graphics& g
                          , juce::ToggleButton& button
                          , bool shouldDrawButtonAsHighlighted
                          , bool shouldDrawButtonAsDown) override {
        (void) shouldDrawButtonAsDown;

        bool has_text = button.getButtonText().isNotEmpty();

        if (button.getToggleState())
            g.setColour(findColour(Colors::object_on));
        else
            g.setColour(findColour(Colors::object_off));

        if (has_text)
            g.fillRect(button.getLocalBounds().reduced(2).toFloat());
        else
            g.fillEllipse(button.getLocalBounds().reduced(2).toFloat());


        if (shouldDrawButtonAsHighlighted)
            g.setColour(findColour(Colors::outline_mouseover));
        else
            g.setColour(findColour(Colors::object_border_color));

        if (has_text)
            g.drawRect(button.getLocalBounds().reduced(2).toFloat(), 1.0f);
        else
            g.drawEllipse(button.getLocalBounds().reduced(2).toFloat(), 1.0f);

        g.setColour(findColour(Colors::text_color));
        g.setFont(default_font);

        if (!button.isEnabled())
            g.setOpacity(0.5f);

        g.drawFittedText(button.getButtonText(), button.getLocalBounds().withTrimmedLeft(2)
                .withTrimmedRight(2), juce::Justification::centred, 10);
    }


private:

};

#endif //SERIALISTLOOPER_LOOK_AND_FEEL_H
