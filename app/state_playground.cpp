

#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include "core/param/parameter_policy.h"
#include "key_state.h"
#include "gui/interaction/input_handler.h"
#include "interaction/interaction_visualizer.h"

class DummyDndInterface : public juce::Component {
public:
    virtual bool supports_drag_to(const DndTarget::Details& source) const = 0;
    virtual void set_text(const std::string& text) = 0;
    virtual void reset() = 0;
};

class DragAndDropMode : public InputMode {
public:
    enum class State {
        enabled = 0
        , hovering = 1
        , dragging_from = 2
        , dragging_to = 3
        , valid_target = 4
    };

    class Visualization : public InteractionVisualization {
    public:
        Visualization() {
            addChildComponent(m_enabled);
            addChildComponent(m_hover);
            addChildComponent(m_drag_to);
            addChildComponent(m_valid_target);
        }


        void resized() override {
            m_enabled.setBounds(getLocalBounds());
            m_hover.setBounds(getLocalBounds());
            m_drag_to.setBounds(getLocalBounds());
            m_valid_target.setBounds(getLocalBounds());
        }


        void state_changed(InputMode* active_mode, int state, AlphaMask& alpha) override {
            auto is_active = active_mode && active_mode->is<DragAndDropMode>();
            m_enabled.setVisible(is_active && state == static_cast<int>(State::enabled));
            m_hover.setVisible(is_active && state == static_cast<int>(State::hovering));
            m_drag_to.setVisible(is_active && state == static_cast<int>(State::dragging_to));
            m_valid_target.setVisible(is_active && state == static_cast<int>(State::valid_target));

            if (is_active && state == static_cast<int>(State::dragging_from)) {
                alpha.set_alpha(0.4f);
            } else {
                alpha.set_alpha(1.0f);
            }
        }


    private:
        BorderHighlight m_enabled{juce::Colours::darkgreen.withAlpha(0.6f), 1};
        BorderHighlight m_hover{juce::Colours::darkgreen, 2};
        FillHighlight m_drag_to{juce::Colours::darkturquoise.withAlpha(0.3f)};
        BorderHighlight m_valid_target{juce::Colours::palegoldenrod.withAlpha(0.1f)};
    };


    explicit DragAndDropMode(DummyDndInterface& dnd_component) : m_dnd_component(dnd_component) {}


    DragBehaviour get_drag_behaviour() override {
        return {DragBehaviour::DragAndDropFrom{}};
    }

    bool has_drag_image() const override {
        return true;
    }

    std::optional<juce::ScaledImage> get_drag_image() const override {
        return juce::ScaledImage(m_dnd_component.createComponentSnapshot(m_dnd_component.getLocalBounds()));
    }


    bool supports_drag_to(const DndTarget::Details& source) const override {
        return m_dnd_component.supports_drag_to(source);
    }


    std::optional<int> mouse_state_changed(const MouseState& mouse_state, const DragAndDropState& dnd_state) override {
        std::cout << "mouse state changed\n";
        if (dnd_state.is_dragging_to) {
            m_dnd_component.set_text("Drop here!");
            return static_cast<int>(State::dragging_to);

        } else if (dnd_state.is_dragging_from) {
            m_dnd_component.set_text("Dragging (from)...");
            return static_cast<int>(State::dragging_from);

        } else if (!mouse_state.is_over_component()) {
            m_dnd_component.set_text("Drag enabled");
            return static_cast<int>(State::enabled);

        } else if (mouse_state.is_directly_over_component()) {
            m_dnd_component.set_text("Click to start drag!");
            return static_cast<int>(State::hovering);

        } else {
            return std::nullopt;
        }
    }


    std::optional<int> mouse_position_changed(const MouseState&, const DragAndDropState&) override {
        return std::nullopt;
    }


    std::optional<int> input_event_registered(std::unique_ptr<InputEvent> input_event) override {
        if (input_event->is<DragDropped>()) {
            std::cout << "Dragdropped\n";
        }
        return std::nullopt;
    }


    void reset() override {
        m_dnd_component.reset();
    }


private:
    DummyDndInterface& m_dnd_component;
};


// ==============================================================================================

class DragAndDroppableComponent : public DummyDndInterface, public juce::DragAndDropTarget  {
public:
    DragAndDroppableComponent(const std::string& initial_text, Vec<std::unique_ptr<InputCondition>> conditions)
            : m_default_text(initial_text)
              , m_text(initial_text)
              , m_input_handler(nullptr, *this, default_modes(*this, std::move(conditions)), {std::ref(m_visualizer)}) {
        addAndMakeVisible(m_visualizer);
    }


