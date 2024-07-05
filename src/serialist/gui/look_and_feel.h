

#ifndef SERIALISTLOOPER_LOOK_AND_FEEL_H
#define SERIALISTLOOPER_LOOK_AND_FEEL_H

#include <juce_gui_extra/juce_gui_extra.h>

namespace serialist {

enum Colors {
    object_on = 0xf000001
    , object_off = 0xf000002
    , object_background = 0xf000003
    , outline_mouseover = 0xf000004
    , object_border_color = 0xf000005
    , text_color = 0xf000006
    , bright_text_color = 0xf000007

    , component_border_color = 0xf000008
    , component_background_color = 0xf000009

    , header_color = 0xf00000a

};

class SerialistLookAndFeel : public juce::LookAndFeel_V4 {
public:
    static inline const int FONT_SIZE = 12;
    static inline const juce::Font DEFAULT_FONT = juce::Font("Arial", FONT_SIZE, juce::Font::FontStyleFlags::plain);

    static inline const juce::Colour OBJECT_ON_C = juce::Colour(0xffffb532);
    static inline const juce::Colour OBJECT_OFF_C = juce::Colour(0xffa5a5a5);
    static inline const juce::Colour OBJECT_BACKGROUND_C = juce::Colour(0xffc3c3c3);
    static inline const juce::Colour OBJECT_MOUSEOVER_C = juce::Colour(0xfff5deb3);
    static inline const juce::Colour OBJECT_BORDER_C = juce::Colour(0xff212121);

    static inline const juce::Colour TEXT_C = juce::Colour(0xff000000);
    static inline const juce::Colour BRIGHT_TEXT_C = juce::Colour(0xffeeeeee);

    static inline const juce::Colour COMPONENT_BORDER_C = juce::Colour(0xffcee5e8);
    static inline const juce::Colour COMPONENT_BACKGROUND_C = juce::Colour(0xff595959);


    static void setup_look_and_feel_colors(juce::LookAndFeel& lnf) {

        // Global custom colors
        lnf.setColour(Colors::object_on, OBJECT_ON_C);
        lnf.setColour(Colors::object_off, OBJECT_OFF_C);
        lnf.setColour(Colors::object_background, OBJECT_BACKGROUND_C);
        lnf.setColour(Colors::outline_mouseover, OBJECT_MOUSEOVER_C);
        lnf.setColour(Colors::object_border_color, OBJECT_BORDER_C);
        lnf.setColour(Colors::text_color, TEXT_C);
        lnf.setColour(Colors::bright_text_color, BRIGHT_TEXT_C);
        lnf.setColour(Colors::component_border_color, COMPONENT_BORDER_C);
        lnf.setColour(Colors::component_background_color, COMPONENT_BACKGROUND_C);

        // Custom objects
        lnf.setColour(Colors::header_color, juce::Colour(0xff68b74d));

        // juce::Label
        lnf.setColour(juce::Label::ColourIds::textColourId, TEXT_C);

        // juce::Slider
        lnf.setColour(juce::Slider::ColourIds::trackColourId, OBJECT_ON_C);
        lnf.setColour(juce::Slider::ColourIds::backgroundColourId, OBJECT_OFF_C);
        lnf.setColour(juce::Slider::ColourIds::textBoxTextColourId, TEXT_C);
        lnf.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, OBJECT_BORDER_C);

        // juce::ComboBox
        lnf.setColour(juce::ComboBox::ColourIds::backgroundColourId, OBJECT_ON_C);
        lnf.setColour(juce::ComboBox::ColourIds::arrowColourId, TEXT_C);
        lnf.setColour(juce::ComboBox::ColourIds::textColourId, TEXT_C);
        lnf.setColour(juce::ComboBox::ColourIds::outlineColourId, OBJECT_BORDER_C);


    }


    juce::Font getLabelFont(juce::Label&) override {
        return DEFAULT_FONT;
    }


    juce::Font getTextButtonFont(juce::TextButton&, int) override {
        return DEFAULT_FONT;
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
        g.setFont(DEFAULT_FONT);

        if (!button.isEnabled())
            g.setOpacity(0.5f);

        g.drawFittedText(button.getButtonText(), button.getLocalBounds().withTrimmedLeft(2)
                .withTrimmedRight(2), juce::Justification::centred, 10);
    }


private:

};

} // namespace serialist

#endif //SERIALISTLOOPER_LOOK_AND_FEEL_H
