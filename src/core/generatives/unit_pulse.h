
#ifndef SERIALISTLOOPER_UNIT_PULSE_H
#define SERIALISTLOOPER_UNIT_PULSE_H


#include "core/generative.h"
#include "core/param/parameter_keys.h"
#include "core/algo/time/trigger.h"
#include "core/algo/time/time_point.h"

class UnitPulse : public Node<Trigger> {
public:
    static const inline std::string CLASS_NAME = "pulsator";


    UnitPulse(const std::string& id, ParameterHandler& parent)
            : m_parameter_handler(id, parent) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
    }


    void update_time(const TimePoint&) override {}


    Voices<Trigger> process() override {
        return m_current_value;
    }


    std::vector<Generative*> get_connected() override { return {}; }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


private:
    ParameterHandler m_parameter_handler;

    Voices<Trigger> m_current_value = Voices<Trigger>::singular(Trigger::without_id(Trigger::Type::pulse_on));

};


#endif //SERIALISTLOOPER_UNIT_PULSE_H
