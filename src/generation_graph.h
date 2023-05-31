

#ifndef SERIALIST_LOOPER_GENERATION_GRAPH_H
#define SERIALIST_LOOPER_GENERATION_GRAPH_H

#include "transport.h"
#include "events.h"
#include "generative.h"

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


    SimplisticMidiGraphV1(std::unique_ptr<Node<double>> onset
                          , std::unique_ptr<Node<double>> duration
                          , std::unique_ptr<Node<int>> pitch
                          , std::unique_ptr<Node<int>> velocity
                          , std::unique_ptr<Node<int>> channel)
            : m_onset(std::move(onset))
              , m_duration(std::move(duration))
              , m_pitch(std::move(pitch))
              , m_velocity(std::move(velocity))
              , m_channel(std::move(channel)) {}

    std::vector<std::unique_ptr<Event>> process(const TimePoint& t) override {
        auto note_pitch = m_pitch->process(t);
        auto note_velocity = m_velocity->process(t);
        auto note_duration = m_duration->process(t);
        auto note_channel = m_channel->process(t);
        auto next_onset = m_onset->process(t);

        std::vector<std::unique_ptr<Event>> events;

        if (note_pitch.empty() || note_velocity.empty() || note_duration.empty() ||
              note_channel.empty() || next_onset.empty()) {
            // if any is missing: ignore all and requeue trigger
            events.emplace_back(std::make_unique<TriggerEvent>(t.get_tick() + 1));
            return events;
        }

        auto note = MidiEvent::note(t.get_tick()
                                    , note_pitch.at(0)
                                    , note_velocity.at(0)
                                    , note_channel.at(0)
                                    , next_onset.at(0) * note_duration.at(0));
        events.emplace_back(std::make_unique<MidiEvent>(note.first));
        events.emplace_back(std::make_unique<MidiEvent>(note.second));
        events.emplace_back(std::make_unique<TriggerEvent>(next_onset.at(0) + t.get_tick()));

        return events;
    }

    template<typename T>
    void set_node(Node node, std::unique_ptr<Node<T> > new_node) {
        switch (node) {
            case Node::onset:
                m_onset = std::move(new_node);
                break;
            case Node::duration:
                m_duration = std::move(new_node);
                break;
            case Node::pitch:
                m_pitch = std::move(new_node);
                break;
            case Node::velocity:
                m_velocity = std::move(new_node);
                break;
            case Node::channel:
                m_channel = std::move(new_node);
                break;
            default:
                throw std::runtime_error("invalid id");
        }
    }

    [[nodiscard]] Node<double>* get_onset() const { return m_onset.get();}

    [[nodiscard]] Node<double>* get_duration() const { return m_duration.get();}

    [[nodiscard]] Node<int>* get_pitch() const { return m_pitch.get();}

    [[nodiscard]] Node<int>* get_velocity() const { return m_velocity.get();}

    [[nodiscard]] Node<int>* get_channel() const { return m_channel.get();}


//    void set_onset(std::unique_ptr<Node<double>> new_node) {
//        m_onset = std::move(new_node);
//    }
//
//
//    void set_duration(std::unique_ptr<Node<double>> new_node) {
//        m_duration = std::move(new_node);
//    }
//
//
//    void set_pitch(std::unique_ptr<Node<int>> new_node) {
//        m_pitch = std::move(new_node);
//    }
//
//
//    void set_velocity(std::unique_ptr<Node<int>> new_node) {
//        m_velocity = std::move(new_node);
//    }
//
//
//    void set_channel(std::unique_ptr<Node<int>> new_node) {
//        m_channel = std::move(new_node);
//    }


private:
//    template<typename DataType>
//    std::optional<DataType> get_first_if_applicable(std::vector<DataType> values) {
//        if (values.empty()) {
//            return std::nullopt;
//        }
//        return values[0];
//    }


    std::unique_ptr<Node<double> > m_onset;
    std::unique_ptr<Node<double> > m_duration;
    std::unique_ptr<Node<int> > m_pitch;
    std::unique_ptr<Node<int> > m_velocity;
    std::unique_ptr<Node<int> > m_channel;

};


#endif //SERIALIST_LOOPER_GENERATION_GRAPH_H
