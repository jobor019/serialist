

#ifndef SERIALISTLOOPER_SEQUENCE_H
#define SERIALISTLOOPER_SEQUENCE_H

#include "parameter_policy.h"
#include "socket_policy.h"
#include "generative.h"
#include "interpolator.h"
#include "parameter_keys.h"

template<typename OutputType, typename StoredType = OutputType>
class Sequence : public DataNode<OutputType> {
public:

    inline static const std::string SEQUENCE_TREE = "SEQUENCE";
    inline static const std::string ENABLED = "enabled";
    inline static const std::string CLASS_NAME = "sequence";


    explicit Sequence(const std::string& id
                      , ParameterHandler& parent
                      , const std::vector<StoredType>& initial_values = {}
                      , Node<Facet>* enabled = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler(ParameterKeys::GENERATIVE_SOCKETS_TREE, m_parameter_handler)
              , m_sequence(SEQUENCE_TREE, m_parameter_handler, initial_values)
              , m_enabled(ENABLED, m_socket_handler, enabled) {
        static_assert(std::is_constructible_v<OutputType, StoredType>
                      && std::is_constructible_v<StoredType, OutputType>
                      , "Cannot create a Sequence with incompatible types");

        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
    }


    std::vector<OutputType> process(const TimePoint&, double y, InterpolationStrategy strategy) override {
        auto values = m_sequence.interpolate(y, std::move(strategy));
        if constexpr (std::is_same_v<OutputType, StoredType>) {
            return values;

        } else {
            std::vector<OutputType> output;
            output.reserve(values.size());
            std::transform(values.begin(), values.end()
                           , std::back_inserter(output)
                           , [](const StoredType& element) { return static_cast<OutputType>(element); }
            );
            return output;
        }
    }


    void disconnect_if(Generative&) override {}


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    ParametrizedSequence<StoredType>& get_parameter_obj() {
        return m_sequence;
    }


    std::vector<Generative*> get_connected() override {
        return {m_enabled.get_connected()};
    }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    Socket<Facet>& get_enabled() { return m_enabled; }


private:
    ParameterHandler m_parameter_handler;
    ParameterHandler m_socket_handler;

    ParametrizedSequence<StoredType> m_sequence;

    Socket<Facet> m_enabled;

};


#endif //SERIALISTLOOPER_SEQUENCE_H
