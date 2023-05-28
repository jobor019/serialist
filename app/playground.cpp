#include <juce_gui_extra/juce_gui_extra.h>
#include <scalable_slider.h>

#include <memory>
#include "oscillator_component.h"
#include "text_sequence_component.h"
#include "interpolation_component.h"
#include "generator_component.h"
#include "look_and_feel.h"
#include "header_component.h"
#include "combobox_component.h"


class PlaygroundComponent : public juce::Component
                            , private juce::Timer {
public:


    PlaygroundComponent()
            : m_value_tree(m_vt_identifier), m_some_handler(m_value_tree, m_undo_manager)
              , m_oscillator("osc1", m_some_handler)
              , m_sequence("seq1", m_some_handler)
              , m_interpolator("interp1", m_some_handler)
              , m_pitch("pitch", m_some_handler)
              , s(ScalableSlider::Orientation::vertical)
              , hc("header", m_some_handler)
              , my_cb("osctype"
                      , m_some_handler
                      , {{  "sin", Oscillator::Type::sin}
                         , {"sqr", Oscillator::Type::square}}
                      , Oscillator::Type::sin
                      , "helo") {

        m_lnf = std::make_unique<SerialistLookAndFeel>();
        SerialistLookAndFeel::setup_look_and_feel_colors(*m_lnf);
        juce::Desktop::getInstance().setDefaultLookAndFeel(m_lnf.get());

        addAndMakeVisible(m_oscillator);
        addAndMakeVisible(m_sequence);
        addAndMakeVisible(m_interpolator);
        addAndMakeVisible(m_pitch);
        addAndMakeVisible(s);

        tb.setButtonText("HH");
        addAndMakeVisible(tb);

        addAndMakeVisible(hc);

        addAndMakeVisible(my_cb);

        std::cout << m_value_tree.toXmlString() << std::endl;

        startTimer(50);
        setSize(600, 400);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumaquamarine);

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello Playground!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
        m_oscillator.setBounds(50, 30, 120, 170);
        m_sequence.setBounds(300, 30, 100, 40);
        m_interpolator.setBounds(50, 200, 180, 40);
        m_pitch.setBounds(300, 200, 180, 210);
        s.setBounds(300, 100, 12, 40);
        tb.setBounds(350, 100, 40, 40);
        hc.setBounds(400, 100, 200, 20);
        my_cb.setBounds(500, 40, 80, 50);
    }


    void timerCallback() override {
        auto ee = dynamic_cast<Oscillator*>(&m_oscillator.get_generative())->process(TimePoint());
        m_oscillator.repaint();
        ++callback_count;

        if (callback_count % 50 == 0) {
            std::cout << m_value_tree.toXmlString() << "\n";
        }

    }


private:

    const juce::Identifier m_vt_identifier = "root";

    std::unique_ptr<juce::LookAndFeel> m_lnf;

    juce::ValueTree m_value_tree;
    juce::UndoManager m_undo_manager;

    ParameterHandler m_some_handler;

    OscillatorComponent m_oscillator;
    TextSequenceComponent<int> m_sequence;
    InterpolationStrategyComponent<int> m_interpolator;
    GeneratorComponent<int> m_pitch;

    juce::ToggleButton tb;

    ScalableSlider s;

    HeaderComponent hc;

    ComboBoxComponent<Oscillator::Type> my_cb;

    int callback_count = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaygroundComponent)
};


// ==============================================================================================

class Playground : public juce::JUCEApplication {
public:

    Playground() = default;


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
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
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
            setContentOwned(new PlaygroundComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }


        void closeButtonPressed() override {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }


    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (Playground)

