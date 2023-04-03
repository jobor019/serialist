

#ifndef SERIALIST_LOOPER_GENERATION_GRAPH_H
#define SERIALIST_LOOPER_GENERATION_GRAPH_H

#include "transport.h"
#include "events.h"
#include "graph_node.h"

#include <vector>
#include <optional>

class GenerationGraph {
public:
    virtual std::vector<std::unique_ptr<Event>> process(const TimePoint& t) = 0;
};


class MidiGraph : public GenerationGraph {

};


class SimplisticMidiGraphV1 : public MidiGraph {
public:
    SimplisticMidiGraphV1(std::unique_ptr<GraphNode<double>>&& onset
                          , std::unique_ptr<GraphNode<double>>&& duration
                          , std::unique_ptr<GraphNode<int>>&& pitch
                          , std::unique_ptr<GraphNode<int>>&& velocity
                          , std::unique_ptr<GraphNode<int>>&& channel)
            : m_onset(std::move(onset))
              , m_duration(std::move(duration))
              , m_pitch(std::move(pitch))
              , m_velocity(std::move(velocity))
              , m_channel(std::move(channel)) {}

    std::vector<std::unique_ptr<Event>> process(const TimePoint& t) override {
        auto note_pitch = get_first_if_applicable(m_pitch->process(t));
        auto note_velocity = get_first_if_applicable(m_velocity->process(t));
        auto note_duration = get_first_if_applicable(m_duration->process(t));
        auto note_channel = get_first_if_applicable(m_channel->process(t));
        auto next_onset = get_first_if_applicable(m_onset->process(t));

        std::vector<std::unique_ptr<Event>> events;

        if (!(note_pitch && note_velocity && note_duration && note_channel && next_onset)) {
            // if any is nullopt: ignore all and requeue trigger
            events.emplace_back(std::make_unique<TriggerEvent>(t.get_tick() + 1));
            return events;
        }

        auto note = MidiEvent::note(t.get_tick()
                                    , note_pitch.value()
                                    , note_velocity.value()
                                    , note_channel.value()
                                    , note_duration.value());
        events.emplace_back(std::make_unique<MidiEvent>(note.first));
        events.emplace_back(std::make_unique<MidiEvent>(note.second));
        events.emplace_back(std::make_unique<TriggerEvent>(next_onset.value() + t.get_tick()));

        return events;
    }

private:
    template<typename T>
    std::optional<T> get_first_if_applicable(std::vector<T> values) {
        if (values.empty()) {
            return std::nullopt;
        }
        return values[0];
    }


    std::unique_ptr<GraphNode<double> > m_onset;
    std::unique_ptr<GraphNode<double> > m_duration;
    std::unique_ptr<GraphNode<int> > m_pitch;
    std::unique_ptr<GraphNode<int> > m_velocity;
    std::unique_ptr<GraphNode<int> > m_channel;

};


#endif //SERIALIST_LOOPER_GENERATION_GRAPH_H
