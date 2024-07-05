

#ifndef SERIALISTLOOPER_NOTE_VIEW_H
#define SERIALISTLOOPER_NOTE_VIEW_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "look_and_feel.h"
#include "core/generatives/note_source_LEGACY.h"

namespace serialist {

class NoteView : public juce::Component
                 , private juce::Timer {
public:

    class SimpleBlink : public juce::Component {
    public:
        void paint(juce::Graphics& g) override {
            if (is_on)
                g.setColour(getLookAndFeel().findColour(Colors::object_on));
            else
                g.setColour(getLookAndFeel().findColour(Colors::object_off));
            g.fillEllipse(getLocalBounds().toFloat());
        }


        void set_is_on(bool v) {
            is_on = v;
            repaint();
        }


    private:
        bool is_on = false;
    };


    explicit NoteView(NoteSource& midi_source)
            : m_midi_source(midi_source) {
        m_label.setText("0 0 0", juce::dontSendNotification);
        m_label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(m_label);
        addAndMakeVisible(m_note_on);
        startTimer(25);
    }


    static int height_of() {
        return 36;
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(Colors::object_background));
        g.setColour(getLookAndFeel().findColour(Colors::object_border_color));
        g.drawRect(getLocalBounds());
    }


    void resized() override {
        auto bounds = getLocalBounds();
        m_label.setBounds(bounds.removeFromLeft(proportionOfWidth(0.8f)));
        m_note_on.setBounds(bounds.reduced(8));
    }


private:
    void timerCallback() override {
        auto notes = m_midi_source.get_played_notes();

        if (notes.empty()) {
            if (juce::Time::currentTimeMillis() - last_found_note > 125) {
                m_note_on.set_is_on(false);
            }
            return;
        }

        m_note_on.set_is_on(true);


        last_found_note = juce::Time::currentTimeMillis();

        auto last_note = notes.back();
        std::ostringstream oss;
        oss << (last_note.get_midi_cents() / 100) << " " << (last_note.get_velocity()) << " "
            << (last_note.get_channel());

        m_label.setText(oss.str(), juce::dontSendNotification);
        // TODO repaint?

    }


    NoteSource& m_midi_source;

    juce::Label m_label;
    SimpleBlink m_note_on;
    juce::int64 last_found_note = 0;

};

namespace serialist {


#endif //SERIALISTLOOPER_NOTE_VIEW_H
