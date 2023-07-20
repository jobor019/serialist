

#ifndef SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H
#define SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "interpolator.h"
#include "variable.h"
#include "generative_component.h"
#include "interpolation_adapter.h"

class InterpolationModule : public GenerativeComponent {
public:

    using InterpType = typename InterpolationStrategy::Type;
    using SliderLayout = SliderWidget::Layout;
    using ComboBoxType = ComboBoxWidget<InterpType>;
    using CbLayout = ComboBoxType::Layout;

    enum class Layout {
        full = 0
        , generator_internal = 1
    };


    explicit InterpolationModule(InterpolationAdapter& interpolation_adapter
                                 , Variable<Facet, InterpType>& internal_type
                                 , Variable<Facet, float>& internal_pivot
                                 , Layout layout = Layout::full)
            : m_interpolation_adapter(interpolation_adapter)
              , m_type_socket(interpolation_adapter.get_type()
                              , std::make_unique<ComboBoxType>(
                            internal_type
                            , std::vector<ComboBoxType::Entry>{
                                    {  "cont", InterpType::continuation}
                                    , {"mod" , InterpType::modulo}
                                    , {"clip", InterpType::clip}
                                    , {"pass", InterpType::pass}}
                            , "type"
                            , CbLayout::label_left))
              , m_pivot_socket(interpolation_adapter.get_pivot(), std::make_unique<SliderWidget>(
                    internal_pivot, 0.0, 20.0, 0.01, false, "pivot", SliderLayout::label_left))
              , m_header(interpolation_adapter.get_parameter_handler().get_id())
              , m_layout(layout) {

        setComponentID(interpolation_adapter.get_parameter_handler().get_id());

        addAndMakeVisible(m_type_socket);
        addAndMakeVisible(m_pivot_socket);

        addAndMakeVisible(m_header);

        addAndMakeVisible(m_interaction_visualizer);

    }


    static int width_of(Layout layout = Layout::full) {
        (void) layout;
        return 2 * DC::COMPONENT_LR_MARGINS
               + 2 * DC::SLIDER_DEFAULT_WIDTH;
    }


    static int height_of(Layout layout = Layout::full) {
        switch (layout) {
            case Layout::full:
                return HeaderWidget::height_of()
                       + 2 * DC::COMPONENT_UD_MARGINS * 2
                       + 1 * SliderWidget::height_of(SliderLayout::label_below);
            case Layout::generator_internal:
                return DC::SLIDER_DEFAULT_HEIGHT;
        }
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


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
    }


    void resized() override {
        auto bounds = getLocalBounds();

        if (m_layout == Layout::full) {
            m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
            bounds.reduce(DC::COMPONENT_LR_MARGINS, DC::COMPONENT_UD_MARGINS);
        }

        m_pivot_socket.setBounds(bounds.removeFromLeft(DC::SLIDER_DEFAULT_WIDTH));
        bounds.removeFromLeft(DC::OBJECT_X_MARGINS_ROW);
        m_type_socket.setBounds(bounds);
    }


    Generative& get_generative() override { return m_interpolation_adapter; }


private:

    InterpolationAdapter& m_interpolation_adapter;

    SocketWidget<Facet> m_type_socket;
    SocketWidget<Facet> m_pivot_socket;

    HeaderWidget m_header;

    Layout m_layout;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};
};


#endif //SERIALISTLOOPER_INTERPOLATOR_COMPONENT_H
