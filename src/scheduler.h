

#ifndef SERIALIST_LOOPER_SCHEDULER_H
#define SERIALIST_LOOPER_SCHEDULER_H

#include <memory>
#include "events.h"

class Scheduler {
public:
    void add_event(std::unique_ptr<Event> event) {
        m_events.push_back(std::move(event));
    }


    void add_events(std::vector<std::unique_ptr<Event>> events) {
        for (auto& event: events) {
            m_events.push_back(std::move(event));
        }
    }


    std::vector<std::unique_ptr<Event>> get_events(const TimePoint& time_point) {
        return get_events(time_point.get_tick());
    }


    std::vector<std::unique_ptr<Event>> get_events(double current_time) {
        auto not_passed = [&](const std::unique_ptr<Event>& event) {
            return event->get_time() > current_time;
        };

        auto passed_events_begin = std::partition(m_events.begin(), m_events.end(), not_passed);
        std::vector<std::unique_ptr<Event>> passed_events;

        for (auto it = passed_events_begin; it != m_events.end(); ++it) {
            passed_events.push_back(std::move(*it));
        }

        m_events.erase(passed_events_begin, m_events.end());
        return passed_events;
    }


    bool is_empty() {
        return m_events.empty();
    }


    template<typename T, std::enable_if_t<std::is_base_of_v<Event, T>, int> = 0>
    bool has_event_of_type() {
        return std::any_of(m_events.begin(), m_events.end()
                           , [](const std::unique_ptr<Event>& event) {
                    return (dynamic_cast<T*>(event.get()) != nullptr);
                });
    }


    bool has_trigger() {
        return has_event_of_type<TriggerEvent>();
    }


    std::size_t size() {
        return m_events.size();
    }


private:
    std::vector<std::unique_ptr<Event>> m_events;
};

#endif //SERIALIST_LOOPER_SCHEDULER_H
