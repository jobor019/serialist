

#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include "look_and_feel.h"
#include "core/param/parameter_policy.h"
#include "key_state.h"
#include "gui/state/state_handler.h"
#include "state/interaction_visualizer.h"
#include "bases/module_stereotypes.h"

class MouseOverVisualization : public SingleVisualizationBase {
public:
    MouseOverVisualization()
            : SingleVisualizationBase(std::make_unique<BorderHighlight>(juce::Colours::floralwhite.withAlpha(0.4f))) {}


    bool is_active(const State& active_state, const MouseState& mouse_state) const override {
//        std::cout << "Mouseover stuff\n";
        return active_state == States::Default && mouse_state.is_over_component();
    }
};


// ==============================================================================================

class MouseDirectlyOverVisualization : public SingleVisualizationBase {
public:
    MouseDirectlyOverVisualization()
            : SingleVisualizationBase(std::make_unique<FillHighlight>(juce::Colours::darkturquoise.withAlpha(0.4f))) {}


    bool is_active(const State& active_state, const MouseState& mouse_state) const override {
        return active_state == States::Default && mouse_state.is_directly_over_component();
    }
};


// ==============================================================================================

class SomeGenerativeModule : public ModuleBase<Node<Facet>> {
public:

};


// ==============================================================================================

class SomeChildComponent : public juce::Component, public Stateful {
public:
    unsigned int CHILD_ID = 3;


    explicit SomeChildComponent(StateHandler& parent_state_handler)
            : m_visualizer(Vec<std::unique_ptr<InteractionVisualization>>(
            std::make_unique<MouseOverVisualization>()
            , std::make_unique<MouseDirectlyOverVisualization>()
    ))
              , m_state_handler(&parent_state_handler, *this, {*this, m_visualizer}
                                , Vec<TriggerableState>(
                            TriggerableState{
                                    std::make_unique<NoKeyCondition>()
                                    , States::Default}
                    )) {
        m_button.setButtonText("A button");
        addAndMakeVisible(m_button);
        addAndMakeVisible(m_visualizer);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::black);
    }


    void resized() override {
//        auto bounds = getLocalBounds().reduced(5);
        m_button.setBounds(getLocalBounds().removeFromTop(20));
        m_visualizer.setBounds(getLocalBounds());
    }


    void state_changed(const State& active_state, const MouseState& mouse_state) override {
        std::cout << "child: " << active_state.get_state_id()
                  << " [over: " << mouse_state.is_over_component()
                  << ", directly_over: " << mouse_state.is_directly_over_component()
                  << ", down: " << mouse_state.is_down
                  << ", dragging: " << mouse_state.is_dragging
                  << "] (update: " << m_count << ")\n";
        ++m_count;

    }


private:
    InteractionVisualizer m_visualizer;
    StateHandler m_state_handler;

    juce::TextButton m_button;
    std::size_t m_count = 0;

};


// ==============================================================================================

class SomeParentComponent : public juce::Component, public Stateful {
public:
    unsigned int PARENT_ID = 2;


    explicit SomeParentComponent(StateHandler& parent_state_handler)
            : m_visualizer(Vec<std::unique_ptr<InteractionVisualization>>(
            std::make_unique<MoveVisualization>()
            , std::make_unique<MouseOverVisualization>()
            , std::make_unique<MouseDirectlyOverVisualization>()
    ))
              , m_state_handler(&parent_state_handler, *this
                                , {*this, m_visualizer}
                                , Vec<TriggerableState>(
                            TriggerableState{
                                    std::make_unique<KeyCondition>(static_cast<int>('Q'))
                                    , States::Move}
                            , TriggerableState{
                                    std::make_unique<NoKeyCondition>()
                                    , States::Default}
                    ))

              , m_child_component(m_state_handler) {
        m_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        addAndMakeVisible(m_slider);
        addAndMakeVisible(m_child_component);
        addAndMakeVisible(m_visualizer);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::deeppink);
    }


    void resized() override {
        auto bounds = getLocalBounds();
        m_slider.setBounds(bounds.removeFromTop(20));
        bounds.removeFromBottom(20);
        m_child_component.setBounds(bounds);

        m_visualizer.setBounds(getLocalBounds());
    }


    void state_changed(const State& active_state, const MouseState& mouse_state) override {
        std::cout << "parent: " << active_state.get_state_id()
                  << " [over: " << mouse_state.is_over_component()
                  << ", directly_over: " << mouse_state.is_directly_over_component()
                  << ", down: " << mouse_state.is_down
                  << ", dragging: " << mouse_state.is_dragging
                  << "] (update: " << m_count << ")\n";
        ++m_count;
    }


private:
    InteractionVisualizer m_visualizer;
    StateHandler m_state_handler;

    SomeChildComponent m_child_component;


    juce::Slider m_slider;

    std::size_t m_count = 0;

};


// ==============================================================================================

class StatePlaygroundComponent : public MainKeyboardFocusComponent
                                 , private juce::HighResolutionTimer
                                 , private juce::ValueTree::Listener
                                 , public Stateful {
public:

    unsigned int ROOT_ID = 1;


    StatePlaygroundComponent()
            : m_visualizer(Vec<std::unique_ptr<InteractionVisualization>>(
            std::make_unique<MouseOverVisualization>()
            , std::make_unique<MouseDirectlyOverVisualization>()
    ))
              , m_some_handler(m_undo_manager)
              , m_state_handler(nullptr, *this, {*this, m_visualizer})
              , m_component1(m_state_handler) {

//        auto vvv = Vec<TriggerableState>(TriggerableState{std::make_unique<AlwaysTrueCondition>(), State{0, 0}});
//        auto y = Vec<TriggerableState>(TriggerableState{std::make_unique<AlwaysTrueCondition>(), State{1, 1}});
//        StateHandler(nullptr, *this, *this, Vec<TriggerableState>(TriggerableState{std::make_unique<AlwaysTrueCondition>(), State{1, 1}}));
//        auto x = Vec<TriggerableState>(TriggerableState{std::make_unique<AlwaysTrueCondition>(), State{1, 1}});
//        StateHandler(nullptr, *this, *this, Vec<TriggerableState>({std::make_unique<AlwaysTrueCondition>(), State{1, 1}}));
        addAndMakeVisible(m_visualizer);
//        auto vv = Vec<std::reference_wrapper<Stateful>>{m_component1};
        addAndMakeVisible(m_component1);
//        startTimer(1);
        setSize(600, 600);
    }

//    static Vec<TriggerableState> states() {
//        std::unique_ptr<Condition> cond = std::make_unique<KeyCondition>('q');
//        TriggerableState{std::make_unique<KeyCondition>('q')
//                      , State{ModuleIds::ConfigurationLayerComponent, StateIds::Configuration::Move}}
//    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumpurple);

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello OSC!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
        m_component1.setBounds(100, 100, 100, 100);
        m_visualizer.setBounds(getLocalBounds());
    }


    void state_changed(const State& active_state, const MouseState& mouse_state) override {
        std::cout << "root: " << active_state.get_state_id()
                  << " [over: " << mouse_state.is_over_component()
                  << ", directly_over: " << mouse_state.is_directly_over_component()
                  << ", down: " << mouse_state.is_down
                  << ", dragging: " << mouse_state.is_dragging
                  << "] (update: " << m_state_count << ")\n";
        ++m_state_count;
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

    long callback_count = 0;

    InteractionVisualizer m_visualizer;
    StateHandler m_state_handler;

    SomeParentComponent m_component1;

    std::size_t m_state_count = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatePlaygroundComponent)
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
            setContentOwned(new StatePlaygroundComponent(), true);

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

