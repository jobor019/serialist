

#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include "look_and_feel.h"
#include "core/param/parameter_policy.h"
#include "key_state.h"
#include "gui/state/state_handler.h"

class SomeChildComponent : public juce::Component {
public:
    SomeChildComponent() {
        m_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        addAndMakeVisible(m_slider);
    }

    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::black);
    }
    void resized() override {
        auto bounds = getLocalBounds().reduced(5);
        m_slider.setBounds(bounds.removeFromTop(20));
    }

private:
    juce::Slider m_slider;

};

class SomeParentComponent : public juce::Component, public Stateful {
public:
    explicit SomeParentComponent(StateHandler& state_handler)
    : m_state_handler(&state_handler, *this, *this) {
        m_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        m_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        addAndMakeVisible(m_slider);
    }

    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::black);
    }
    void resized() override {
        auto bounds = getLocalBounds().reduced(5);
        m_slider.setBounds(bounds.removeFromTop(20));
    }

    void update_state(const State &active_state) override {
        std::cout << "parent state hehe\n";
    }

private:
    StateHandler m_state_handler;

    juce::Slider m_slider;

};


// ==============================================================================================

class StatePlaygroundComponent : public MainKeyboardFocusComponent
                               , private juce::HighResolutionTimer
                               , private juce::ValueTree::Listener
                               , public Stateful {
public:

    StatePlaygroundComponent()
            : m_some_handler(m_undo_manager)
            , m_state_handler(nullptr, *this, *this)
            , m_component1(m_state_handler)
    {
        addAndMakeVisible(m_component1);
//        startTimer(1);
        setSize(600, 600);


    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumaquamarine);

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello OSC!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
        m_component1.setBounds(100, 100, 100, 100);
    }

    void update_state(const State &active_state) override {
        std::cout << "root state hehe\n";
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

    StateHandler m_state_handler;

    SomeParentComponent m_component1;


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

