

#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include "core/param/parameter_policy.h"
#include "key_state.h"
#include "gui/interaction/input_handler.h"
#include "interaction/interaction_visualizer.h"

class DummyTextInterface {
public:
    virtual void set_text(const std::string& text) = 0;
    virtual void reset() = 0;


    virtual std::string get_identifier() const { return ""; }
};

class DummyDragInfo : public DragInfo {
public:
    explicit DummyDragInfo(const DummyTextInterface& ti) : text_interface(ti) {}


    const DummyTextInterface& text_interface;
};


// ==============================================================================================

class DefaultMode : public InputMode {
public:
    DefaultMode(DummyTextInterface& text_component, std::string text)
            : m_text_component(text_component)
              , m_text(std::move(text)) {}


    std::optional<int> mode_activated() override {
        std::cout << "DEFAULT MODE ACTIVATED (text: " << m_text << ")" << std::endl;
        m_text_component.set_text(m_text);
        return 0;
    }


    std::optional<int> mouse_state_changed(const MouseState&) override {
        return std::nullopt;
    }


    std::optional<int> mouse_position_changed(const MouseState&) override {
        return std::nullopt;
    }


    std::optional<int> input_event_registered(std::unique_ptr<InputEvent>) override {
        return std::nullopt;
    }


    void reset() override {
    }


private:
    DummyTextInterface& m_text_component;
    std::string m_text;
};


// ==============================================================================================

class DragAndDropMode : public InputMode {
public:
    enum class State {
        enabled = 1
        , hovering = 2
        , dragging_from = 3
        , dragging_to = 4
        , valid_target = 5
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
            m_valid_target.setBounds(getLocalBounds().reduced(2));
        }


        void state_changed(InputMode* active_mode, int state, AlphaMask& alpha) override {
            auto is_active = active_mode && active_mode->is<DragAndDropMode>();
            m_enabled.setVisible(is_active && state == static_cast<int>(State::enabled));
            m_hover.setVisible(is_active && state == static_cast<int>(State::hovering));
            m_drag_to.setVisible(is_active && state == static_cast<int>(State::dragging_to));
            m_valid_target.setVisible(is_active && (state == static_cast<int>(State::valid_target)
                                                    || state == static_cast<int>(State::dragging_to)));

            if (is_active && state == static_cast<int>(State::dragging_from)) {
                alpha.set_alpha(0.4f);
            } else {
                alpha.set_alpha(1.0f);
            }
        }


    private:
        BorderHighlight m_enabled{juce::Colours::darkgreen.withAlpha(0.6f), 2};
        BorderHighlight m_hover{juce::Colours::darkgreen, 4};
        FillHighlight m_drag_to{juce::Colours::darkturquoise.withAlpha(0.3f)};
        BorderHighlight m_valid_target{juce::Colours::palegoldenrod.withAlpha(0.8f)};
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    explicit DragAndDropMode(DummyTextInterface& dnd_component) : m_dnd_component(dnd_component) {}


    DragBehaviour get_drag_behaviour() override {
        return DragBehaviour::drag_and_drop;
    }


    std::unique_ptr<DragInfo> get_drag_info() override {
        auto info = std::make_unique<DummyDragInfo>(m_dnd_component);
//        m_source = info.get();
        return std::move(info);
    }


//    std::optional<juce::ScaledImage> get_drag_image() const override {
//        return juce::ScaledImage(m_dnd_component.createComponentSnapshot(m_dnd_component.getLocalBounds()));
//    }

    std::optional<int> item_dropped(const DragInfo&) override {
        std::cout << "!!!!!! ITEM DROPPED !!!!!!!!!" << std::endl;
        m_dnd_component.set_text("Drag enabled");
        return static_cast<int>(State::enabled);
    }


    bool supports_drop_from(const DragInfo& source) const override {
        return typeid(source) == typeid(DummyDragInfo)
               && &dynamic_cast<const DummyDragInfo*>(&source)->text_interface != &m_dnd_component;
    }


