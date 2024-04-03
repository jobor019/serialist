
#ifndef SERIALISTLOOPER_TRIGGER_H
#define SERIALISTLOOPER_TRIGGER_H

#include "core/collections/vec.h"

class Trigger {
public:
    static inline const std::size_t NO_ID = 0;

    enum class Type {
        pulse_off = 0
        , pulse_on = 1
    };

    explicit Trigger(const Type& type, std::size_t id) : m_type(type), m_id(id) {}

    static Trigger without_id(const Type& type) {
        return Trigger(type, NO_ID);
    }

    static Trigger pulse_off(std::optional<std::size_t> id) {
        return Trigger(Type::pulse_off, id.value_or(NO_ID));
    }

    static Trigger pulse_on(std::optional<std::size_t> id) {
        return Trigger(Type::pulse_on, id.value_or(NO_ID));
    }

    static bool contains(const Vec<Trigger>& triggers, const Type& t) {
        return triggers.contains([&t](const Trigger& trigger) {
            return trigger.get_type() == t;
        });
    }

    static bool contains(const Vec<Trigger>& triggers, const Type& t, std::size_t id) {
        return triggers.contains([&t, id](const Trigger& trigger) {
            return trigger.get_type() == t && trigger.get_id() == id;
        });
    }

    static bool contains_any(const Vec<Trigger>& triggers, const Vec<Type>& types) {
        return std::any_of(types.begin(), types.end(), [&triggers](const Type& t) {
            return contains(triggers, t);
        });
    }

    static bool contains_pulse_on(const Vec<Trigger>& triggers) {
        return contains(triggers, Type::pulse_on);
    }

    static bool contains_pulse_on(const Vec<Trigger>& triggers, std::size_t id) {
        return contains(triggers, Type::pulse_on, id);
    }

    static bool contains_pulse_off(const Vec<Trigger>& triggers) {
        return contains(triggers, Type::pulse_off);
    }

    static bool contains_pulse_off(const Vec<Trigger>& triggers, std::size_t id) {
        return contains(triggers, Type::pulse_off, id);
    }

    static bool contains_any_pulse(const Vec<Trigger>& triggers) {
        return contains_pulse_on(triggers) || contains_pulse_off(triggers);
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

    std::size_t get_id() const { return m_id; }


private:
    Type m_type;
    std::size_t m_id;

};


#endif //SERIALISTLOOPER_TRIGGER_H
