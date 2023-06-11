

#ifndef SERIALISTLOOPER_SEQUENCE_H
#define SERIALISTLOOPER_SEQUENCE_H

#include "parameter_policy.h"
#include "socket_policy.h"
#include "generative.h"
#include "interpolator.h"

template<typename T>
class Sequence : public DataNode<T> {
public:

    inline static const std::string PARAMETER_ADDRESS = "sequence";


    explicit Sequence(const std::string& id
                      , ParameterHandler& parent
                      , const std::vector<T>& initial_values = {}
                      , Node<bool>* enabled = nullptr)
            : DataNode<T>(id, parent)
              , m_sequence(PARAMETER_ADDRESS, *this, initial_values)
              , m_enabled("enabled", *this, enabled) {}


    std::vector<T> process(const TimePoint&, double y, InterpolationStrategy<T> strategy) override {
        return m_sequence.interpolate(y, std::move(strategy));
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
    ParametrizedSequence<T> m_sequence;

    Socket<bool> m_enabled;

};


#endif //SERIALISTLOOPER_SEQUENCE_H