    static InputModeMap default_modes(DummyDndInterface& managed_component
                                      , Vec<std::unique_ptr<InputCondition>> conditions) {
        InputModeMap map;
        map.add(std::move(conditions), std::make_unique<DragAndDropMode>(managed_component));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{std::make_unique<DragAndDropMode::Visualization>()};
    }


    void resized() override {
        m_visualizer.setBounds(getLocalBounds());
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::lightskyblue);
        g.setColour(juce::Colours::whitesmoke);
        g.drawFittedText(m_text, getLocalBounds(), juce::Justification::centred, 1);
    }


    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_input_handler.drag_enter(dragSourceDetails);
    }


    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_input_handler.drag_exit(dragSourceDetails);

    }


    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_input_handler.drag_move(dragSourceDetails);

    }


    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_input_handler.drop(dragSourceDetails);
    }


    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        return m_input_handler.interested_in(dragSourceDetails);
    }


    bool supports_drag_to(const DndTarget::Details& source) const override {
        // note: definitely not safe!
        return source.sourceComponent.get() != this && dynamic_cast<DragAndDroppableComponent*>(source.sourceComponent.get()) ;
    }


    void set_text(const std::string& text) override {
        m_text = text;
        repaint();
    }


    void reset() override {
        m_text = m_default_text;
        repaint();
    }


private:
    std::string m_default_text;
    std::string m_text;

    InteractionVisualizer m_visualizer{*this, default_visualizations()};
    InputHandler m_input_handler;
};



// ==============================================================================================

class MoveMode : public InputMode {
public:
    enum class State {
        enabled = 0
        , hovering = 1
        , dragging = 2
    };

    class Visualization : public InteractionVisualization {
    public:
        Visualization() {
            addChildComponent(m_enabled);
            addChildComponent(m_hover);
            addChildComponent(m_ongoing_move);
        }


        void resized() override {
            m_enabled.setBounds(getLocalBounds());
            m_hover.setBounds(getLocalBounds());
            m_ongoing_move.setBounds(getLocalBounds());
        }


        void state_changed(InputMode* active_mode, int state, AlphaMask&) override {
            auto is_move = active_mode && active_mode->is<MoveMode>();
            m_enabled.setVisible(is_move && state == static_cast<int>(MoveMode::State::enabled));
            m_hover.setVisible(is_move && (state == static_cast<int>(MoveMode::State::hovering) ||
                                           state == static_cast<int>(MoveMode::State::dragging)));
            m_ongoing_move.setVisible(is_move && state == static_cast<int>(MoveMode::State::dragging));
        }


    private:
        BorderHighlight m_enabled{juce::Colours::yellowgreen.withAlpha(0.6f), 1};
        BorderHighlight m_hover{juce::Colours::yellowgreen, 2};
        FillHighlight m_ongoing_move{juce::Colours::palegoldenrod.withAlpha(0.1f)};

    };


    explicit MoveMode(juce::Component& managed_component) : m_managed_component(managed_component) {}


    DragBehaviour get_drag_behaviour() override {
        return {DragBehaviour::DefaultDrag{}};
    }


    bool supports_drag_to(const DndTarget::Details&) const override {
        return false;
    }


    std::optional<int> mouse_state_changed(const MouseState& mouse_state
                                           , const DragAndDropState&) override {

        if (!mouse_state.is_over_component()) {
            return static_cast<int>(State::enabled);

        } else if (mouse_state.is_down && !m_mouse_down_position) {
            assert(mouse_state.position);
            m_mouse_down_position = mouse_state.position;
            return static_cast<int>(State::dragging);

        } else if (!mouse_state.is_down && m_mouse_down_position) {
            m_mouse_down_position = std::nullopt;
            return static_cast<int>(State::hovering);

        } else if (!mouse_state.is_down && mouse_state.is_directly_over_component()) {
            return static_cast<int>(State::hovering);

        } else {
            return std::nullopt;
        }
    }


    std::optional<int> mouse_position_changed(const MouseState& mouse_state
                                              , const DragAndDropState&) override {
        if (m_mouse_down_position) {
            assert(mouse_state.position);
            auto bounds = m_managed_component.getBounds();
            bounds += mouse_state.position.value() - m_mouse_down_position.value();
            m_managed_component.setBounds(bounds);
        }
        return std::nullopt;

    }


    std::optional<int> input_event_registered(std::unique_ptr<InputEvent>) override {
        std::cout << "some input event??";
        return std::nullopt;
    }


    void reset() override {
        m_mouse_down_position = std::nullopt;
    }


private:
    juce::Component& m_managed_component;
    std::optional<juce::Point<int>> m_mouse_down_position = std::nullopt;
};


// ==============================================================================================

