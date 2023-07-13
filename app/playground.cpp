#include <juce_gui_extra/juce_gui_extra.h>
#include <scalable_slider.h>

#include <memory>
#include "look_and_feel.h"
#include "parameter_policy.h"
#include "generative_component.h"
#include "oscillator.h"
#include "modular_generator.h"
#include "slider_widget.h"
//#include "modules/oscillator_module.h"
//#include "modular_generator.h"
//#include "module_factory.h"
//#include "modules/generator_module.h"
//#include "modules/text_sequence_module.h"
//#include "modules/interpolation_module.h"
//
//#include "widgets/header_widget.h"
#include "widgets/combobox_widget.h"
#include "oscillator_module.h"
#include "module_factory.h"
//#include "modules/note_source_module.h"

#include "generator.h"
#include "configuration_layer_component.h"

class SomeObject : public ParameterHandler {
public:
    SomeObject(ParameterHandler&& handler) : ParameterHandler(std::move(handler)) {}
};

class PlaygroundComponent : public MainKeyboardFocusComponent
                            , private juce::HighResolutionTimer
                            , private juce::ValueTree::Listener {
public:


    PlaygroundComponent()
//            : m_some_handler(m_undo_manager)
            : m_modular_generator(ParameterHandler(m_undo_manager))
              , m_config_layer_component(m_modular_generator)
//              , m_pitch("pitch", m_some_handler)
//              , s(ScalableSlider::Orientation::vertical)
//              , hc("header", m_some_handler)
//              , my_cb("osctype"
//                      , m_some_handler
//                      , {{  "sin", Oscillator::Type::sin}
//                         , {"sqr", Oscillator::Type::square}}
//                      , Oscillator::Type::sin
//                      , "helo")
    {

//        auto e = SomeObject(std::move(m_some_handler));

        m_lnf = std::make_unique<SerialistLookAndFeel>();
        SerialistLookAndFeel::setup_look_and_feel_colors(*m_lnf);
        juce::Desktop::getInstance().setDefaultLookAndFeel(m_lnf.get());


//        auto [oscillator, g1] = ModuleFactory::new_oscillator("osc1", m_modular_generator);
//        m_oscillator = std::move(oscillator);
//        m_modular_generator.add(std::move(g1));
//        addAndMakeVisible(*m_oscillator);
//
//        auto [sequence, g2] = ModuleFactory::new_text_sequence<int>("seq1", m_modular_generator);
//        m_sequence = std::move(sequence);
//        m_modular_generator.add(std::move(g2));
//        addAndMakeVisible(*m_sequence);
//
//        auto [interpolator, g3] = ModuleFactory::new_interpolator<int>("seq1", m_modular_generator);
//        m_interpolator = std::move(interpolator);
//        m_modular_generator.add(std::move(g3));
//        addAndMakeVisible(*m_interpolator);
//
//        auto [source, g4] = ModuleFactory::new_midi_note_source("src1", m_modular_generator);
//        m_source = std::move(source);
//        m_modular_generator.add(std::move(g4));
//        addAndMakeVisible(*m_source);
//
//        auto [pitch_generator, g5] = ModuleFactory::new_generator<int>("pitchg1", m_modular_generator);
//        m_pitch_generator = std::move(pitch_generator);
//        m_modular_generator.add(std::move(g5));
//        addAndMakeVisible(*m_pitch_generator);

        addAndMakeVisible(m_config_layer_component);

//        auto* generator = dynamic_cast<Node<int>*>(m_modular_generator.find("pitchg1"));
//        auto* midi_source = dynamic_cast<MidiNoteSource*>(m_modular_generator.find("src1"));
//        if (!generator || !midi_source) {
//            throw std::runtime_error("Failed to convert");
//        }

//        midi_source->set_pitch(generator);


//        m_model.add(std::move(generatives));


//        m_oscillator = std::move(oscillator);
//
//                          addAndMakeVisible(*m_oscillator);



//        auto oscillator_data = ModuleFactory::new_oscillator("osc1", m_model, OscillatorModule::Layout::full);

//        m_model.add(std::move(oscillator_data.generatives));
//        m_oscillator = std::move(oscillator_data.component);


//        addAndMakeVisible(m_oscillator);

//        addAndMakeVisible(m_sequence);
//        addAndMakeVisible(m_interpolator);
//        addAndMakeVisible(m_pitch);
//        addAndMakeVisible(s);
//        addAndMakeVisible(m_source);

//        tb.setButtonText("HH");
//        addAndMakeVisible(tb);
//
//        addAndMakeVisible(hc);
//
//        addAndMakeVisible(my_cb);

//        std::cout << m_some_handler.get_value_tree().toXmlString() << std::endl;

        m_transport.start();
        startTimer(1);
        setSize(1000, 400);

        m_modular_generator.get_value_tree().addListener(this);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumaquamarine);

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello Playground!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
//        m_oscillator->setBounds(50, 30, OscillatorModule::width_of(), OscillatorModule::height_of());
//
//        m_sequence->setBounds(300, 30, 100, TextSequenceModule<int>::height_of());
//        m_interpolator->setBounds(50, 200, InterpolationModule<int>::width_of(), InterpolationModule<int>::height_of());
//        m_pitch_generator->setBounds(300, 200, GeneratorModule<int>::width_of(), GeneratorModule<int>::height_of());
//        m_pitch_generator->setBounds(300, 200, 100, 100);
//        s.setBounds(300, 100, 12, 40);
//        tb.setBounds(350, 100, 40, 40);
//        hc.setBounds(400, 100, 200, 20);
//        my_cb.setBounds(500, 40, 80, 50);
//        m_source->setBounds(50, 270, NoteSourceModule::width_of(), NoteSourceModule::height_of());

        m_config_layer_component.setBounds(getLocalBounds().reduced(10));

    }


    void hiResTimerCallback() override {
//        auto ee = dynamic_cast<Oscillator*>(&m_oscillator.get_generative())->process(TimePoint());
//        m_oscillator.repaint();
        m_modular_generator.process(m_transport.update_time());

        ++callback_count;

        if (callback_count % 1000 == 0) {
//            std::cout << m_modular_generator.get_value_tree().toXmlString() << "\n";
        }

    }


    void globalFocusChanged(juce::Component* focusedComponent) override {
        if (focusedComponent) {
            std::cout << "focused component dims " << focusedComponent->getWidth() << " "
                      << focusedComponent->getHeight() << "\n";
        } else {
            std::cout << "nullptr\n";
        }
        MainKeyboardFocusComponent::globalFocusChanged(focusedComponent);
    }

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier &) override {
//        std::cout << m_modular_generator.get_value_tree().toXmlString() << "\n";
    }


private:
    Transport m_transport;

    std::unique_ptr<juce::LookAndFeel> m_lnf;

    juce::UndoManager m_undo_manager;

//    ParameterHandler m_some_handler;

    ModularGenerator m_modular_generator;
//
//    std::unique_ptr<OscillatorModule> m_oscillator;
//    std::unique_ptr<TextSequenceModule<int>> m_sequence;
//    std::unique_ptr<InterpolationModule<int>> m_interpolator;
//    std::unique_ptr<GeneratorModule<int>> m_pitch_generator;
//
//    std::unique_ptr<NoteSourceModule> m_source;

    ConfigurationLayerComponent m_config_layer_component;

//    juce::ToggleButton tb;

//    ScalableSlider s;

//    HeaderWidget hc;

//    ComboBoxWidget<Oscillator::Type> my_cb;

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

