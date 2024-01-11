
#ifndef SERIALISTLOOPER_MODULE_STEREOTYPES_H
#define SERIALISTLOOPER_MODULE_STEREOTYPES_H

#include "state/generative_component.h"
#include "bases/connectable_module.h"
#include "header_widget.h"
#include "socket_widget.h"
#include "state/interaction_visualizations.h"
#include "state/state_handler.h"
#include "state/interaction_visualizer.h"

class ModuleStereotypeFuncs {
public:

    using KeyCodes = ConfigurationLayerKeyboardShortcuts;


    static void paint_module(juce::Graphics& g, const juce::LookAndFeel& lnf, const juce::Rectangle<int>& bounds) {
        g.setColour(lnf.findColour(Colors::component_background_color));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        g.setColour(lnf.findColour(Colors::component_border_color));
        g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
    }


    static Vec<std::unique_ptr<InteractionVisualization>> shared_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>(
                std::make_unique<MoveVisualization>()
                , std::make_unique<DeleteVisualization>()
        );
    }


    static Vec<std::unique_ptr<InteractionVisualization>> node_visualizations() {
        return {}; // , std::make_unique<ConnectVisualization>() // TODO: Update

    }


    static Vec<std::unique_ptr<InteractionVisualization>> root_visualizations() {
        return {};
    }


    static Vec<TriggerableState> shared_states() {
        return Vec<TriggerableState>(
                TriggerableState{std::make_unique<NoKeyCondition>(), States::Default}
                , TriggerableState{std::make_unique<KeyCondition>(KeyCodes::MOVE_KEY), States::Move}
                , TriggerableState{std::make_unique<KeyCondition>(KeyCodes::DELETE_KEY), States::Delete}
        );
    }


    static Vec<TriggerableState> node_states() {
        return {}; // , TriggerableState{std::make_unique<KeyCondition>(KeyCodes::CONNECT_KEY), States::Connect}
    }


    static Vec<TriggerableState> root_states() {
        return {};
    }

};


// ==============================================================================================

template<typename GenerativeType>
class ModuleBase : public GenerativeComponent, public Stateful {
public:
    ModuleBase(
            Generative& main_generative
            , StateHandler& parent_handler
            , Variable<Facet, bool>* internal_enabled = nullptr
            , Variable<Facet, float>* internal_num_voices = nullptr
            , Vec<
            std::unique_ptr<InteractionVisualization>> visualizations = ModuleStereotypeFuncs::shared_visualizations()
            , Vec<TriggerableState> states = ModuleStereotypeFuncs::shared_states()
            , const State& initial_state = States::Default
            , bool header_visible = true
    )
            : m_main_generative(main_generative)
              , m_header(m_main_generative.get_parameter_handler().get_id()
                         , internal_enabled
                         , nullptr
                         , internal_num_voices)
              , m_interaction_visualizer(std::move(visualizations))
              , m_state_handler(&parent_handler
                                , *this
                                , {*this, m_interaction_visualizer}
                                , std::move(states)
                                , initial_state)
              , m_header_visible(header_visible) {
        static_assert(std::is_base_of_v<Generative, GenerativeType>, "GenerativeType must be derived from Generative");

        setComponentID(m_main_generative.get_parameter_handler().get_id());

        addChildComponent(m_header);
        m_header.setVisible(m_header_visible);

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

        if (m_header_visible) {
            m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
            bounds.reduce(DimensionConstants::COMPONENT_LR_MARGINS, DimensionConstants::COMPONENT_UD_MARGINS);
        }

        on_resized(bounds);

        m_interaction_visualizer.setBounds(getLocalBounds());
    }


    StateHandler& get_state_handler() {
        return m_state_handler;
    }


    InteractionVisualizer& get_visualizer() {
        return m_interaction_visualizer;
    }


protected:
    virtual void on_resized(juce::Rectangle<int>& bounds) = 0;


private:
    GenerativeType& m_main_generative;

    HeaderWidget m_header;


    InteractionVisualizer m_interaction_visualizer;
    StateHandler m_state_handler;

    bool m_header_visible;
};

// ==============================================================================================

using RootModuleBase = ModuleBase<Root>;


// ==============================================================================================

template<typename OutputType = Facet>
class NodeModuleBase : public ModuleBase<Node<OutputType>>
                       , public ConnectableModule
                       , public juce::DragAndDropTarget {
public:
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

    // TODO: Update
//
//    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
//        return m_connectable_dnd_controller.is_interested_in(dragSourceDetails);
//    }
//
//
//    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
//        m_connectable_dnd_controller.item_dropped(dragSourceDetails);
//    }
//
//
//    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
//        m_connectable_dnd_controller.item_drag_enter(dragSourceDetails);
//    }
//
//
//    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
//        m_connectable_dnd_controller.item_drag_exit(dragSourceDetails);
//    }


private:
//    ConnectableDndController m_connectable_dnd_controller{*this, *this, get_visualizer()}; TODO: Update

    bool m_header_visible = true;
};

#endif //SERIALISTLOOPER_MODULE_STEREOTYPES_H
