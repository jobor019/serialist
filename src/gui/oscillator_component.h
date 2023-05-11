#ifndef SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
#define SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

template<typename T>
class OscillatorComponent : public juce::Component
                            , public juce::Timer {
public:
    explicit OscillatorComponent(Generator<T>* generator)
            : m_generator(generator)
              , m_queue(10) {
        startTimerHz(50);
    }

    void timerCallback() override {
        double x = m_generator->get_phasor_position();
        if (m_queue.empty() || std::abs(x - m_queue.back()) > 1e-4) {
            m_queue.push(x);
            repaint();
        }
    }

private:
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::red);
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds());
        g.setColour(juce::Colours::aquamarine);
        if (!m_queue.empty()) {
//            g.fillRect(getLocalBounds().getX()
//                       , getLocalBounds().getY()
//                       , getLocalBounds().getWidth()
//                       , static_cast<int>(getLocalBounds().getHeight() * m_queue.back()));
            auto bounds = getLocalBounds();
            int staple_width = bounds.getWidth() / m_queue.size();
            auto step_size = std::abs(m_generator->get_step_size());
            auto separator_width = 2;
//            if (step_size != 0.0) {
//                staple_width = static_cast<int>(step_size * (bounds.getWidth() - separator_width * static_cast<int>(1/step_size)));
//            }

            for (auto& value: m_queue.get_snapshot()) {
                auto staple = bounds.removeFromLeft(staple_width);
                g.fillRect(staple.getX()
                       , staple.getY()
                       , staple.getWidth()
                       , static_cast<int>(staple.getHeight() * value));
                bounds.removeFromLeft(separator_width);
            }
        }

//        g.drawText(std::to_string(m_generator->get_phasor_position()), getLocalBounds(), juce::Justification::Flags::horizontallyCentred);

    }

    void resized() override {}


private:
    Generator<T>* m_generator;
    utils::Queue m_queue;


};

#endif //SERIALIST_LOOPER_OSCILLATOR_COMPONENT_H
