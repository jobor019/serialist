

#ifndef SERIALISTLOOPER_SEQUENCE_H
#define SERIALISTLOOPER_SEQUENCE_H

#include "parameter_policy.h"
#include "socket_policy.h"
#include "generative.h"
#include "interpolator.h"

template<typename T>
class Sequence : public DataNode<T> {
public:

    inline static const std::string SEQUENCE_TREE = "SEQUENCE";
    inline static const std::string ENABLED = "enabled";
    inline static const std::string CLASS_NAME = "sequence";


    explicit Sequence(const std::string& id
                      , ParameterHandler& parent
                      , const std::vector<T>& initial_values = {}
                      , Node<bool>* enabled = nullptr)
            : m_parameter_handler(id, parent)
            , m_socket_handler(ParameterKeys::GENERATIVE_SOCKETS_TREE, m_parameter_handler)
              , m_sequence(SEQUENCE_TREE, m_parameter_handler, initial_values)
              , m_enabled(ENABLED, m_socket_handler, enabled) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
    }


    std::vector<T> process(const TimePoint&, double y, InterpolationStrategy strategy) override {
        return m_sequence.interpolate(y, std::move(strategy));
    }

    void disconnect_if(Generative&) override {}


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    ParametrizedSequence<T>& get_parameter_obj() {
        return m_sequence;
    }


    std::vector<Generative*> get_connected() override {
        return {m_enabled.get_connected()};
    }


    void set_enabled(Node<bool>* enabled) { m_enabled = enabled; }


    Socket<bool>& get_enabled() { return m_enabled; }


private:
    ParameterHandler m_parameter_handler;
    ParameterHandler m_socket_handler;

    ParametrizedSequence<T> m_sequence;

    Socket<bool> m_enabled;

};


#endif //SERIALISTLOOPER_SEQUENCE_H
