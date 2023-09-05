
#ifndef SERIALISTLOOPER_CONNECTABLE_MODULE_BASE_H
#define SERIALISTLOOPER_CONNECTABLE_MODULE_BASE_H

#include "generative_component.h"
#include "connectable_module.h"
#include "interaction_visualizer.h"
#include "connectable_dnd_controller.h"
#include "header_widget.h"
#include "socket_widget.h"

template<typename OutputType = Facet>
class ConnectableModuleBase : public GenerativeComponent
                              , public ConnectableModule
                              , public juce::DragAndDropTarget {
public:
    explicit ConnectableModuleBase(Generative& main_generative
                                   , Variable<Facet, bool>* internal_enabled = nullptr
                                   , Variable<Facet, float>* internal_num_voices = nullptr)
            : m_main_generative(main_generative)
              , m_header(m_main_generative.get_parameter_handler().get_id()
                         , internal_enabled
                         , nullptr
                         , internal_num_voices) {

        setComponentID(m_main_generative.get_parameter_handler().get_id());

        addAndMakeVisible(m_header);
        addAndMakeVisible(m_interaction_visualizer);
    }


    Generative& get_generative() override {
        return m_main_generative;
    }


    bool connectable_to(juce::Component& component) override {
        if (auto* socket = dynamic_cast<SocketWidget<OutputType>*>(&component)) {
            return socket->connectable_to(*this);
        }
        return false;
    }


    bool connect(ConnectableModule& connectable) override {
        if (auto* socket = dynamic_cast<SocketWidget<OutputType>*>(&connectable)) {
            return socket->connect(*this);
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


    void paint(juce::Graphics& g) override {
        g.setColour(getLookAndFeel().findColour(Colors::component_background_color));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
        g.setColour(getLookAndFeel().findColour(Colors::component_border_color));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
    }


    void resized() final {
        auto bounds = getLocalBounds();

        if (m_header_visible) {
            m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
            bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);
        }

        on_resized(bounds);
    }


protected:
    virtual void on_resized(juce::Rectangle<int>& bounds) = 0;


    virtual std::vector<std::unique_ptr<InteractionVisualization>> create_visualizations() {
        std::vector<std::unique_ptr<InteractionVisualization>> visualizations;
        visualizations.emplace_back(std::make_unique<MoveVisualization>(*this));
        visualizations.emplace_back(std::make_unique<DeleteVisualization>(*this));
        return visualizations;
    }


    void set_header_visibility(bool visible) {
        m_header_visible = visible;
    }


private:
    Generative& m_main_generative;

    HeaderWidget m_header;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};
    ConnectableDndController m_connectable_dnd_controller{*this, *this, &m_interaction_visualizer};

    bool m_header_visible = true;
};

#endif //SERIALISTLOOPER_CONNECTABLE_MODULE_BASE_H
