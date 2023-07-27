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
#include "connectable_dnd_controller.h"


class OscillatorModule : public GenerativeComponent
                         , public juce::DragAndDropTarget
                         , public Connectable {
public:

    using SliderLayout = SliderWidget::Layout;
    using ComboBoxType = ComboBoxWidget<Oscillator::Type>;
    using CbLayout = ComboBoxType::Layout;

    enum class Layout {
        full
        , generator_internal
    };


    OscillatorModule(Oscillator& oscillator
                     , Variable<Facet, Oscillator::Type>& internal_type
                     , Variable<Facet, float>& internal_freq
                     , Variable<Facet, float>& internal_mul
                     , Variable<Facet, float>& internal_add
                     , Variable<Facet, float>& internal_duty
                     , Variable<Facet, float>& internal_curve
                     , Variable<Facet, bool>& internal_enabled
                     , Layout layout = Layout::full)
            : m_oscillator(oscillator)
              , m_type_socket(oscillator.get_type(), std::make_unique<ComboBoxType>(
                    internal_type
                    , std::vector<ComboBoxType::Entry>{
                            {  "phasor", Oscillator::Type::phasor}
                            , {"sin"   , Oscillator::Type::sin}
                            , {"sqr"   , Oscillator::Type::square}
                            , {"tri"   , Oscillator::Type::tri}}
                    , "type"
                    , CbLayout::label_left))
              , m_freq_socket(oscillator.get_freq(), std::make_unique<SliderWidget>(
                    internal_freq, 0.0, 20.0, 0.01, false, "freq", SliderLayout::label_left))
              , m_mul_socket(oscillator.get_mul(), std::make_unique<SliderWidget>(
                    internal_mul, 0.0, 20.0, 0.01, false, "mul", SliderLayout::label_left))
              , m_add_socket(oscillator.get_add(), std::make_unique<SliderWidget>(
                    internal_add, 0.0, 20.0, 0.01, false, "add", SliderLayout::label_left))
              , m_duty_socket(oscillator.get_duty(), std::make_unique<SliderWidget>(
                    internal_duty, 0.0, 1.0, 0.01, false, "duty", SliderLayout::label_left))
              , m_curve_socket(oscillator.get_curve(), std::make_unique<SliderWidget>(
                    internal_curve, 0.0, 1.0, 0.01, false, "curve", SliderLayout::label_left))
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
                       + SliderWidget::default_width(SliderLayout::label_left, true);
            case Layout::generator_internal:
                return 4 * SliderWidget::default_width(SliderLayout::label_below, true)
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
                       + ComboBoxType::height_of(CbLayout::label_left) // type
                       + 5 * SliderWidget::height_of(SliderLayout::label_left) // f, m, a, c, d
                       + 7 * DC::OBJECT_Y_MARGINS_COLUMN;
            case Layout::generator_internal:
                return 2 * SliderWidget::height_of((SliderLayout::label_below))
                       + DC::OBJECT_Y_MARGINS_ROW;
        }
        std::cout << "oscillator: layout not implemented\n";
        return 0;
    }


    std::vector<std::unique_ptr<InteractionVisualization>> create_visualizations() {
        std::vector<std::unique_ptr<InteractionVisualization>> visualizations;
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


    bool connect(Connectable& connectable) override {
        if (auto* socket = dynamic_cast<SocketWidget<Facet>*>(&connectable)) {
            return socket->connect(*this);
        }
        return false;
    }


    bool connectable_to(juce::Component& component) override {
        if (auto* socket = dynamic_cast<SocketWidget<Facet>*>(&component)) {
            return socket->connectable_to(*this);
        }
        return false;
    }


    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        return m_connectable_dnd_controller.is_interested_in(dragSourceDetails);
    }


    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_connectable_dnd_controller.item_dropped(dragSourceDetails);
    }


    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_connectable_dnd_controller.item_drag_enter(dragSourceDetails);
    }


    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_connectable_dnd_controller.item_drag_exit(dragSourceDetails);
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

        m_interaction_visualizer.setBounds(getLocalBounds());
    }


    void full_layout() {
        auto slider_height = SliderWidget::height_of(SliderLayout::label_left);
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
    }


    void set_layout_for_all_sliders(SliderWidget::Layout layout) {
        auto i = static_cast<int>(layout);
        m_freq_socket.set_layout(i);
        m_mul_socket.set_layout(i);
        m_add_socket.set_layout(i);
        m_duty_socket.set_layout(i);
        m_curve_socket.set_layout(i);
    }


    void layout_generator_internal() {
        auto label_position = SliderLayout::label_below;
        auto slider_width = SliderWidget::default_width(SliderLayout::label_below, true);
        auto x_margins = DC::OBJECT_X_MARGINS_ROW;
        auto y_margins = DC::OBJECT_Y_MARGINS_ROW;
        auto slider_height = SliderWidget::height_of(label_position);

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


    SocketWidget<Facet> m_type_socket;
    SocketWidget<Facet> m_freq_socket;
    SocketWidget<Facet> m_mul_socket;
    SocketWidget<Facet> m_add_socket;
    SocketWidget<Facet> m_duty_socket;
    SocketWidget<Facet> m_curve_socket;

    HeaderWidget m_header;

    Layout m_layout;

    OscillatorView m_oscillator_view;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};
    ConnectableDndController m_connectable_dnd_controller{*this, *this, &m_interaction_visualizer};

};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
