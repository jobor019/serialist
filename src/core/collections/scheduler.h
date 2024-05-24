

#ifndef SERIALIST_LOOPER_SCHEDULER_H
#define SERIALIST_LOOPER_SCHEDULER_H

#include <memory>
#include "core/algo/temporal/transport.h"
#include "voices.h"
#include "multi_voiced.h"

template<typename EventType, typename TimePointType = double
         , typename = std::enable_if_t<std::is_convertible_v<
                decltype(std::declval<TimePointType>() <= std::declval<TimePointType>()), bool>>>
class Scheduler /* : public Flushable<ScheduledEvent> */ {
public:

    struct ScheduledEvent {
        EventType event;
        TimePointType time;
    };


    void schedule(EventType&& event, const TimePointType& t) {
        m_events.append({std::move(event), t});
    }

    Voice<ScheduledEvent> poll(const TimePointType& t) {
        return m_events.filter_drain([&t](const ScheduledEvent& e) { return e.time > t; });
//                .as_type<EventType>([](const ScheduledEvent& e) { return std::move(e.event); });
    }

    Voice<ScheduledEvent> flush() {
        return m_events.drain();
//                .as_type<EventType>([](const ScheduledEvent& e) { return std::move(e.event); });
    }

    Voice<ScheduledEvent> flush(std::function<bool(const ScheduledEvent&)> f) {
        return m_events.filter_drain(f);
//                .as_type<EventType>([](const ScheduledEvent& e) { return std::move(e.event); });
    }

    bool empty() const { return m_events.empty(); }

    void clear() {
        m_events.clear();
    }

    bool has_event_matching(std::function<bool(const EventType&)> condition) const {
        return m_events.contains([&condition](const ScheduledEvent& event) {
            return condition(event.event);
        });
    }

    std::size_t count() const { return m_events.size(); }

    // Vec<std::reference_wrapper<EventType>> peek() const {} // TODO: Not sure if this function is needed

private:
    Vec<ScheduledEvent> m_events;

};


//// ==============================================================================================
//
//template<typename EventType, typename TimePointType = double
//         , typename = std::enable_if_t<std::is_convertible_v<
//                decltype(std::declval<TimePointType>() <= std::declval<TimePointType>()), bool>>>
//class MultiVoiceScheduler {
//public:
//    explicit MultiVoiceScheduler(std::size_t num_voices) : m_schedulers(num_voices) {}
//
//    void schedule(EventType&& event, const TimePointType& t, std::size_t voice_index) {
//        m_schedulers.get_objects()[voice_index].schedule(std::move(event), t);
//    }
//
//    Voices<EventType> poll(const TimePointType& t) {
//        auto output = Voices<EventType>::zeros(m_schedulers.size());
//        for (std::size_t i = 0; i < m_schedulers.size(); ++i) {
//            output[i] = m_schedulers.get_objects()[i].poll(t);
//        }
//        return output;
//    }
//
//    Voices<EventType> poll(const TimePointType& t, std::size_t voice_index) {
//        return m_schedulers.get_objects()[voice_index].poll(t);
//    }
//
//    Voices<EventType> flush() {
//        return m_schedulers.flush();
//    }
//
//    Voices<EventType> flush(std::function<bool(const EventType&)> f) {
//        return m_schedulers.flush(f);
//    }
//
//    Voices<EventType> flush(std::size_t voice_index) {
//        return m_schedulers.flush(voice_index);
//    }
//
//    Voices<EventType> resize(std::size_t num_voices) {
//        return m_schedulers.resize(num_voices);
//    }
//
//    std::size_t size() const {
//        return m_schedulers.size();
//    }
//
//    Scheduler<EventType, TimePointType>& operator[](std::size_t voice_index) {
//        return m_schedulers.get_objects()[voice_index];
//    }
//
//
//private:
//    MultiVoiced<Scheduler<EventType, TimePointType>, EventType> m_schedulers;
//};


#endif //SERIALIST_LOOPER_SCHEDULER_H
