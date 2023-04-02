

#ifndef SERIALIST_LOOPER_GENERATION_GRAPH_H
#define SERIALIST_LOOPER_GENERATION_GRAPH_H

#include "transport.h"
#include "events.h"
#include "graph_node.h"

#include <vector>

class GenerationGraph {
public:
    virtual std::vector<std::unique_ptr<Event>> process(const TimePoint& t) = 0;
};


class MidiGraph : public GenerationGraph {

};


class SimplisticMidiGraphV1 : public MidiGraph {
public:
    std::vector<std::unique_ptr<Event>> process(const TimePoint& t) override {
        auto note_pitch = m_pitch->process(t);
        auto note_velocity = m_velocity->process(t);
        auto note_duration = m_duration->process(t);
        auto note_channel = m_channel->process(t);
        auto next_onset = m_onset->process(t);

        std::vector<std::unique_ptr<Event>> events;
        auto note = MidiEvent::note(t.get_tick(), note_pitch, note_velocity, note_channel, note_duration);
        events.emplace_back(std::make_unique<MidiEvent>(note.first));
        events.emplace_back(std::make_unique<MidiEvent>(note.second));
        events.emplace_back(std::make_unique<TriggerEvent>(next_onset));

        return events;
    }


public:
private:
    std::unique_ptr<GraphNode<double> > m_onset;
    std::unique_ptr<GraphNode<double> > m_duration;
    std::unique_ptr<GraphNode<int> > m_pitch;
    std::unique_ptr<GraphNode<int> > m_velocity;
    std::unique_ptr<GraphNode<int> > m_channel;

};


#endif //SERIALIST_LOOPER_GENERATION_GRAPH_H