    std::optional<int> mouse_state_changed(const MouseState& mouse_state) override {
        if (mouse_state.is_dragging_from) {
            m_dnd_component.set_text("Dragging (from)...");
            return static_cast<int>(State::dragging_from);

        } else if (mouse_state.is_dragging_to) {
            m_dnd_component.set_text("Drop here!");
            return static_cast<int>(State::dragging_to);

        } else if (mouse_state.has_external_ongoing_drag) {
            m_dnd_component.set_text("Can be dropped here!");
            return static_cast<int>(State::valid_target);

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


    std::optional<int> mouse_position_changed(const MouseState&) override {
        return std::nullopt;
    }


    std::optional<int> input_event_registered(std::unique_ptr<InputEvent> input_event) override {
        if (input_event->is<DragDropped>()) {
            std::cout << "Dragdropped\n";
        }
        return std::nullopt;
    }


    void reset() override {
        std::cout << m_dnd_component.get_identifier() << "::resetting!\n";
        m_dnd_component.reset();
    }


private:
    DummyTextInterface& m_dnd_component;

//    DummyDragInfo* m_source = nullptr;
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


    explicit MoveMode(juce::Component& managed_component
                      , DummyTextInterface& text_interface
                      , std::string text)
            : m_text(std::move(text))
              , m_managed_component(managed_component)
              , m_text_component(text_interface) {}


    std::optional<int> mode_activated() override {
        m_text_component.set_text(m_text);
        return std::nullopt;
    }


    std::optional<int> mouse_state_changed(const MouseState& mouse_state) override {

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


    std::optional<int> mouse_position_changed(const MouseState& mouse_state) override {
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
    std::string m_text;
    juce::Component& m_managed_component;
    DummyTextInterface& m_text_component;
    std::optional<juce::Point<int>> m_mouse_down_position = std::nullopt;
};


// ==============================================================================================

class DragAndDroppableComponent : public juce::Component, public DropArea, public DummyTextInterface {
public:
    DragAndDroppableComponent(const std::string& initial_text
                              , Vec<std::unique_ptr<InputCondition>> conditions
                              , GlobalDragAndDropContainer& dnd_container
                              , const std::string& identifier)
            : m_default_text(initial_text)
              , m_text(initial_text)
              , m_input_handler(nullptr, *this
                                , default_modes(*this, std::move(conditions))
                                , dnd_container
                                , {std::ref(m_visualizer)}
                                , identifier)
              , m_identifier(identifier) {
        addAndMakeVisible(m_visualizer);
    }


    static InputModeMap default_modes(DragAndDroppableComponent& managed_component
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
        std::cout << "~" << m_identifier << "::painting text: " << m_text << std::endl;
    }


    DropListener& get_drop_listener() override {
        return m_input_handler;
    }


//    bool supports_drag_to(const DndTarget::Details& source) const override {
//        // note: definitely not safe!
//        return source.sourceComponent.get() != this && dynamic_cast<DragAndDroppableComponent*>(source.sourceComponent.get()) ;
//    }


    void set_text(const std::string& text) override {
        m_text = text;
        std::cout << m_identifier << "::text set to " << m_text << std::endl;
        repaint();
    }


    void reset() override {
        m_text = m_default_text;
        std::cout << m_identifier << "::text set to " << m_default_text << " (reset)" << std::endl;
        repaint();
    }


    std::string get_identifier() const override {
        return m_identifier;
    }


private:
    std::string m_default_text;
    std::string m_text;

    InteractionVisualizer m_visualizer{*this, default_visualizations()};
    InputHandler m_input_handler;

    std::string m_identifier;
};



// ==============================================================================================




// ==============================================================================================

class MovableComponent : public juce::Component, public DummyTextInterface {
public:
    MovableComponent(std::string active_text
                     , std::string default_text
                     , Vec<std::unique_ptr<InputCondition>> move_conditions
                     , GlobalDragAndDropContainer& dnd_container)
            : m_input_handler(nullptr, *this
                              , default_modes(std::move(active_text)
                                              , std::move(default_text)
                                              , *this, std::move(move_conditions))
                              , dnd_container
                              , {std::ref(m_visualizer)}) {
        addAndMakeVisible(m_visualizer);
    }


    static InputModeMap default_modes(std::string active_text
                                      , std::string disabled_text
                                      , MovableComponent& c
                                      , Vec<std::unique_ptr<InputCondition>> conditions) {
        InputModeMap map;
        map.add(std::move(conditions), std::make_unique<MoveMode>(c, c, std::move(active_text)));
        map.add(std::make_unique<NoKeyCondition>(), std::make_unique<DefaultMode>(c, std::move(disabled_text)));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{std::make_unique<MoveMode::Visualization>()};
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::orchid);
        g.setColour(juce::Colours::whitesmoke);
        g.drawFittedText(m_active_text, getLocalBounds(), juce::Justification::centred, 1);
    }


    void resized() override {
        m_visualizer.setBounds(getLocalBounds());
    }


    void set_text(const std::string& text) override {
        m_active_text = text;
        repaint();
    }


    void reset() override {}


private:
    std::string m_active_text;
    std::string m_default_text;

    InteractionVisualizer m_visualizer{*this, default_visualizations()};
    InputHandler m_input_handler;//{nullptr, *this, modes(*this), {std::ref(m_visualizer)}};
};


// ==============================================================================================

class StatePlaygroundComponent : public juce::Component {
public:
    StatePlaygroundComponent()
            : m_dnd_container(*this)
              , m_always_movable_component(
                    "Always Movable"
                    , "Always Movable"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<AlwaysTrueCondition>()}
                    , m_dnd_container
            )
              , m_conditioned_movable_component(
                    "Drag to move!"
                    , "Movable (Q)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , m_dnd_container)
              , m_dnd_component1(
                    "Drag & Drop (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "001"
            )
              , m_dnd_component2(
                    "Drag & Drop (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "002"
            ) {
        addAndMakeVisible(m_always_movable_component);
        addAndMakeVisible(m_conditioned_movable_component);
        addAndMakeVisible(m_dnd_component1);
        addAndMakeVisible(m_dnd_component2);
        addAndMakeVisible(m_dnd_container);
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

        m_dnd_container.setBounds(getLocalBounds());
    }


//    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails &) override {
//        std::cout << "drag operation endede\n";
//    }


private:
    GlobalDragAndDropContainer m_dnd_container;

    MovableComponent m_always_movable_component;

    MovableComponent m_conditioned_movable_component;

    // TODO: This should have an InputHandler and pass it to its children
    //  (verify that no-state input handler works as intended)

    DragAndDroppableComponent m_dnd_component1;
    DragAndDroppableComponent m_dnd_component2;
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

