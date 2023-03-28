

#ifndef SERIALIST_LOOPER_GENERATION_GRAPH_H
#define SERIALIST_LOOPER_GENERATION_GRAPH_H

#include "transport.h"
#include "events.h"
#include "graph_node.h"

#include <vector>

class GenerationGraph {
public:
    virtual std::vector<Event> process(const TimePoint& t) = 0;
};


class MidiGraph : public GenerationGraph {

};


class SimplisticMidiGraphV1 : public MidiGraph {
public:
    std::vector<Event> process(const TimePoint& t) override {
        auto note_pitch = pitch->process(t);
        auto note_velocity = velocity->process(t);
        auto note_duration = duration->process(t);
        auto note_channel = channel->process(t);
        auto next_onset = onset->process(t);

        std::vector<Event> events;
        auto note = MidiEvent::note(t.get_tick(), note_pitch, note_velocity, note_channel, note_duration);
        events.emplace_back(note.first);
        events.emplace_back(note.second);
        events.emplace_back(TriggerEvent{next_onset});

        return events;
    }


public:
private:
    std::unique_ptr<GraphNode<double> > onset;
    std::unique_ptr<GraphNode<double> > duration;
    std::unique_ptr<GraphNode<int> > pitch;
    std::unique_ptr<GraphNode<int> > velocity;
    std::unique_ptr<GraphNode<int> > channel;

};


#endif //SERIALIST_LOOPER_GENERATION_GRAPH_H
