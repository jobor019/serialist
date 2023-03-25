#ifndef SERIALIST_LOOPER_MAIN_COMPONENT_H
#define SERIALIST_LOOPER_MAIN_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>


class MainComponent : public juce::Component
                      , private juce::HighResolutionTimer {
public:
    MainComponent() {
        setSize(600, 400);
        startTimer(1000);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);
        g.drawText("Hello World!", getLocalBounds(), juce::Justification::centred, true);
    }

    void resized() override {

    }


    void hiResTimerCallback() override {
        std::cout << "hehe\n";
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#endif //SERIALIST_LOOPER_MAIN_COMPONENT_H