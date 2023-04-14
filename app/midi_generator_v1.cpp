#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include "../src/scheduler.h"
#include "../src/renderers.h"
#include "../src/transport.h"
#include "../src/generation_graph.h"
#include "../src/looper.h"
#include "../src/generator.h"
#include "../src/gui/looper_component.h"
#include "../src/gui/mapping_component.h"
#include "../src/gui/generator_component.h"

class MidiGeneratorV1Component : public juce::Component
                                 , public juce::ComboBox::Listener
                                 , private juce::HighResolutionTimer {
public:
    static const int LOOPER_ID = 1;
    static const int GENERATOR_ID = 2;


    MidiGeneratorV1Component() {
        auto onset = std::make_shared<Looper<double>>(
                MultiMapping<double>{1.0, 1.0, 1.0, 2.0, 1.0}, 1.0, 0.0
                , Phasor::Mode::stepped);


        auto duration = std::make_shared<Looper<double>>(MultiMapping<double>{1.0}, 1.0, 0.0, Phasor::Mode::stepped);
        auto pitch = std::make_shared<Looper<int>>(MultiMapping<int>{6000, 6200, 6400, 6700}, 1.0, 0.0
                                                   , Phasor::Mode::stepped);
        auto velocity = std::make_shared<Looper<int>>(MultiMapping<int>{100}, 1.0, 0.0, Phasor::Mode::stepped);
        auto channel = std::make_shared<Looper<int>>(MultiMapping<int>{1}, 1.0, 0.0, Phasor::Mode::stepped);

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

        populate_combo_box(&m_onset_type);
        populate_combo_box(&m_duration_type);
        populate_combo_box(&m_pitch_type);
        populate_combo_box(&m_velocity_type);
        populate_combo_box(&m_channel_type);


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

        auto row = bounds.removeFromTop(60);
        m_onset_type.setBounds(row.removeFromLeft(90));
        m_onset->setBounds(row);

        row = bounds.removeFromTop(60);
        m_duration_type.setBounds(row.removeFromLeft(90));
        m_duration->setBounds(row);

        row = bounds.removeFromTop(60);
        m_pitch_type.setBounds(row.removeFromLeft(90));
        m_pitch->setBounds(row);

        row = bounds.removeFromTop(60);
        m_velocity_type.setBounds(row.removeFromLeft(90));
        m_velocity->setBounds(row);

        row = bounds.removeFromTop(60);
        m_channel_type.setBounds(row.removeFromLeft(90));
        m_channel->setBounds(row);
    }


    void hiResTimerCallback() override {
        const auto& time = m_transport.update_time();
        auto events = m_scheduler.get_events(time);

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
    }


    void comboBoxChanged(juce::ComboBox* combo_box) override {
        bool to_generator = combo_box->getSelectedId() == GENERATOR_ID;
        if (combo_box == &m_onset_type) {
            std::shared_ptr<GraphNode<double>> new_node;
            std::unique_ptr<Component> new_component;
            if (to_generator) {
                new_node = std::make_shared<Generator<double>>();
                m_onset = std::make_unique<GeneratorComponent<double>>(std::dynamic_pointer_cast<Generator<double>>(new_node), "onset");
            } else {
                new_node = std::make_shared<Looper<double>>();
                m_onset = std::make_unique<LooperComponent<double>>(std::dynamic_pointer_cast<Looper<double>>(new_node), "onset");
            }
            m_graph->set_onset(std::move(new_node));
            addAndMakeVisible(*m_onset);


        } else if (combo_box == &m_duration_type) {
            std::shared_ptr<GraphNode<double>> new_node;
            if (to_generator) {
                new_node = std::make_shared<Generator<double>>();
                m_duration = std::make_unique<GeneratorComponent<double>>(std::dynamic_pointer_cast<Generator<double>>(new_node), "duration");
            } else {
                new_node = std::make_shared<Looper<double>>();
                m_duration = std::make_unique<LooperComponent<double>>(std::dynamic_pointer_cast<Looper<double>>(new_node), "duration");
            }
            m_graph->set_duration(std::move(new_node));
            addAndMakeVisible(*m_duration);

        } else if (combo_box == &m_pitch_type) {
            std::shared_ptr<GraphNode<int>> new_node;
            if (to_generator) {
                new_node = std::make_shared<Generator<int>>();
                auto a = std::dynamic_pointer_cast<Generator<int>>(new_node);
                m_pitch = std::make_unique<GeneratorComponent<int>>(a, "pitch");
            } else {
                new_node = std::make_shared<Looper<int>>();
                auto e =std::dynamic_pointer_cast<Looper<int>>(new_node);
                m_pitch = std::make_unique<LooperComponent<int>>(std::dynamic_pointer_cast<Looper<int>>(new_node), "pitch");
            }
            m_graph->set_pitch(std::move(new_node));
            addAndMakeVisible(*m_pitch);

        } else if (combo_box == &m_velocity_type) {
            std::shared_ptr<GraphNode<int>> new_node;
            if (to_generator) {
                new_node = std::make_shared<Generator<int>>();
                m_velocity = std::make_unique<GeneratorComponent<int>>(std::dynamic_pointer_cast<Generator<int>>(new_node), "velocity");
            } else {
                new_node = std::make_shared<Looper<int>>();
                m_velocity = std::make_unique<LooperComponent<int>>(std::dynamic_pointer_cast<Looper<int>>(new_node), "velocity");
            }
            m_graph->set_velocity(std::move(new_node));
            addAndMakeVisible(*m_velocity);

        } else if (combo_box == &m_channel_type) {
            std::shared_ptr<GraphNode<int>> new_node;
            if (to_generator) {
                new_node = std::make_shared<Generator<int>>();
                m_channel = std::make_unique<GeneratorComponent<int>>(std::dynamic_pointer_cast<Generator<int>>(new_node), "channel");
            } else {
                new_node = std::make_shared<Looper<int>>();
                m_channel = std::make_unique<LooperComponent<int>>(std::dynamic_pointer_cast<Looper<int>>(new_node), "channel");
            }
            m_graph->set_channel(std::move(new_node));
            addAndMakeVisible(*m_channel);
        }

        resized();
    }


private:
    Transport m_transport;
    Scheduler m_scheduler;
    MidiRenderer m_midi_renderer;
    std::unique_ptr<SimplisticMidiGraphV1> m_graph;

    std::unique_ptr<Component> m_onset;
    std::unique_ptr<Component> m_duration;
    std::unique_ptr<Component> m_pitch;
    std::unique_ptr<Component> m_velocity;
    std::unique_ptr<Component> m_channel;

    juce::ComboBox m_onset_type;
    juce::ComboBox m_duration_type;
    juce::ComboBox m_pitch_type;
    juce::ComboBox m_velocity_type;
    juce::ComboBox m_channel_type;

    void populate_combo_box(juce::ComboBox* box) {
        addAndMakeVisible(box);
        box->addItem("Looper", LOOPER_ID);
        box->addItem("Generator", GENERATOR_ID);
        box->setSelectedId(1, juce::dontSendNotification);
        box->addListener(this);
    }


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
