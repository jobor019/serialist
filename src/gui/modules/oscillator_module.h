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
#include "socket_widget.h"
#include "interaction_visualizations.h"


class OscillatorModule : public GenerativeComponent {
public:

    using SliderLayout = SliderWidget<float>::Layout;
    using CbLayout = ComboBoxWidget<Oscillator::Type>::Layout;

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
              , m_type_socket(oscillator.get_type(), std::make_unique<ComboBoxWidget<Oscillator::Type>>(
                    internal_type
                    , std::vector<ComboBoxWidget<Oscillator::Type>::Entry>{
                            {  "phasor", Oscillator::Type::phasor}
                            , {"sin"   , Oscillator::Type::sin}
                            , {"sqr"   , Oscillator::Type::square}
                            , {"tri"   , Oscillator::Type::tri}}
                    , "type"
                    , CbLayout::label_left))
              , m_freq_socket(oscillator.get_freq(), std::make_unique<SliderWidget<float>>(
                    internal_freq, 0.0f, 20.0f, 0.01f, "freq", SliderLayout::label_left))
              , m_mul_socket(oscillator.get_mul(), std::make_unique<SliderWidget<float>>(
                    internal_mul, 0.0f, 20.0f, 0.01f, "mul", SliderLayout::label_left))
              , m_add_socket(oscillator.get_add(), std::make_unique<SliderWidget<float>>(
                    internal_add, 0.0f, 20.0f, 0.01f, "add", SliderLayout::label_left))
              , m_duty_socket(oscillator.get_duty(), std::make_unique<SliderWidget<float>>(
                    internal_duty, 0.0f, 1.0f, 0.01f, "duty", SliderLayout::label_left))
              , m_curve_socket(oscillator.get_curve(), std::make_unique<SliderWidget<float>>(
                    internal_curve, 0.0f, 1.0f, 0.01f, "curve", SliderLayout::label_left))
              , m_header(oscillator.get_parameter_handler().get_id(), internal_enabled)
              , m_layout(layout)
              , m_oscillator_view(oscillator) {

        setComponentID(oscillator.get_parameter_handler().get_id());

        addAndMakeVisible(m_oscillator_view);

        addAndMakeVisible(m_type_socket);
        addAndMakeVisible(m_freq_socket);
        addAndMakeVisible(m_mul_socket);
        addAndMakeVisible(m_add_socket);
        addAndMakeVisible(m_duty_socket);
        addAndMakeVisible(m_curve_socket);

        addAndMakeVisible(m_header);

        addAndMakeVisible(m_interaction_visualizer);
    }


    static std::string default_name() {
        return "oscillator";
    }


    static int width_of(Layout layout = Layout::full) {
        switch (layout) {
            case Layout::full:
                return 2 * DC::COMPONENT_LR_MARGINS
                       + SliderWidget<float>::default_width(SliderLayout::label_left, true);
            case Layout::generator_internal:
                return 4 * SliderWidget<float>::default_width(SliderLayout::label_below, true)
                       + 3 * DC::OBJECT_X_MARGINS_ROW;
        }

        return 0;
    }


    static int height_of(Layout layout = Layout::full) {
        switch (layout) {
            case Layout::full:
                return HeaderWidget::height_of()
                       + 2 * DC::COMPONENT_UD_MARGINS
                       + OscillatorView::height_of(OscillatorView::Layout::full)
                       + ComboBoxWidget<Oscillator::Type>::height_of(CbLayout::label_left) // type
                       + 5 * SliderWidget<float>::height_of(SliderLayout::label_left) // f, m, a, c, d
                       + 7 * DC::OBJECT_Y_MARGINS_COLUMN;
            case Layout::generator_internal:
                return 2 * SliderWidget<float>::height_of((SliderLayout::label_below))
                       + DC::OBJECT_Y_MARGINS_ROW;
        }
        std::cout << "oscillator: layout not implemented\n";
        return 0;
    }


