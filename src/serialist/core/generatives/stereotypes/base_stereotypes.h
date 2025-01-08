
#ifndef SERIALISTLOOPER_BASE_STEREOTYPES_H
#define SERIALISTLOOPER_BASE_STEREOTYPES_H

#include "core/generative.h"
#include "core/param/socket_handler.h"
#include "core/param/parameter_keys.h"
#include "core/temporal/time_gate.h"
#include "core/collections/voices.h"
#include "core/algo/facet.h"
#include "core/temporal/time_point.h"
#include "core/temporal/trigger.h"
#include "collections/multi_voiced.h"

namespace serialist {

class GenerativeCommons {
public:
    GenerativeCommons() = delete;


    template<std::size_t max_count = 128, typename... Args>
    static std::size_t voice_count(Socket<Facet>& voices_count_socket, Args... args) {
        auto num_voices = static_cast<long>(voices_count_socket.process().adapted_to(1).first_or(0));
        if (num_voices <= 0) {
            return std::min(max_count, std::max({static_cast<std::size_t>(1), args...}));
        }

        return std::min(max_count, static_cast<std::size_t>(num_voices));
    }
};


// ==============================================================================================

// TODO: Lots of code duplication from StaticNode. Not sure if this can be handled better
class RootBase : public Root {
public:
    RootBase(const std::string& id
             , ParameterHandler& parent
             , const std::string& class_name)
            : m_parameter_handler(Specification(param::types::generative)
                                          .with_identifier(id)
                                          .with_static_property(param::properties::template_class, class_name)
                                  , parent)
              , m_socket_handler(m_parameter_handler) {
    }


    std::vector<Generative*> get_connected() override { return m_socket_handler.get_connected(); }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


    void disconnect_if(Generative& connected_to) override { m_socket_handler.disconnect_if(connected_to); }


protected:
    template<typename OutputType>
    Socket<OutputType>& add_socket(const std::string& id, Node<OutputType>* initial = nullptr) {
        return m_socket_handler.create_socket(id, initial);
    }


private:
    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

};


// ==============================================================================================

template<typename T>
class StaticNode : public Node<T> {
public:
    StaticNode(const std::string& id
               , ParameterHandler& parent
               , const std::string& class_name)
            : m_parameter_handler(Specification(param::types::generative)
            .with_identifier(id)
            .with_static_property(param::properties::template_class, class_name)
            , parent)
              , m_socket_handler(m_parameter_handler) {}


    std::vector<Generative*> get_connected() override { return m_socket_handler.get_connected(); }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


    void disconnect_if(Generative& connected_to) override { m_socket_handler.disconnect_if(connected_to); }


protected:
    template<typename OutputType>
    Socket<OutputType>& add_socket(const std::string& id, Node<OutputType>* initial = nullptr) {
        return m_socket_handler.create_socket(id, initial);
    }


private:
    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

};


// ==============================================================================================

template<typename T>
class NodeBase : public StaticNode<T> {
public:
    NodeBase(const std::string& id
             , ParameterHandler& parent
             , Node<Facet>* enabled
             , Node<Facet>* num_voices
             , const std::string& class_name)
            : StaticNode<T>(id, parent, class_name)
              , m_enabled(StaticNode<T>::add_socket(param::properties::enabled, enabled))
              , m_num_voices(StaticNode<T>::add_socket(param::properties::num_voices, num_voices)) {}


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<Facet>& get_enabled() { return m_enabled; }


    Socket<Facet>& get_num_voices() { return m_num_voices; }


protected:
    std::optional<TimePoint> pop_time() { return m_time_gate.pop_time(); }

    bool is_enabled() {
        return m_enabled.process().first_or(true);
    }

    bool is_enabled(const TimePoint& t) {
        return t.get_transport_running() && is_enabled();
    }


    template<std::size_t max_count = 128, typename... Args>
    std::size_t voice_count(Args... args) {
        return GenerativeCommons::voice_count(m_num_voices, args...);
    }

    template<typename InputType, typename OutputType>
    static Vec<OutputType> adapted(Voices<InputType>&& values
                                   , std::size_t num_voices
                                   , const OutputType& default_value) {
        return values.adapted_to(num_voices).firsts_or(default_value);
    }


private:
    Socket<Facet>& m_enabled;
    Socket<Facet>& m_num_voices;

    TimeGate m_time_gate;
};


// ==============================================================================================

template<typename T>
class PulsatorBase : public NodeBase<Trigger> {
public:

    PulsatorBase(const std::string& id
                 , ParameterHandler& parent
                 , Node<Facet>* enabled
                 , Node<Facet>* num_voices
                 , const std::string& class_name)
            : NodeBase<Trigger>(id, parent, enabled, num_voices, class_name) {
        static_assert(std::is_base_of_v<Flushable<Trigger>, T>);
    }


    Voices<Trigger> process() final {
        auto t = pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        bool enabled = NodeBase<Trigger>::is_enabled(*t);
        auto enabled_state = m_enabled_gate.update(enabled);
        if (auto flushed = handle_enabled_state(enabled_state)) {
            m_current_value = *flushed;
        }

        if (!enabled)
            return m_current_value;

        auto num_voices = get_voice_count();
        Voices<Trigger> output = Voices<Trigger>::zeros(num_voices);

        bool resized = false;
        if (auto flushed = update_size(num_voices)) {
            if (!flushed->is_empty_like()) {
                // from this point on, size of output may be different from num_voices,
                //   but this is the only point where resizing should be allowed
                output.merge_uneven(*flushed, true);
            }
            resized = true;
        }

        auto transport_events = m_time_event_gate.poll(*t);
        if (auto flushed = handle_transport_events(*t, transport_events)) {
            output.merge_uneven(*flushed, false);
        }

        update_parameters(num_voices, resized);

        auto pulsator_output = process_pulsator(*t, num_voices);
        output.merge_uneven(pulsator_output, false);

        m_current_value = std::move(output);
        return m_current_value;
    }

protected:
    virtual std::size_t get_voice_count() = 0;

    /** @return new `m_current_value` on significant state change, otherwise std::nullopt */
    virtual std::optional<Voices<Trigger>> handle_enabled_state(EnabledState state) = 0;

    virtual void update_parameters(std::size_t num_voices, bool size_has_changed) = 0;

    virtual std::optional<Voices<Trigger>>
    handle_transport_events(const TimePoint& t, const Vec<TimeEvent>& events) = 0;

    virtual Voices<Trigger> process_pulsator(const TimePoint& t, std::size_t num_voices) = 0;


    /**
     * @return flushed triggers (Voices<Trigger>) if num_voices has changed, std::nullopt otherwise
     *         note that the flushed triggers will have the same size as the previous num_voices, hence
     *         merge_uneven(.., true) is required
     */
    virtual std::optional<Voices<Trigger>> update_size(std::size_t num_voices) {
        if (num_voices != m_pulsators.size()) {
            auto flushed = m_pulsators.resize(num_voices);
            return flushed;
        }

        return std::nullopt;
    }


    MultiVoiced<T, Trigger>& pulsators() { return m_pulsators; }

private:
    MultiVoiced<T, Trigger> m_pulsators;

    Voices<Trigger> m_current_value = Voices<Trigger>::empty_like();
    EnabledGate m_enabled_gate;
    TimeEventGate m_time_event_gate;
};

} // namespace serialist


#endif //SERIALISTLOOPER_BASE_STEREOTYPES_H
