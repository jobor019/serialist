
#ifndef SERIALISTLOOPER_INTERPOLATION_ADAPTER_H
#define SERIALISTLOOPER_INTERPOLATION_ADAPTER_H

#include "generative.h"
#include "socket_policy.h"

class InterpolationAdapter : public Node<InterpolationStrategy> {

    static const inline std::string TYPE = "type";
    static const inline std::string PIVOT = "pivot";

    static const inline std::string CLASS_NAME = "interpolation_adapter";

public :
    InterpolationAdapter(const std::string& identifier
                         , ParameterHandler& parent
                         , Node<InterpolationStrategy::Type>* type = nullptr
                         , Node<float>* pivot = nullptr)
            : m_parameter_handler(identifier, parent)
              , m_socket_handler("", m_parameter_handler, ParameterKeys::GENERATIVE_SOCKETS_TREE)
              , m_type(TYPE, m_socket_handler, type)
              , m_pivot(PIVOT, m_socket_handler, pivot) {}


    std::vector<Generative*> get_connected() override {
        return collect_connected(m_type.get_connected(), m_pivot.get_connected());
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    void disconnect_if(Generative& connected_to) override {
        m_type.disconnect_if(connected_to);
        m_pivot.disconnect_if(connected_to);
    }


    std::vector<InterpolationStrategy> process(const TimePoint& t) override {
        auto default_strategy = InterpolationStrategy::default_strategy();
        auto strategy_type = m_type.process_or(t, default_strategy.get_type());
        auto strategy_pivot = m_pivot.process_or(t, default_strategy.get_pivot());
        return {InterpolationStrategy(strategy_type, strategy_pivot)};
    }


    void set_type(Node<InterpolationStrategy::Type>* type) { m_type = type; }


    void set_pivot(Node<float>* pivot) { m_pivot = pivot; }


    Socket<InterpolationStrategy::Type>& get_type() { return m_type; }


    Socket<float>& get_pivot() { return m_pivot; }


private:
    ParameterHandler m_parameter_handler;
    ParameterHandler m_socket_handler;

    Socket<InterpolationStrategy::Type> m_type;
    Socket<float> m_pivot;

};

#endif //SERIALISTLOOPER_INTERPOLATION_ADAPTER_H
