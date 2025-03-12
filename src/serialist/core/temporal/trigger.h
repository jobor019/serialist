
#ifndef SERIALISTLOOPER_TRIGGER_H
#define SERIALISTLOOPER_TRIGGER_H

#include "core/collections/vec.h"
#include "core/collections/voices.h"

namespace serialist {

class TriggerIds {
public:
    static inline const std::size_t NO_ID = 0;
    static inline const std::size_t FIRST_ID = NO_ID + 1;

    static TriggerIds& get_instance() {
        static TriggerIds instance;
        return instance;
    }

    ~TriggerIds() = default;

    TriggerIds(TriggerIds const&) = delete;

    static std::size_t new_or(const std::optional<std::size_t>& id) {
        // Note: using std::optional::value_or is not a good solution here as that will
        //       unnecessarily call next_id even when `id` is defined
        if (id) {
            return *id;
        } else {
            return get_instance().next_id();
        }
    }

    void operator=(TriggerIds const&) = delete;

    TriggerIds(TriggerIds&&) noexcept = delete;

    TriggerIds& operator=(TriggerIds&&) noexcept = delete;

    std::size_t next_id() {
        std::lock_guard lock{m_mutex};
        auto id = m_next_id;
        m_next_id = utils::increment(m_next_id, FIRST_ID);
        return id;
    }

    std::size_t peek_next_id() {
        std::lock_guard lock{m_mutex};
        return m_next_id;
    }

private:
    TriggerIds() = default;

    std::mutex m_mutex;

    std::size_t m_next_id = FIRST_ID;
};


// ==============================================================================================

class Trigger {
public:

    enum class Type {
        pulse_off = 0
        , pulse_on = 1
    };

    static Trigger without_id(const Type& type) {
        return {type, TriggerIds::NO_ID};
    }

    static Trigger with_manual_id(const Type& type, std::size_t id) {
        return {type, id};
    }

    static Trigger pulse_off(std::optional<std::size_t> id) {
        return {Type::pulse_off, id.value_or(TriggerIds::NO_ID)};
    }

    static Trigger pulse_on() {
        return {Type::pulse_on, TriggerIds::get_instance().next_id()};
    }

    static bool contains(const Vec<Trigger>& triggers, const Type& t) {
        return triggers.contains([&t](const Trigger& trigger) {
            return trigger.get_type() == t;
        });
    }

    static bool contains(const Voices<Trigger>& triggers, const Type& t) {
        return triggers.vec().any([&t](const Vec<Trigger>& trigger) {
            return Trigger::contains(trigger, t);
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

    static bool contains_pulse_on(const Voices<Trigger>& triggers) {
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

    static bool contains_pulse_off(const Voices<Trigger>& triggers) {
        return contains(triggers, Type::pulse_off);
    }

    static bool contains_any_pulse(const Vec<Trigger>& triggers) {
        return contains_pulse_on(triggers) || contains_pulse_off(triggers);
    }

    static std::string format(Type type, std::optional<std::size_t> id) {
        std::string t = type == Type::pulse_on ? "pulse_on" : "pulse_off";
            t += "(" + ( id ? std::to_string(*id) : "*" )+ ")";
        return t;
    }

    bool operator==(const Trigger& other) const {
        return m_type == other.m_type && m_id == other.m_id;
    }

    bool operator==(const Type& type) const {
        return is(type);
    }

    explicit operator std::string() const {
        return std::string("Trigger(type=") + std::to_string(static_cast<int>(m_type)) +
               ", id=" + std::to_string(m_id) + ")";
    }


    friend std::ostream& operator<<(std::ostream& os, const Trigger& trigger) {
        os << static_cast<std::string>(trigger) << ")";
        return os;
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
    Trigger(const Type& type, std::size_t id) : m_type(type), m_id(id) {}

    Type m_type;
    std::size_t m_id;

};


} // namespace serialist

#endif //SERIALISTLOOPER_TRIGGER_H
