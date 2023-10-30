
#ifndef SERIALISTLOOPER_INTERPOLATION_ADAPTER_RM_H
#define SERIALISTLOOPER_INTERPOLATION_ADAPTER_RM_H

#include "core/generative.h"
#include "core/param/socket_policy.h"

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
              , m_socket_handler(m_parameter_handler)
              , m_type(m_socket_handler.create_socket(TYPE, type))
              , m_pivot(m_socket_handler.create_socket(PIVOT, pivot)) {}


    std::vector<Generative*> get_connected() override {
        return m_socket_handler.get_connected();
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    void disconnect_if(Generative& connected_to) override {
        m_socket_handler.disconnect_if(connected_to);
    }

    Voices<InterpolationStrategy> process() override {
        // TODO: Can be optimized to avoid unnecessary computations

        auto default_strategy = InterpolationStrategy::default_strategy();
        if (!m_type.is_connected() && !m_pivot.is_connected()) {
            return Voices<InterpolationStrategy>(default_strategy);
        }

        auto strategy_type = m_type.process();
        auto strategy_pivot = m_pivot.process();

        if (strategy_type.size() > strategy_pivot.size()) {
            strategy_pivot = strategy_pivot.adapted_to(strategy_type.size());
        } else {
            strategy_type = strategy_type.adapted_to(strategy_pivot.size());
        }

        auto pivots = strategy_pivot.firsts_or(default_strategy.get_pivot());
        auto types = strategy_pivot.firsts_or(default_strategy.get_type());

        std::vector<InterpolationStrategy> output;
        output.reserve(pivots.size());

        for (std::size_t i = 0; i < pivots.size(); ++i) {
            output.emplace_back(types.at(i), pivots.at(i));
        }

        return Voices<InterpolationStrategy>(output);
    }


    void set_type(Node<Facet>* type) { m_type = type; }


    void set_pivot(Node<Facet>* pivot) { m_pivot = pivot; }


    Socket<Facet>& get_type() { return m_type; }


    Socket<Facet>& get_pivot() { return m_pivot; }


private:
    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

    Socket<Facet>& m_type;
    Socket<Facet>& m_pivot;

};

#endif //SERIALISTLOOPER_INTERPOLATION_ADAPTER_RM_H
