
#include <juce_gui_extra/juce_gui_extra.h>
#include "interaction/drag_and_drop/drag_and_drop.h"
#include "key_state.h"
#include "core/param/parameter_policy.h"
#include "slider_widget.h"


class MultiSliderPlaygroundComponent : public juce::Component {
public:
    MultiSliderPlaygroundComponent()
            : m_dnd_container(*this)
            , m_slider(nullptr, 0.0, Slider::Bounds(0, 100))
    {
        addAndMakeVisible(m_dnd_container);
        addAndMakeVisible(m_slider);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumpurple);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(50);

        auto col = bounds.removeFromLeft(100);
        m_slider.setBounds(col.removeFromTop(40));


        m_dnd_container.setBounds(getLocalBounds());
    }



private:
    GlobalDragAndDropContainer m_dnd_container;

    Slider m_slider;
};


// ==============================================================================================

class MainComponent : public MainKeyboardFocusComponent {
public:

    unsigned int ROOT_ID = 1;


    MainComponent()
            : m_parameter_handler(m_undo_manager) {

        addAndMakeVisible(m_playground);

        setSize(900, 600);
    }


    void resized() override {
        m_playground.setBounds(getLocalBounds());
    }



    void globalFocusChanged(juce::Component* focusedComponent) override {
        MainKeyboardFocusComponent::globalFocusChanged(focusedComponent);
    }


private:
    std::unique_ptr<juce::LookAndFeel> m_lnf;
    ParameterHandler m_parameter_handler;
    juce::UndoManager m_undo_manager;

    MultiSliderPlaygroundComponent m_playground;


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
