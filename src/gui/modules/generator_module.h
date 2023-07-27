

#ifndef SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
#define SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H

#include "interaction_visualizations.h"
#include "generative_component.h"
#include "parameter_policy.h"
#include "generator.h"
#include "oscillator_module.h"
#include "text_sequence_module.h"
#include "interpolation_module.h"


template<typename OutputType, typename InternalSequenceType = OutputType>
class GeneratorModule : public GenerativeComponent
                        , public Connectable
                        , public juce::DragAndDropTarget {
public:
    enum class Layout {
        full
    };


    GeneratorModule(Generator<OutputType>& generator
                    , std::unique_ptr<OscillatorModule> oscillator
                    , std::unique_ptr<InterpolationModule> interpolator
                    , std::unique_ptr<TextSequenceModule<OutputType, InternalSequenceType>> sequence
                    , Variable<Facet, bool>& internal_enabled
                    , Layout layout = Layout::full)
            : m_generator(generator)
              , m_oscillator_socket(generator.get_cursor(), std::move(oscillator))
              , m_interpolator(generator.get_interpolation_strategy(), std::move(interpolator))
              , m_sequence_socket(generator.get_sequence(), std::move(sequence))
              , m_header(generator.get_parameter_handler().get_id(), internal_enabled) {
        (void) layout;

        setComponentID(generator.get_parameter_handler().get_id());

        addAndMakeVisible(m_oscillator_socket);
        addAndMakeVisible(m_interpolator);
        addAndMakeVisible(m_sequence_socket);

        addAndMakeVisible(m_header);
        addAndMakeVisible(m_interaction_visualizer);
    }


    static std::string default_name() {
        return "generator";
    }


    static int width_of(Layout layout = Layout::full) {
        (void) layout;
        return OscillatorModule::width_of(OscillatorModule::Layout::generator_internal);
    }


    static int height_of(Layout layout = Layout::full) {
        (void) layout;
        return HeaderWidget::height_of()
               + 2 * DC::COMPONENT_UD_MARGINS
               + OscillatorModule::height_of(OscillatorModule::Layout::generator_internal)
               + DC::DEFAULT_SEQUENCE_HEIGHT
               + InterpolationModule::height_of(InterpolationModule::Layout::generator_internal)
               + 2 * DC::OBJECT_Y_MARGINS_COLUMN;
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
        return m_generator;
    }


    bool connectable_to(juce::Component& component) override {
        if (auto* socket = dynamic_cast<SocketWidget<OutputType>*>(&component)) {
            return socket->connectable_to(*this);
        }
        return false;
    }


    bool connect(Connectable& connectable) override {
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
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
    }


    void resized() override {
        if (m_layout == Layout::full) {
            full_layout();
        }
    }


private:
    void full_layout() {
        // layout
        auto oscillator_layout = OscillatorModule::Layout::generator_internal;
        auto sequence_layout = TextSequenceModule<OutputType>::Layout::generator_internal;
        auto interpolator_layout = InterpolationModule::Layout::generator_internal;

        m_oscillator_socket.set_layout(static_cast<int>(oscillator_layout));
        m_sequence_socket.set_layout(static_cast<int>(sequence_layout));
        m_interpolator.set_layout(static_cast<int>(interpolator_layout));

        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
        bounds.reduce(DC::COMPONENT_LR_MARGINS, DC::COMPONENT_UD_MARGINS);

        m_oscillator_socket.setBounds(bounds.removeFromTop(OscillatorModule::height_of(oscillator_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_sequence_socket.setBounds(bounds.removeFromTop(TextSequenceModule<OutputType>::height_of(sequence_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_interpolator.setBounds(bounds.removeFromTop(InterpolationModule::height_of(interpolator_layout)));

        m_interaction_visualizer.setBounds(getLocalBounds());
    }


    Generator<OutputType>& m_generator;

    SocketWidget<Facet> m_oscillator_socket;

    SocketWidget<InterpolationStrategy> m_interpolator;
    DataSocketWidget<OutputType> m_sequence_socket;

    HeaderWidget m_header;

    Layout m_layout = Layout::full;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};
    ConnectableDndController m_connectable_dnd_controller{*this, *this, &m_interaction_visualizer};
};

#endif //SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
