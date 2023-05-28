

#ifndef SERIALISTLOOPER_LOOK_AND_FEEL_H
#define SERIALISTLOOPER_LOOK_AND_FEEL_H

#include <juce_gui_extra/juce_gui_extra.h>

enum Colors {
    object_on = 0xf000001
    , object_off = 0xf000002
    , outline_mouseover = 0xf000003
    , object_border_color = 0xf000004
    , text_color = 0xf000005

    , component_border_color = 0xf000006
    , component_background_color = 0xf000007

    , header_color = 0xf000008

};

class SerialistLookAndFeel : public juce::LookAndFeel_V4 {
public:

    static inline const juce::Font default_font = juce::Font("Arial", 12.0f, juce::Font::FontStyleFlags::plain);

    static inline const juce::Colour object_on = juce::Colour(0xffffb532);
    static inline const juce::Colour object_off = juce::Colour(0xffa5a5a5);
    static inline const juce::Colour text = juce::Colour(0xff000000);
    static inline const juce::Colour outline_mouseover = juce::Colour(0xfff5deb3);
    static inline const juce::Colour object_border = juce::Colour(0xff212121);
    static inline const juce::Colour component_border = juce::Colour(0xffcee5e8);
    static inline const juce::Colour component_background = juce::Colour(0xff595959);


    static void setup_look_and_feel_colors(juce::LookAndFeel& lnf) {

        // Global custom colors
        lnf.setColour(Colors::object_on, object_on);
        lnf.setColour(Colors::object_off, object_off);
        lnf.setColour(Colors::outline_mouseover, outline_mouseover);
        lnf.setColour(Colors::object_border_color, object_border);
        lnf.setColour(Colors::text_color, text);
        lnf.setColour(Colors::component_border_color, component_border);
        lnf.setColour(Colors::component_background_color, component_background);

        // Custom objects
        lnf.setColour(Colors::header_color, juce::Colour(0xff68b74d));

        // JUCE objects
        lnf.setColour(juce::Label::ColourIds::textColourId, text);

        lnf.setColour(juce::Slider::ColourIds::trackColourId, object_on);
        lnf.setColour(juce::Slider::ColourIds::backgroundColourId, object_off);
        lnf.setColour(juce::Slider::ColourIds::backgroundColourId, object_border);
        lnf.setColour(juce::Slider::ColourIds::textBoxTextColourId, text);
        lnf.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, object_border);


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
