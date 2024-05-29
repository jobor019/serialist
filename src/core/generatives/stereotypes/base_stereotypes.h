
#ifndef SERIALISTLOOPER_BASE_STEREOTYPES_H
#define SERIALISTLOOPER_BASE_STEREOTYPES_H

#include "core/generative.h"
#include "core/param/socket_handler.h"
#include "core/param/parameter_keys.h"
#include "core/algo/temporal/time_gate.h"
#include "core/collections/voices.h"
#include "core/algo/facet.h"
#include "core/algo/temporal/time_point.h"

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
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, class_name);
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
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, class_name);
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
class NodeBase : public StaticNode<T> {
public:
    NodeBase(const std::string& id
             , ParameterHandler& parent
             , Node<Facet>* enabled
             , Node<Facet>* num_voices
             , const std::string& class_name)
            : StaticNode<T>(id, parent, class_name)
              , m_enabled(StaticNode<T>::add_socket(ParameterKeys::ENABLED, enabled))
              , m_num_voices(StaticNode<T>::add_socket(ParameterKeys::NUM_VOICES, num_voices)) {}


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<Facet>& get_enabled() { return m_enabled; }


    Socket<Facet>& get_num_voices() { return m_num_voices; }


protected:
    std::optional<TimePoint> pop_time() { return m_time_gate.pop_time(); }


    bool is_enabled(const TimePoint& t) {
        return t.get_transport_running() && m_enabled.process().first_or(true);
    }


    template<std::size_t max_count = 128, typename... Args>
    std::size_t voice_count(Args... args) {
        return GenerativeCommons::voice_count(m_num_voices, args...);
    }


private:
    Socket<Facet>& m_enabled;
    Socket<Facet>& m_num_voices;

    TimeGate m_time_gate;
};


#endif //SERIALISTLOOPER_BASE_STEREOTYPES_H
