
#include <juce_gui_extra/juce_gui_extra.h>
#include "interaction/drag_and_drop/drag_and_drop.h"
#include "key_state.h"
#include "core/param/parameter_policy.h"
#include "slider_widget.h"


class MultiSliderPlaygroundComponent : public juce::Component {
public:
    MultiSliderPlaygroundComponent()
            : m_dnd_container(*this)
              , m_slider_unit_range(nullptr)
              , m_slider_midi_range(nullptr, SliderValue(0.0, DiscreteRange<double>::from_size(0, 128, 128), true))
              , m_slider_initial_value(nullptr, SliderValue(0.7))
              , m_slider_discrete(nullptr, SliderValue(0.0, DiscreteRange<double>::from_size(0, 10, 10), true))
              , m_slider_binary(nullptr, SliderValue(0.0, DiscreteRange<double>::from_size(0, 2, 2), true))
              , m_slider_infinitesimal(nullptr, SliderValue(0.0, DiscreteRange<double>::from_size(0, 1e-4, 200)))
              , m_slider_exponential1(nullptr
                                      , SliderValue(0.0, DiscreteRange<double>::from_size(0, 1000, 201, true)
                                                    , false, std::nullopt, std::nullopt
                                                    , Exponential<double>(2)))
              , m_slider_exponential2(nullptr
                                      , SliderValue(0.0, DiscreteRange<double>::from_size(0, 1000, 201, true)
                                                    , false, std::nullopt, std::nullopt
                                                    , Exponential<double>(8))) {
        addAndMakeVisible(m_dnd_container);
        addAndMakeVisible(m_slider_unit_range);
        addAndMakeVisible(m_slider_midi_range);
        addAndMakeVisible(m_slider_initial_value);
        addAndMakeVisible(m_slider_discrete);
        addAndMakeVisible(m_slider_binary);
        addAndMakeVisible(m_slider_infinitesimal);
        addAndMakeVisible(m_slider_exponential1);
        addAndMakeVisible(m_slider_exponential2);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumpurple);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(50);

        auto col = bounds.removeFromLeft(100);
        m_slider_unit_range.setBounds(col.removeFromTop(30));

        col.removeFromTop(40);
        m_slider_midi_range.setBounds(col.removeFromTop(30));

        col.removeFromTop(40);
        m_slider_initial_value.setBounds(col.removeFromTop(30));

        col.removeFromTop(40);
        m_slider_discrete.setBounds(col.removeFromTop(30));

        col.removeFromTop(40);
        m_slider_binary.setBounds(col.removeFromTop(30));

        col.removeFromTop(40);
        m_slider_infinitesimal.setBounds(col.removeFromTop(30));

        col.removeFromTop(40);
        m_slider_exponential1.setBounds(col.removeFromTop(30));

        bounds.removeFromLeft(60);
        col = bounds.removeFromLeft(100);

        m_slider_exponential2.setBounds(col.removeFromTop(30));


        m_dnd_container.setBounds(getLocalBounds());
    }


private:
    GlobalDragAndDropContainer m_dnd_container;

    Slider m_slider_unit_range;
    Slider m_slider_midi_range;
    Slider m_slider_initial_value;
    Slider m_slider_discrete;
    Slider m_slider_binary;
    Slider m_slider_infinitesimal;
    Slider m_slider_exponential1;
    Slider m_slider_exponential2;
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
