
#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include "look_and_feel.h"
#include "core/param/parameter_policy.h"
#include "key_state.h"
#include "core/generatives/osc_sender.h"
#include "core/generatives/unit_pulse.h"
#include "core/generatives/sequence.h"
#include "core/generatives/variable.h"
#include "multi_slider/score_slider.h"
#include "multi_slider/multi_slider.h"
#include "multi_slider/bar_slider.h"

class OscPlaygroundComponent : public MainKeyboardFocusComponent
                               , private juce::HighResolutionTimer
                               , private juce::ValueTree::Listener {
public:

    OscPlaygroundComponent()
            : m_some_handler(m_undo_manager)
              , m_pulse("pulse", m_some_handler)
              , m_send_data("sequence", m_some_handler, Voices<int>::transposed({1234, 1234, 4556}))
              , m_address("address", m_some_handler, "/test")
              , m_sender("sender", m_some_handler, &m_address, &m_send_data, &m_pulse)
              , m_multi_slider_sequence("multi", m_some_handler, 99)
              , m_multi_slider(m_multi_slider_sequence, std::make_unique<BarSliderConfig<int>>()) {

        addAndMakeVisible(m_multi_slider);

        m_sender.set_target({"127.0.0.1", 8080});

        m_lnf = std::make_unique<SerialistLookAndFeel>();
        SerialistLookAndFeel::setup_look_and_feel_colors(*m_lnf);
        juce::Desktop::getInstance().setDefaultLookAndFeel(m_lnf.get());

        Sequence<Facet, int> seq("sequence", m_some_handler);
        Vec<Vec<int>> values = Vec<Vec<int>>::allocated(3);
        values.append({1, 2, 3, 4});
        values.append({99, 98, 97});
        values.append({0, -1, 0, 1});
        seq.set_values(Voices<int>(values));

        auto property = seq.get_parameter_handler().get_value_tree().getChild(0).getChild(0).getProperty("v2");

//        std::cout << "is string" << property.isString() << "\n";
//        std::cout << "is int" << property.isInt() << "\n";
//        std::cout << "is bool" << property.isBool() << "\n";
//        std::cout << "is double" << property.isDouble() << "\n";

//        std::cout << seq.get_parameter_handler().get_value_tree().getChild(0).getChild(0).getProperty("v2").toString() << "\n";

//        seq.set_values(Voices<int>(values));
//        std::cout << "ValueTree:\n";
//        std::cout << m_some_handler.get_value_tree().toXmlString() << "\n";
//ö
//        startTimer(1);
        setSize(1000, 400);


//        auto [pulsator_module, pulsator_generatives] = ModuleFactory::new_pulsator(m_generation_graph, PulsatorModule::Layout::note_source_internal);
//        auto& generative = pulsator_module->get_generative();
//        if (generative) {
//            auto* pulsator = dynamic_cast<Node<TriggerEvent>*>(&generative);
//            std::cout << "dc worked\n";
//        } else {
//            std::cout << "generative not set\n";
//        }

    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumaquamarine);

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello OSC!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
        m_multi_slider.setBounds(50, 50, 300, 100);
    }


    void hiResTimerCallback() override {
        callback_count += 1;
        if (callback_count % 500 == 0) {
            std::cout << "sending..?\n";
            m_sender.update_time(TimePoint());
            m_sender.process();
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


    UnitPulse m_pulse;
    Sequence<Facet, int> m_send_data;
    Variable<std::string> m_address;
    OscSenderNode m_sender;

    Sequence<Facet, int> m_multi_slider_sequence;
    MultiSlider<int> m_multi_slider;
//    ScoreSlider m_score_slider;

    long callback_count = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscPlaygroundComponent)
};


// ==============================================================================================

class OscPlayground : public juce::JUCEApplication {
public:

    OscPlayground() = default;


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
            setContentOwned(new OscPlaygroundComponent(), true);

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

START_JUCE_APPLICATION (OscPlayground)

