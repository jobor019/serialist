
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
                         , Node<Facet>* type = nullptr
                         , Node<Facet>* pivot = nullptr)
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
        auto strategy_type = static_cast<InterpolationStrategy::Type>(m_type.process_or(t, Facet(default_strategy.get_type())));
        auto strategy_pivot = m_pivot.process_or(t, Facet(default_strategy.get_pivot()));
        return {InterpolationStrategy(static_cast<InterpolationStrategy::Type>(strategy_type)
                                      , static_cast<float>(strategy_pivot.get()))};
    }


    void set_type(Node<Facet>* type) { m_type = type; }


    void set_pivot(Node<Facet>* pivot) { m_pivot = pivot; }


    Socket<Facet>& get_type() { return m_type; }


    Socket<Facet>& get_pivot() { return m_pivot; }


private:
    ParameterHandler m_parameter_handler;
    ParameterHandler m_socket_handler;

    Socket<Facet> m_type;
    Socket<Facet> m_pivot;

};

#endif //SERIALISTLOOPER_INTERPOLATION_ADAPTER_H
