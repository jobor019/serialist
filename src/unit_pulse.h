
#ifndef SERIALISTLOOPER_UNIT_PULSE_H
#define SERIALISTLOOPER_UNIT_PULSE_H


#include "events.h"
#include "generative.h"

class UnitPulse : public Node<Trigger>
                  , public Stateful {
public:
    static const inline std::string CLASS_NAME = "pulsator";


    UnitPulse(const std::string& id, ParameterHandler& parent)
            : m_parameter_handler(id, parent) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
    }


    void update_time(const TimePoint& t) override {
        m_current_value = Voices<Trigger>(Trigger(t.get_tick(), Trigger::Type::pulse, 1));
    }


    Voices<Trigger> process() override {
        return m_current_value;
    }


    std::vector<Generative*> get_connected() override { return {}; }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


    void disconnect_if(Generative&) override {}


private:
    ParameterHandler m_parameter_handler;

    Voices<Trigger> m_current_value = Voices<Trigger>::create_empty_like();

};


#endif //SERIALISTLOOPER_UNIT_PULSE_H
