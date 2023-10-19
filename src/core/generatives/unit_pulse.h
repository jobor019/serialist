
#ifndef SERIALISTLOOPER_UNIT_PULSE_H
#define SERIALISTLOOPER_UNIT_PULSE_H


#include "core/algo/time/events.h"
#include "core/generative.h"
#include "core/param/parameter_keys.h"

class UnitPulse : public Node<TriggerEvent> {
public:
    static const inline std::string CLASS_NAME = "pulsator";


    UnitPulse(const std::string& id, ParameterHandler& parent)
            : m_parameter_handler(id, parent) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
    }


    void update_time(const TimePoint& t) override {
        m_current_value = Voices<TriggerEvent>::singular(TriggerEvent(t.get_tick(), Trigger::pulse_on, 1));
    }


    Voices<TriggerEvent> process() override {
        return m_current_value;
    }


    std::vector<Generative*> get_connected() override { return {}; }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


private:
    ParameterHandler m_parameter_handler;

    Voices<TriggerEvent> m_current_value = Voices<TriggerEvent>::empty_like();

};


#endif //SERIALISTLOOPER_UNIT_PULSE_H
