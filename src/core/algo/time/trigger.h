
#ifndef SERIALISTLOOPER_TRIGGER_H
#define SERIALISTLOOPER_TRIGGER_H

#include "core/collections/vec.h"

class Trigger {
public:
    static const int NO_ID = -1;

    enum class Type {
        pulse_off = 0
        , pulse_on = 1
    };

    explicit Trigger(const Type& type, int id) : m_type(type), m_id(id) {}

    static Trigger without_id(const Type& type) {
        return Trigger(type, NO_ID);
    }

    static Trigger pulse_off(int id) {
        return Trigger(Type::pulse_off, id);
    }

    static Trigger pulse_on(int id) {
        return Trigger(Type::pulse_on, id);
    }

    static bool contains(const Vec<Trigger>& triggers, const Type& t) {
        return triggers.contains([&t](const Trigger& trigger) {
            return trigger.get_type() == t;
        });
    }

    static bool contains_pulse_on(const Vec<Trigger>& triggers) {
        return contains(triggers, Type::pulse_on);
    }

    bool operator==(const Trigger& other) const {
        return m_type == other.m_type && m_id == other.m_id;
    }

    bool operator==(const Type& type) const {
        return is(type);
    }

    bool terminates(const Trigger& start) const {
        return start.m_type == Type::pulse_on && m_type == Type::pulse_off && start.m_id == m_id;
    }

    bool is(const Type& type) const {
        return m_type == type;
    }

    Type get_type() const { return m_type; }

    int get_id() const { return m_id; }


private:
    Type m_type;
    int m_id;

};


#endif //SERIALISTLOOPER_TRIGGER_H
