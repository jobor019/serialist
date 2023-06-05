

#ifndef SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H
#define SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "interpolator.h"
#include "variable.h"
#include "generative_component.h"

template<typename T>
class InterpolationModule : public GenerativeComponent
                            , juce::Slider::Listener
                            , juce::ComboBox::Listener {
public:
    enum class Layout {
        full = 0
        , generator_internal = 1
    };


    explicit InterpolationModule(Variable<InterpolationStrategy<T>>& strategy
                                 , Layout layout = Layout::full)
            : m_strategy(strategy)
              , m_header(strategy.get_identifier_as_string())
              , m_layout(layout) {

        initialize_components();

    }


    static int width_of(Layout layout = Layout::full) {
        (void) layout;
        return DimensionConstants::COMPONENT_LR_MARGINS * 2 + DimensionConstants::SLIDER_DEFAULT_WIDTH * 2;
    }


    static int height_of(Layout layout = Layout::full) {
        if (layout == Layout::full) {
            return HeaderWidget::height_of()
                   + 2 * DimensionConstants::COMPONENT_UD_MARGINS * 2
                   + DimensionConstants::SLIDER_DEFAULT_HEIGHT;
        }
        std::cout << "interpolation module: layout not implemented\n";
        return 0;
    }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        resized();
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
    }


    void resized() override {
        if (m_layout == Layout::full) {
            auto bounds = getLocalBounds();

            m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
            bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);

            m_pivot.setBounds(bounds.removeFromLeft(DimensionConstants::SLIDER_DEFAULT_WIDTH));
            bounds.removeFromLeft(DimensionConstants::OBJECT_X_MARGINS_ROW);
            m_type.setBounds(bounds);
        } else {
            std::cout << "internal layout not implemented for interpolationstrategy\n";
        }
    }


    Generative& get_generative() override { return m_strategy; }


private:
    void initialize_components() {
        addAndMakeVisible(m_header);

        m_type.addItem("continuation", static_cast<int>(InterpolationStrategy<T>::Type::continuation));
        m_type.addItem("modulo", static_cast<int>(InterpolationStrategy<T>::Type::modulo));
        m_type.addItem("clip", static_cast<int>(InterpolationStrategy<T>::Type::clip));
        m_type.addItem("pass", static_cast<int>(InterpolationStrategy<T>::Type::pass));
        m_type.setSelectedId(static_cast<int>(m_strategy.get_value().get_type()), juce::dontSendNotification);
        m_type.addListener(this);
        addAndMakeVisible(m_type);

        m_pivot.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
        m_pivot.setValue(m_strategy.get_value().get_pivot(), juce::dontSendNotification);
        m_pivot.setNumDecimalPlacesToDisplay(std::is_integral_v<T> ? 0 : 2);
        m_pivot.setTextBoxIsEditable(false);
        m_pivot.addListener(this);
        addAndMakeVisible(m_pivot);
    }


    void sliderValueChanged(juce::Slider*) override {
        value_changed();
    }


    void comboBoxChanged(juce::ComboBox*) override {
        value_changed();
    }


    void value_changed() {
        typename InterpolationStrategy<T>::Type type{m_type.getSelectedId()};
        std::cout << "new value: " << static_cast<int>(type) << ":" << static_cast<T>(m_pivot.getValue()) << "\n";
        m_strategy.set_value(InterpolationStrategy<T>(type, static_cast<T>(m_pivot.getValue())));
    }


    Variable<InterpolationStrategy<T>>& m_strategy;

    HeaderWidget m_header;

    Layout m_layout;

    juce::ComboBox m_type;
    juce::Slider m_pivot;

};


#endif //SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H
