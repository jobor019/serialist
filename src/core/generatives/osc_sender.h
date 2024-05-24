
#ifndef SERIALISTLOOPER_OSC_SENDER_H
#define SERIALISTLOOPER_OSC_SENDER_H

#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/temporal/trigger.h"
#include "osc.h"
#include "core/algo/temporal/time_point.h"

class OscSenderNode : public RootBase {
public:

    struct OscTarget {
        std::string ip;
        int port;
    };

    static const inline std::string CLASS_NAME = "osc_sender";
    static const inline std::string OSC_ADDRESS = "osc_address";
    static const inline std::string INPUT = "input";
    static const inline std::string FLATTEN = "flatten";
    static const inline std::string CHANGE = "change";


    OscSenderNode(const std::string& identifier
                  , ParameterHandler& parent
                  , Node<std::string>* osc_address = nullptr
                  , Node<Facet>* input = nullptr
                  , Node<Trigger>* trigger = nullptr
                  , Node<Facet>* flatten = nullptr
                  , Node<Facet>* trigger_on_change_only = nullptr
                  , Node<Facet>* enabled = nullptr)
            : RootBase(identifier, parent, CLASS_NAME)
              , m_osc_address(add_socket(OSC_ADDRESS, osc_address))
              , m_input(add_socket(INPUT, input))
              , m_trigger(add_socket(ParameterKeys::TRIGGER, trigger))
              , m_flatten(add_socket(FLATTEN, flatten))
              , m_trigger_on_change_only(add_socket(CHANGE, trigger_on_change_only))
              , m_enabled(add_socket(ParameterKeys::ENABLED, enabled)) {}


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    void process() override {
        if (!is_connected() ||
            !m_time_gate.pop_time() ||
            !m_enabled.process(1).first_or(true) ||
            m_trigger.process().is_empty_like())
            return;

        if (!m_input.has_changed() && m_trigger_on_change_only.process().first_or(false))
            return;

        auto address = m_osc_address.process().first();

        if (!address || address->empty())
            return;

        auto input = m_input.process();
        auto flatten = m_flatten.process().first_or(false);

        m_sender->send(*address, input, flatten);
    }


    bool is_connected() {
        return static_cast<bool>(m_sender);
    }


    bool set_target(const OscTarget& target) {
        try {
            m_sender = std::make_unique<OscSender>(target.ip, target.port);
            return true;
        } catch (IOError&) {
            // TODO: Logging / passing error message to user
            return false;
        }
    }


    void set_input(Node<Facet>* input) { m_input = input; }


    void set_trigger(Node<Trigger>* trigger) { m_trigger = trigger; }


    void set_flatten(Node<Facet>* flatten) { m_flatten = flatten; }


    void set_trigger_on_change_only(Node<Facet>* trigger_on_change_only) {
        m_trigger_on_change_only = trigger_on_change_only;
    }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    Socket<Facet>& get_input() const { return m_input; }


    Socket<Trigger>& get_trigger() const { return m_trigger; }


    Socket<Facet>& get_flatten() const { return m_flatten; }


    Socket<Facet>& get_trigger_on_change_only() const { return m_trigger_on_change_only; }


    Socket<Facet>& get_enabled() const { return m_enabled; }


private:
    std::unique_ptr<OscSender> m_sender = nullptr;

    Socket<std::string>& m_osc_address;
    Socket<Facet>& m_input;
    Socket<Trigger>& m_trigger;


    Socket<Facet>& m_flatten;
    Socket<Facet>& m_trigger_on_change_only;

    Socket<Facet>& m_enabled;

    TimeGate m_time_gate;


};

#endif //SERIALISTLOOPER_OSC_SENDER_H