class MovableComponent : public juce::Component {
public:
    MovableComponent(std::string text
                     , Vec<std::unique_ptr<InputCondition>> move_conditions)
            : m_text(std::move(text))
              , m_input_handler(nullptr, *this
                                , default_modes(*this, std::move(move_conditions)), {std::ref(m_visualizer)}) {
        addAndMakeVisible(m_visualizer);
    }


    static InputModeMap default_modes(juce::Component& managed_component
                                      , Vec<std::unique_ptr<InputCondition>> conditions) {
        InputModeMap map;
        map.add(std::move(conditions), std::make_unique<MoveMode>(managed_component));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{std::make_unique<MoveMode::Visualization>()};
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::orchid);
        g.setColour(juce::Colours::whitesmoke);
        g.drawFittedText(m_text, getLocalBounds(), juce::Justification::centred, 1);
    }


    void resized() override {
        m_visualizer.setBounds(getLocalBounds());
    }


private:
    std::string m_text;

    InteractionVisualizer m_visualizer{*this, default_visualizations()};
    InputHandler m_input_handler;//{nullptr, *this, modes(*this), {std::ref(m_visualizer)}};
};


// ==============================================================================================

class StatePlaygroundComponent : public juce::Component, public juce::DragAndDropContainer {
public:
    StatePlaygroundComponent() {
        addAndMakeVisible(m_always_movable_component);
        addAndMakeVisible(m_conditioned_movable_component);
        addAndMakeVisible(m_dnd_component1);
        addAndMakeVisible(m_dnd_component2);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumpurple);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(50);
        auto col1 = bounds.removeFromLeft(100);
        m_always_movable_component.setBounds(col1.removeFromTop(100));
        col1.removeFromTop(50);
        m_conditioned_movable_component.setBounds(col1.removeFromTop(100));
        col1.removeFromTop(50);
        m_dnd_component1.setBounds(col1.removeFromTop(100));
        col1.removeFromTop(50);
        m_dnd_component2.setBounds(col1.removeFromTop(100));
    }


private:
    MovableComponent m_always_movable_component{
            "Always Movable"
            , Vec<std::unique_ptr<InputCondition>>{std::make_unique<AlwaysTrueCondition>()}
    };

    MovableComponent m_conditioned_movable_component{
            "Movable (Q)"
            , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
    };

    DragAndDroppableComponent m_dnd_component1{
            "Drag & Drop (W)"
            , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
    };

    DragAndDroppableComponent m_dnd_component2{
            "Drag & Drop (W)"
            , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
    };
};


class MainComponent : public MainKeyboardFocusComponent
                      , private juce::HighResolutionTimer
                      , private juce::ValueTree::Listener {
public:

    unsigned int ROOT_ID = 1;


    MainComponent()
            : m_some_handler(m_undo_manager) {
        addAndMakeVisible(m_playground);

        setSize(600, 600);
    }

//    static Vec<TriggerableState> states() {
//        std::unique_ptr<Condition> cond = std::make_unique<KeyCondition>('q');
//        TriggerableState{std::make_unique<KeyCondition>('q')
//                      , State{ModuleIds::ConfigurationLayerComponent, StateIds::Configuration::Move}}
//    }


    void resized() override {
        m_playground.setBounds(getLocalBounds());
    }


    void hiResTimerCallback() override {
        callback_count += 1;
        if (callback_count % 500 == 0) {
        }
    }


    void globalFocusChanged(juce::Component* focusedComponent) override {
        if (focusedComponent) {
//            std::cout << "focused component dims " << focusedComponent->getWidth() << " "
//                      << focusedComponent->getHeight() << "\n";
        } else {
//            std::cout << "nullptr\n";
        }
        MainKeyboardFocusComponent::globalFocusChanged(focusedComponent);
    }


    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {
//        std::cout << m_some_handler.get_value_tree().toXmlString() << "\n";
    }


private:
    std::unique_ptr<juce::LookAndFeel> m_lnf;
    juce::UndoManager m_undo_manager;
    ParameterHandler m_some_handler;

    StatePlaygroundComponent m_playground;

    long callback_count = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


// ==============================================================================================

class StatePlayground : public juce::JUCEApplication {
public:

    StatePlayground() = default;


    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }


    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }


    bool moreThanOneInstanceAllowed() override { return true; }


    void initialise(const juce::String& commandLine) override {
        juce::ignoreUnused(commandLine);

        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }


    void shutdown() override {
        mainWindow = nullptr;
    }


    void systemRequestedQuit() override {
        quit();
    }


    void anotherInstanceStarted(const juce::String& commandLine) override {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused(commandLine);
    }


    class MainWindow : public juce::DocumentWindow {
    public:
        explicit MainWindow(const juce::String& name)
                : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }


        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }


    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;

};

START_JUCE_APPLICATION (StatePlayground)

