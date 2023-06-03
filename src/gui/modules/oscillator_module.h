#ifndef SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
#define SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>

#include "oscillator.h"
#include "generative_component.h"
#include "widgets/slider_widget.h"
#include "widgets/toggle_button_widget.h"
#include "widgets/header_widget.h"
#include "widgets/combobox_widget.h"
#include "views/oscillator_view.h"
#include "socket_component.h"


class OscillatorModule : public GenerativeComponent {
public:

    enum class Layout {
        full
        , generator_internal
    };


    OscillatorModule(Oscillator& oscillator
                     , Variable<Oscillator::Type>& internal_type
                     , Variable<float>& internal_freq
                     , Variable<float>& internal_mul
                     , Variable<float>& internal_add
                     , Variable<float>& internal_duty
                     , Variable<float>& internal_curve
                     , Variable<bool>& internal_enabled
                     , Layout layout = Layout::full)
            : m_oscillator(oscillator)
              , m_type_socket(oscillator.get_type()
                              , std::make_unique<ComboBoxWidget<Oscillator::Type>>(
                            internal_type
                            , std::vector<ComboBoxWidget<Oscillator::Type>::Entry>{
                                    {  "phasor", Oscillator::Type::phasor}
                                    , {"sin"   , Oscillator::Type::sin}
                                    , {"sqr"   , Oscillator::Type::square}
                                    , {"tri"   , Oscillator::Type::tri}
                            }))
              , m_freq_socket(oscillator.get_freq(), std::make_unique<SliderWidget<float>>(internal_freq))
              , m_mul_socket(oscillator.get_mul(), std::make_unique<SliderWidget<float>>(internal_mul))
              , m_add_socket(oscillator.get_add(), std::make_unique<SliderWidget<float>>(internal_add))
              , m_duty_socket(oscillator.get_duty(), std::make_unique<SliderWidget<float>>(internal_duty))
              , m_curve_socket(oscillator.get_curve(), std::make_unique<SliderWidget<float>>(internal_curve))
              , m_header(oscillator.get_identifier().toString().toStdString(), internal_enabled)
              , m_layout(layout)
              , m_oscillator_view(oscillator) {

        addAndMakeVisible(m_oscillator_view);

        addAndMakeVisible(m_type_socket);
        addAndMakeVisible(m_freq_socket);
        addAndMakeVisible(m_mul_socket);
        addAndMakeVisible(m_add_socket);
        addAndMakeVisible(m_duty_socket);
        addAndMakeVisible(m_curve_socket);

        addAndMakeVisible(m_header);
    }


    static int width_of(Layout layout) {
        if (layout == Layout::full) {
            return 2 * DimensionConstants::COMPONENT_LR_MARGINS
                   + SliderWidget<float>::default_width(SliderWidget<float>::Layout::label_left, true);
        }
        std::cout << "oscillator: layout not implemented\n";
        return 0;
    }


    static int height_of(Layout layout) {
        if (layout == Layout::full) {
            return HeaderWidget::height_of()
                   + 2 * DimensionConstants::COMPONENT_UD_MARGINS
                   + OscillatorView::height_of(OscillatorView::Layout::full)
                   + ComboBoxWidget<float>::height_of(ComboBoxWidget<float>::Layout::label_left) // type
                   + 5 * SliderWidget<float>::height_of(SliderWidget<float>::Layout::label_left) // f, m, a, c, d
                   + 7 * DimensionConstants::OBJECT_Y_MARGINS_COLUMN;
        }
        std::cout << "oscillator: layout not implemented\n";
        return 0;
    }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        resized();
    }


    Generative& get_generative() override {
        return m_oscillator;
    }


