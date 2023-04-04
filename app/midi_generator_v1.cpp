#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include "../src/scheduler.h"
#include "../src/renderers.h"
#include "../src/transport.h"
#include "../src/generation_graph.h"
#include "../src/looper.h"
#include "../src/gui/looper_component.h"
#include "../src/gui/mapping_component.h"

class MidiGeneratorV1Component : public juce::Component
                                 , private juce::HighResolutionTimer {
public:
    MidiGeneratorV1Component() {
        auto onset = std::make_shared<Looper<double>>(Mapping<double>{
                {  1.0}
                , {1.0}
                , {1.0}
                , {2.0}
                , {1.0}}, 1.0, 0.0, Phasor::Mode::stepped);


        auto duration = std::make_shared<Looper<double>>(Mapping<double>{{1.0}}
                                                         , 1.0
                                                         , 0.0
                                                         , Phasor::Mode::stepped);
        auto pitch = std::make_shared<Looper<int>>(Mapping<int>{{  6000}
                                                                , {6200}
                                                                , {6400}
                                                                , {6700}}, 1.0, 0.0, Phasor::Mode::stepped);
        auto velocity = std::make_shared<Looper<int>>(Mapping<int>{{100}}
                                                      , 1.0
                                                      , 0.0
                                                      , Phasor::Mode::stepped);
        auto channel = std::make_shared<Looper<int>>(Mapping<int>{{1}}
                                                     , 1.0
                                                     , 0.0
                                                     , Phasor::Mode::stepped);

        m_onset = std::make_unique<LooperComponent<double>>(onset, "onset");
        m_duration = std::make_unique<LooperComponent<double>>(duration, "duration");
        m_pitch = std::make_unique<LooperComponent<int>>(pitch, "pitch");
        m_velocity = std::make_unique<LooperComponent<int>>(velocity, "velocity");
        m_channel = std::make_unique<LooperComponent<int>>(channel, "channel");

        addAndMakeVisible(*m_onset);
        addAndMakeVisible(*m_duration);
        addAndMakeVisible(*m_pitch);
        addAndMakeVisible(*m_velocity);
        addAndMakeVisible(*m_channel);


        m_graph = std::make_unique<SimplisticMidiGraphV1>(onset, duration, pitch, velocity, channel);

        m_scheduler.add_event(std::make_unique<TriggerEvent>(m_transport.update_time()));

        m_transport.start();

        bool is_initialized = m_midi_renderer.initialize(std::string("IAC Driver IAC1"));

        assert(is_initialized);

        startTimer(1);
//        scheduler.add_event()
        setSize(600, 400);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello MIDI!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
        auto bounds = getLocalBounds();
        m_onset->setBounds(bounds.removeFromTop(60));
        m_duration->setBounds(bounds.removeFromTop(60));
        m_pitch->setBounds(bounds.removeFromTop(60));
        m_velocity->setBounds(bounds.removeFromTop(60));
        m_channel->setBounds(bounds.removeFromTop(60));
    }


    void hiResTimerCallback() override {
//    void timerCallback() override {
//        std::cout << "(start)\n";
        const auto& time = m_transport.update_time();
//        std::cout << "timer callback (" << time.get_tick() << ")\n";
        auto events = m_scheduler.get_events(time);
//        std::cout << "n events: " << events.size() << "(remaining: " << m_scheduler.size() << ")\n";

//        std::cout << "timer callback ()\n";
        for (auto& event: events) {
            if (dynamic_cast<TriggerEvent*>(event.get())) {
                std::cout << "#################### Trigger: " << event->get_time() << "\n";
                m_scheduler.add_events(m_graph->process(time));
            } else if (auto note_event = dynamic_cast<MidiEvent*>(event.get())) {
                std::cout << "note:    @" << note_event->get_time() << " (" << note_event->get_note_number() << ", "
                          << note_event->get_velocity() << ", " << note_event->get_channel() << ")\n";
                m_midi_renderer.render(note_event);
            }
        }
//        std::cout << "(end)\n";
    }


private:
    Transport m_transport;
    Scheduler m_scheduler;
    MidiRenderer m_midi_renderer;
    std::unique_ptr<SimplisticMidiGraphV1> m_graph;

    std::unique_ptr<LooperComponent<double>> m_onset;
    std::unique_ptr<LooperComponent<double>> m_duration;
    std::unique_ptr<LooperComponent<int>> m_pitch;
    std::unique_ptr<LooperComponent<int>> m_velocity;
    std::unique_ptr<LooperComponent<int>> m_channel;

    std::unique_ptr<TempMappingComponent<double>> m_onset_values;
    std::unique_ptr<TempMappingComponent<double>> m_duration_values;
    std::unique_ptr<TempMappingComponent<int>> m_pitch_values;
    std::unique_ptr<TempMappingComponent<int>> m_velocity_values;
    std::unique_ptr<TempMappingComponent<int>> m_channel_values;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiGeneratorV1Component)
};


// ==============================================================================================

class MidiGeneratorV1 : public juce::JUCEApplication {
public:
    MidiGeneratorV1() = default;

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
            setContentOwned(new MidiGeneratorV1Component(), true);

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

START_JUCE_APPLICATION (MidiGeneratorV1)
