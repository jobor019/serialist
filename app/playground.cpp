#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include "look_and_feel.h"
#include "parameter_policy.h"
#include "key_state.h"
#include "transport.h"
#include "generation_graph.h"
#include "configuration_layer_component.h"
#include "generator_module.h"
#include "pulsator_module.h"
#include "interpolation_module.h"
#include "note_source_module.h"

class SomeObject : public ParameterHandler {
public:
    explicit SomeObject(ParameterHandler&& handler) : ParameterHandler(std::move(handler)) {}
};

class PlaygroundComponent : public MainKeyboardFocusComponent
                            , private juce::HighResolutionTimer
                            , private juce::ValueTree::Listener {
public:


    PlaygroundComponent()
            : m_some_handler(m_undo_manager)
              , m_modular_generator(m_some_handler)
              , m_config_layer_component(m_modular_generator)

    {

        m_lnf = std::make_unique<SerialistLookAndFeel>();
        SerialistLookAndFeel::setup_look_and_feel_colors(*m_lnf);
        juce::Desktop::getInstance().setDefaultLookAndFeel(m_lnf.get());

        addAndMakeVisible(m_config_layer_component);

        m_transport.start();
        startTimer(1);
        setSize(1000, 400);

        m_modular_generator.get_parameter_handler().get_value_tree().addListener(this);

//        auto [pulsator_module, pulsator_generatives] = ModuleFactory::new_pulsator(m_modular_generator, PulsatorModule::Layout::note_source_internal);
//        auto& generative = pulsator_module->get_generative();
//        if (generative) {
//            auto* pulsator = dynamic_cast<Node<Trigger>*>(&generative);
//            std::cout << "dc worked\n";
//        } else {
//            std::cout << "generative not set\n";
//        }

    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumaquamarine);

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello Playground!", getLocalBounds(), juce::Justification::centred, true);
    }


    void resized() override {
        m_config_layer_component.setBounds(getLocalBounds().reduced(10));

    }


    void recurse(juce::Component* component, std::vector<juce::Component*>& components) {
        for (auto* child: component->getChildren()) {
            if (child && child->getComponentID().isNotEmpty()) {
                components.push_back(child);
                recurse(child, components);
            }
        }
    }


    juce::Component* find_recursively(juce::Component* component, const juce::String& component_id) {
        if (!component)
            return nullptr;

        if (component->getComponentID().equalsIgnoreCase(component_id))
            return component;

        for (auto* child: component->getChildren()) {
            juce::Component* foundComponent = find_recursively(child, component_id);
            if (foundComponent)
                return foundComponent;
        }

        return nullptr;
    }


    void hiResTimerCallback() override {
        m_modular_generator.process(m_transport.update_time());

        ++callback_count;

        if (callback_count % 1000 == 0) {
            std::cout << m_modular_generator.get_parameter_handler().get_value_tree().toXmlString() << "\n";
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
    Transport m_transport;

    std::unique_ptr<juce::LookAndFeel> m_lnf;


    juce::UndoManager m_undo_manager;

    ParameterHandler m_some_handler;

    GenerationGraph m_modular_generator;

    ConfigurationLayerComponent m_config_layer_component;

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