private:
    void paint(juce::Graphics& g) override {
        g.setColour(getLookAndFeel().findColour(Colors::component_background_color));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
        g.setColour(getLookAndFeel().findColour(Colors::component_border_color));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
    }


    void resized() override {
        if (m_layout == Layout::full) {
            full_layout();
        } else if (m_layout == Layout::generator_internal) {
            generator_internal_layout();
        }
    }


    void full_layout() {
        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
        bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);

        m_oscillator_view.set_layout(OscillatorView::Layout::full);
        m_oscillator_view.setBounds(bounds.removeFromTop(OscillatorView::height_of(OscillatorView::Layout::full)));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

        layout_socket(m_type_socket, bounds, static_cast<int>(ComboBoxWidget<Oscillator::Type>::Layout::label_left));
        layout_socket(m_freq_socket, bounds, static_cast<int>(SliderWidget<float>::Layout::label_left));
        layout_socket(m_mul_socket, bounds, static_cast<int>(SliderWidget<float>::Layout::label_left));
        layout_socket(m_add_socket, bounds, static_cast<int>(SliderWidget<float>::Layout::label_left));
        layout_socket(m_duty_socket, bounds, static_cast<int>(SliderWidget<float>::Layout::label_left));
        layout_socket(m_curve_socket, bounds, static_cast<int>(SliderWidget<float>::Layout::label_left));

    }


    template<typename SocketType>
    void layout_socket(SocketComponent<SocketType>& socket, juce::Rectangle<int>& bounds, int layout) {
        socket.get_internal().set_layout(layout);
        socket.setBounds(bounds.removeFromTop(SliderWidget<float>::height_of(SliderWidget<float>::Layout::label_left)));
        bounds.removeFromTop(DimensionConstants::OBJECT_Y_MARGINS_COLUMN);

    }


    void generator_internal_layout() {
//        auto bounds = getLocalBounds().reduced(5, 8);
//
//        auto col1 = bounds.removeFromLeft(bounds.proportionOfWidth(0.45f));
//
//        m_oscillator_view.set_layout(OscillatorView::Layout::compact);
//        m_oscillator_view.setBounds(col1.removeFromTop(m_oscillator_view.default_height()));
//        col1.removeFromTop(FULL_LAYOUT_SPACING);
//
//        m_type_socket.set_label_position(ComboBoxWidget<Oscillator::Type>::LabelPosition::bottom);
//        m_type_socket.setBounds(col1.removeFromLeft(col1.getWidth() / 2));
//        col1.removeFromLeft(FULL_LAYOUT_SPACING);
//
//        m_freq_socket.get_internal().set_layout(static_cast<int>(SliderWidget<float>::Layout::label_below));
//        m_freq_socket.setBounds(col1);
//
//        auto col2 = bounds.removeFromLeft(bounds.getWidth() / 2);
//        m_mul_socket.get_internal().set_layout(static_cast<int>(SliderWidget<float>::Layout::label_below));
//        m_mul_socket.setBounds(col2.removeFromTop(m_mul_socket.default_height()));
//        col2.removeFromTop(FULL_LAYOUT_SPACING);
//
//        m_add_socket.get_internal().set_layout(static_cast<int>(SliderWidget<float>::Layout::label_below));
//        m_add_socket.setBounds(col2.removeFromTop(m_add_socket.default_height()));
//        col2.removeFromTop(FULL_LAYOUT_SPACING);
//
//        m_duty_socket..get_internal().set_layout(static_cast<int>(SliderWidget<float>::Layout::label_below));
//        m_duty_socket.setBounds(bounds.removeFromTop(m_duty_socket.default_height()));
//        bounds.removeFromTop(FULL_LAYOUT_SPACING);
//
//        m_curve_socket..get_internal().set_layout(static_cast<int>(SliderWidget<float>::Layout::label_below));
//        m_curve_socket.setBounds(bounds.removeFromTop(m_curve_socket.default_height()));
//        bounds.removeFromTop(FULL_LAYOUT_SPACING);
    }


    Oscillator& m_oscillator;


    SocketComponent<Oscillator::Type> m_type_socket;
    SocketComponent<float> m_freq_socket;
    SocketComponent<float> m_mul_socket;
    SocketComponent<float> m_add_socket;
    SocketComponent<float> m_duty_socket;
    SocketComponent<float> m_curve_socket;

    HeaderWidget m_header;

    Layout m_layout;

    OscillatorView m_oscillator_view;


};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
