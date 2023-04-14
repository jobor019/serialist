

#ifndef SERIALIST_LOOPER_GENERATION_GRAPH_H
#define SERIALIST_LOOPER_GENERATION_GRAPH_H

#include "transport.h"
#include "events.h"
#include "graph_node.h"

#include <vector>
#include <optional>

class GenerationGraph {
public:
    virtual ~GenerationGraph() = default;

    virtual std::vector<std::unique_ptr<Event>> process(const TimePoint& t) = 0;
};


// ==============================================================================================


class MidiGraph : public GenerationGraph {

};


// ==============================================================================================

class SimplisticMidiGraphV1 : public MidiGraph {
public:

    enum class Node {
        onset = 0
        , duration = 1
        , pitch = 2
        , velocity = 3
        , channel = 4
    };


    SimplisticMidiGraphV1(std::shared_ptr<GraphNode<double>>&& onset
                          , std::shared_ptr<GraphNode<double>>&& duration
                          , std::shared_ptr<GraphNode<int>>&& pitch
                          , std::shared_ptr<GraphNode<int>>&& velocity
                          , std::shared_ptr<GraphNode<int>>&& channel)
            : m_onset(std::move(onset))
              , m_duration(std::move(duration))
              , m_pitch(std::move(pitch))
              , m_velocity(std::move(velocity))
              , m_channel(std::move(channel)) {}

    std::vector<std::unique_ptr<Event>> process(const TimePoint& t) override {
//        auto note_pitch = get_first_if_applicable(m_pitch->process(t));
//        auto note_velocity = get_first_if_applicable(m_velocity->process(t));
//        auto note_duration = get_first_if_applicable(m_duration->process(t));
//        auto note_channel = get_first_if_applicable(m_channel->process(t));
//        auto next_onset = get_first_if_applicable(m_onset->process(t));
//
//        std::vector<std::unique_ptr<Event>> events;
//
//        if (!(note_pitch && note_velocity && note_duration && note_channel && next_onset)) {
//            // if any is nullopt: ignore all and requeue trigger
//            events.emplace_back(std::make_unique<TriggerEvent>(t.get_tick() + 1));
//            return events;
//        }
//
//        auto note = MidiEvent::note(t.get_tick()
//                                    , note_pitch.value()
//                                    , note_velocity.value()
//                                    , note_channel.value()
//                                    , next_onset.value() * note_duration.value());
//        events.emplace_back(std::make_unique<MidiEvent>(note.first));
//        events.emplace_back(std::make_unique<MidiEvent>(note.second));
//        events.emplace_back(std::make_unique<TriggerEvent>(next_onset.value() + t.get_tick()));
//
//        return events;
        // TODO: TEMP
        (void) t;
        return {};
    }

    // TODO: Find proper solution for templated type
    template<typename T>
    void set_node(int id, std::shared_ptr<GraphNode<T> > new_node) {
        switch (id) {
            case static_cast<int>(Node::onset):
                m_onset = std::move(new_node);
                break;
            case static_cast<int>(Node::duration):
                m_duration = std::move(new_node);
                break;
            case static_cast<int>(Node::pitch):
                m_pitch = std::move(new_node);
                break;
            case static_cast<int>(Node::velocity):
                m_velocity = std::move(new_node);
                break;
            case static_cast<int>(Node::channel):
                m_channel = std::move(new_node);
                break;
            default:
                throw std::runtime_error("invalid id");
        }
    }

    template<typename T>
    void set_node(Node node_position, std::shared_ptr<GraphNode<T> > new_node) {
        set_node(static_cast<int>(node_position), std::move(new_node));
    }

    void set_onset(std::shared_ptr<GraphNode<double>> new_node) {
        m_onset = std::move(new_node);
    }

    void set_duration(std::shared_ptr<GraphNode<double>> new_node) {
        m_duration = std::move(new_node);
    }

    void set_pitch(std::shared_ptr<GraphNode<int>> new_node) {
        m_pitch = std::move(new_node);
    }

    void set_velocity(std::shared_ptr<GraphNode<int>> new_node) {
        m_velocity = std::move(new_node);
    }

    void set_channel(std::shared_ptr<GraphNode<int>> new_node) {
        m_channel = std::move(new_node);
    }


private:
    template<typename T>
    std::optional<T> get_first_if_applicable(std::vector<T> values) {
        if (values.empty()) {
            return std::nullopt;
        }
        return values[0];
    }


    std::shared_ptr<GraphNode<double> > m_onset;
    std::shared_ptr<GraphNode<double> > m_duration;
    std::shared_ptr<GraphNode<int> > m_pitch;
    std::shared_ptr<GraphNode<int> > m_velocity;
    std::shared_ptr<GraphNode<int> > m_channel;

};


#endif //SERIALIST_LOOPER_GENERATION_GRAPH_H
