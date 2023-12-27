
#ifndef SERIALISTLOOPER_MODULE_STEREOTYPES_H
#define SERIALISTLOOPER_MODULE_STEREOTYPES_H

#include "state/generative_component_LEGACY_WIP.h"
#include "bases/connectable_module.h"
#include "interaction_visualizer_LEGACY.h"
#include "bases/connectable_dnd_controller.h"
#include "header_widget.h"
#include "socket_widget.h"

class ModuleStereotypeFuncs {
public:
    static void paint_module(juce::Graphics& g, const juce::LookAndFeel& lnf, const juce::Rectangle<int>& bounds) {
        g.setColour(lnf.findColour(Colors::component_background_color));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        g.setColour(lnf.findColour(Colors::component_border_color));
        g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
    }


    static std::vector<std::unique_ptr<InteractionVisualization>> node_visualizations(juce::Component& source) {
        std::vector<std::unique_ptr<InteractionVisualization>> visualizations;
        visualizations.emplace_back(std::make_unique<ConnectVisualization>(source));
        visualizations.emplace_back(std::make_unique<MoveVisualization>(source));
        visualizations.emplace_back(std::make_unique<DeleteVisualization>(source));
        return visualizations;
    }


    static std::vector<std::unique_ptr<InteractionVisualization>> root_visualizations(juce::Component& source) {
        std::vector<std::unique_ptr<InteractionVisualization>> visualizations;
        visualizations.emplace_back(std::make_unique<MoveVisualization>(source));
        visualizations.emplace_back(std::make_unique<DeleteVisualization>(source));
        return visualizations;
    }

};


// ==============================================================================================

class RootModuleBase : public GenerativeComponent {
public:
    explicit RootModuleBase(Root& main_generative
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


    void paint(juce::Graphics& g) override {
        ModuleStereotypeFuncs::paint_module(g, getLookAndFeel(), getLocalBounds());
    }


    void resized() final {
        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
        bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);

        on_resized(bounds);

        m_interaction_visualizer.setBounds(getLocalBounds());
    }


protected:
    virtual void on_resized(juce::Rectangle<int>& bounds) = 0;


    std::vector<std::unique_ptr<InteractionVisualization>> create_visualizations() {
        return ModuleStereotypeFuncs::root_visualizations(*this);
    }


private:
    Root& m_main_generative;

    HeaderWidget m_header;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};
};


// ==============================================================================================

template<typename OutputType = Facet>
class NodeModuleBase : public GenerativeComponent
                 , public ConnectableModule
                 , public juce::DragAndDropTarget {
public:
    explicit NodeModuleBase(Node<OutputType>& main_generative
                      , Variable<Facet, bool>* internal_enabled = nullptr
                      , Variable<Facet, float>* internal_num_voices = nullptr
                              , bool header_visible = true)
            : m_main_generative(main_generative)
              , m_header(m_main_generative.get_parameter_handler().get_id()
                         , internal_enabled
                         , nullptr
                         , internal_num_voices)
             , m_header_visible(header_visible) {

        setComponentID(m_main_generative.get_parameter_handler().get_id());

        if (m_header_visible)
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
        ModuleStereotypeFuncs::paint_module(g, getLookAndFeel(), getLocalBounds());
    }


    void resized() final {
        auto bounds = getLocalBounds();

        if (m_header_visible) {
            m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
            bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);
        }

        on_resized(bounds);

        m_interaction_visualizer.setBounds(getLocalBounds());
    }


protected:
    virtual void on_resized(juce::Rectangle<int>& bounds) = 0;


    std::vector<std::unique_ptr<InteractionVisualization>> create_visualizations() {
        return ModuleStereotypeFuncs::node_visualizations(*this);
    }


    void set_header_visibility(bool visible) {
        m_header_visible = visible;
        resized();
    }


private:
    Node<OutputType>& m_main_generative;

    HeaderWidget m_header;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};
    ConnectableDndController m_connectable_dnd_controller{*this, *this, &m_interaction_visualizer};

    bool m_header_visible = true;
};

#endif //SERIALISTLOOPER_MODULE_STEREOTYPES_H
