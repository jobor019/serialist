

#ifndef SERIALIST_LOOPER_SCHEDULER_H
#define SERIALIST_LOOPER_SCHEDULER_H

#include <memory>
#include "events.h"

template<typename EventType = Event, std::enable_if_t<std::is_base_of_v<Event, EventType>, int> = 0>
class Scheduler {
public:
    void add_event(std::unique_ptr<EventType> event) {
        m_events.push_back(std::move(event));
    }


    void add_events(std::vector<std::unique_ptr<EventType>> events) {
        for (auto& event: events) {
            m_events.push_back(std::move(event));
        }
    }


    std::vector<std::unique_ptr<EventType>> poll(const TimePoint& time_point) {
        return poll(time_point.get_tick());
    }


    std::vector<std::unique_ptr<EventType>> poll(double current_time) {
        auto not_passed = [&](const std::unique_ptr<EventType>& event) {
            return event->get_time() > current_time;
        };

        auto passed_events_begin = std::partition(m_events.begin(), m_events.end(), not_passed);
        std::vector<std::unique_ptr<EventType>> passed_events;

        for (auto it = passed_events_begin; it != m_events.end(); ++it) {
            passed_events.push_back(std::move(*it));
        }

        m_events.erase(passed_events_begin, m_events.end());
        return passed_events;
    }

    std::vector<std::unique_ptr<EventType>> flush() {
        std::vector<std::unique_ptr<EventType>> v;
        std::swap(m_events, v);
        return v;
    }


    bool is_empty() {
        return m_events.empty();
    }


    template<typename T, std::enable_if_t<std::is_base_of_v<EventType, T>, int> = 0>
    bool has_event_of_type() {
        return std::any_of(m_events.begin(), m_events.end()
                           , [](const std::unique_ptr<EventType>& event) {
                    return (dynamic_cast<T*>(event.get()) != nullptr);
                });
    }




    std::vector<EventType*> peek() const {
        std::vector<EventType*> output;
        output.reserve(m_events.size());

        std::transform(m_events.begin(), m_events.end(), std::back_inserter(output)
                       , [&](const std::unique_ptr<EventType>& v) { return v.get(); });

        return output;
    }


    void remove(const std::function<bool(const std::unique_ptr<EventType>&)>& condition) {
        m_events.erase(std::remove_if(m_events.begin(), m_events.end(), condition), m_events.end());
    }


    std::size_t size() {
        return m_events.size();
    }


private:
    std::vector<std::unique_ptr<EventType>> m_events;
};

#endif //SERIALIST_LOOPER_SCHEDULER_H
