

#ifndef SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
#define SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H

#include "interaction_visualizations.h"
#include "generative_component.h"
#include "parameter_policy.h"
#include "generator.h"
#include "oscillator_module.h"
#include "text_sequence_module.h"
#include "interpolation_module.h"


template<typename T>
class GeneratorModule : public GenerativeComponent
                        , public Connectable
                        , public juce::DragAndDropTarget {
public:
    enum class Layout {
        full
    };


    GeneratorModule(Generator<T>& generator
                    , std::unique_ptr<OscillatorModule> oscillator
                    , std::unique_ptr<InterpolationModule<T>> interpolator
                    , std::unique_ptr<TextSequenceModule<T>> sequence
                    , Variable<bool>& internal_enabled
                    , Layout layout = Layout::full)
            : m_generator(generator)
              , m_oscillator_socket(generator.get_cursor(), std::move(oscillator))
              , m_interpolator(generator.get_interpolation_strategy(), std::move(interpolator))
              , m_internal_sequence(std::move(sequence))
              , m_header(generator.get_identifier_as_string(), internal_enabled) {
        (void) layout;

        if (/*!m_oscillator_socket || !m_interpolator || */ !m_internal_sequence)
            throw std::runtime_error("A GeneratorModule requires all internal modules to be initialized");

        addAndMakeVisible(m_oscillator_socket);
        addAndMakeVisible(m_interpolator);
        addAndMakeVisible(m_internal_sequence.get());

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
               + TextSequenceModule<T>::height_of(TextSequenceModule<T>::Layout::generator_internal)
               + InterpolationModule<T>::height_of(InterpolationModule<T>::Layout::generator_internal)
               + 2 * DC::OBJECT_Y_MARGINS_COLUMN;
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
        return m_generator;
    }


    bool connectable_to(juce::Component& component) override {
        if (auto* socket = dynamic_cast<SocketWidget<T>*>(&component)) {
            return socket->connectable_to(*this);
        }
        return false;
    }


    bool connect(Connectable& connectable) override {
        if (auto* socket = dynamic_cast<SocketWidget<T>*>(&connectable)) {
            return socket->connect(*this);
        }
        return false;

    }


    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        if (auto* source = dragSourceDetails.sourceComponent.get()) {
            return connectable_to(*source);
        }
        return false;
    }


    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        if (auto* connectable = dynamic_cast<Connectable*>(dragSourceDetails.sourceComponent.get())) {
            connect(*connectable);
        }
    }


    void mouseDrag(const juce::MouseEvent&) override {
        juce::DragAndDropContainer* parent_drag_component =
                juce::DragAndDropContainer::findParentDragContainerFor(this);

        if (parent_drag_component && !parent_drag_component->isDragAndDropActive()) {
            parent_drag_component->startDragging("src", this, juce::ScaledImage(
                    createComponentSnapshot(juce::Rectangle<int>(1, 1))));
        }
    }


    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails&) override {
        std::cout << "enter SOCKET\n";
        m_interaction_visualizer.set_drag_and_dropping(true);
    }


    void itemDragExit(const juce::DragAndDropTarget::SourceDetails&) override {
        std::cout << "exit SOCKET\n";
        m_interaction_visualizer.set_drag_and_dropping(false);
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
        auto sequence_layout = TextSequenceModule<T>::Layout::generator_internal;
        auto interpolator_layout = InterpolationModule<T>::Layout::generator_internal;

        m_oscillator_socket.set_layout(static_cast<int>(oscillator_layout));
        m_internal_sequence->set_layout(static_cast<int>(sequence_layout));
        m_interpolator.set_layout(static_cast<int>(interpolator_layout));

        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
        bounds.reduce(DC::COMPONENT_LR_MARGINS, DC::COMPONENT_UD_MARGINS);

        m_oscillator_socket.setBounds(bounds.removeFromTop(OscillatorModule::height_of(oscillator_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_internal_sequence->setBounds(bounds.removeFromTop(TextSequenceModule<T>::height_of(sequence_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_interpolator.setBounds(bounds.removeFromTop(InterpolationModule<T>::height_of(interpolator_layout)));

        m_interaction_visualizer.setBounds(getLocalBounds());
    }


    Generator<T>& m_generator;

    SocketWidget<double> m_oscillator_socket;

    SocketWidget<InterpolationStrategy<T>> m_interpolator;
    std::unique_ptr<TextSequenceModule<T>> m_internal_sequence; // TODO: Replace with generic SequenceComponent

    HeaderWidget m_header;

    Layout m_layout = Layout::full;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};
};

#endif //SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