    std::vector<std::unique_ptr<InteractionVisualization>> create_visualizations() {
        std::vector<std::unique_ptr<InteractionVisualization>> visualizations;
        visualizations.emplace_back(std::make_unique<ConnectVisualization>(*this));
        visualizations.emplace_back(std::make_unique<MoveVisualization>(*this));
        visualizations.emplace_back(std::make_unique<DeleteVisualization>(*this));
        return visualizations;
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
            layout_generator_internal();
        }
    }


    void full_layout() {
        auto slider_height = SliderWidget<float>::height_of(SliderLayout::label_left);
        auto y_margin = DC::OBJECT_Y_MARGINS_COLUMN;

        auto bounds = getLocalBounds();

        // header
        m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
        bounds.reduce(DC::COMPONENT_LR_MARGINS, DC::COMPONENT_UD_MARGINS);

        // layout
        m_oscillator_view.set_layout(OscillatorView::Layout::full);
        m_type_socket.set_layout(static_cast<int>(CbLayout::label_left));
        set_layout_for_all_sliders(SliderLayout::label_left);

        m_oscillator_view.setBounds(bounds.removeFromTop(OscillatorView::height_of(OscillatorView::Layout::full)));
        bounds.removeFromTop(y_margin);

        m_type_socket.setBounds(bounds.removeFromTop(slider_height));
        bounds.removeFromTop(y_margin);
        m_freq_socket.setBounds(bounds.removeFromTop(slider_height));
        bounds.removeFromTop(y_margin);
        m_mul_socket.setBounds(bounds.removeFromTop(slider_height));
        bounds.removeFromTop(y_margin);
        m_add_socket.setBounds(bounds.removeFromTop(slider_height));
        bounds.removeFromTop(y_margin);
        m_duty_socket.setBounds(bounds.removeFromTop(slider_height));
        bounds.removeFromTop(y_margin);
        m_curve_socket.setBounds(bounds.removeFromTop(slider_height));

        m_interaction_visualizer.setBounds(getLocalBounds());
    }


    void set_layout_for_all_sliders(SliderWidget<float>::Layout layout) {
        auto i = static_cast<int>(layout);
        m_freq_socket.set_layout(i);
        m_mul_socket.set_layout(i);
        m_add_socket.set_layout(i);
        m_duty_socket.set_layout(i);
        m_curve_socket.set_layout(i);
    }


    void layout_generator_internal() {
        auto label_position = SliderLayout::label_below;
        auto slider_width = SliderWidget<float>::default_width(SliderLayout::label_below, true);
        auto x_margins = DC::OBJECT_X_MARGINS_ROW;
        auto y_margins = DC::OBJECT_Y_MARGINS_ROW;
        auto slider_height = SliderWidget<float>::height_of(label_position);

        auto bounds = getLocalBounds();

        // layout
        m_oscillator_view.set_layout(OscillatorView::Layout::compact);
        m_type_socket.set_layout(static_cast<int>(CbLayout::label_below));
        set_layout_for_all_sliders(label_position);

        // row 1:
        auto row1 = bounds.removeFromTop(slider_height);
        m_oscillator_view.setBounds(row1.removeFromLeft(2 * slider_width + x_margins));
        row1.removeFromLeft(x_margins);
        m_duty_socket.setBounds(row1.removeFromLeft(slider_width));
        row1.removeFromLeft(x_margins);
        m_curve_socket.setBounds(row1);

        bounds.removeFromTop(y_margins);

        // row 2:
        auto row2 = bounds;
        m_type_socket.setBounds(row2.removeFromLeft(slider_width));
        row2.removeFromLeft(x_margins);
        m_freq_socket.setBounds(row2.removeFromLeft(slider_width));
        row2.removeFromLeft(x_margins);
        m_add_socket.setBounds(row2.removeFromLeft(slider_width));
        row2.removeFromLeft(x_margins);
        m_mul_socket.setBounds(row2.removeFromLeft(slider_width));
        row2.removeFromLeft(x_margins);
    }


    Oscillator& m_oscillator;


    SocketWidget<Oscillator::Type> m_type_socket;
    SocketWidget<float> m_freq_socket;
    SocketWidget<float> m_mul_socket;
    SocketWidget<float> m_add_socket;
    SocketWidget<float> m_duty_socket;
    SocketWidget<float> m_curve_socket;

    HeaderWidget m_header;

    Layout m_layout;

    OscillatorView m_oscillator_view;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};

//    ModuleEditState m_edit_state;
//    EditHighlightManager m_highlight_manager;

};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
