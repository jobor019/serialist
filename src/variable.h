

#ifndef SERIALIST_LOOPER_VARIABLE_H
#define SERIALIST_LOOPER_VARIABLE_H


#include "generative.h"
#include "parameter_policy.h"

template<typename T>
class Variable : public Node<T> {
public:
    template<typename U>
    using ParameterType = typename std::conditional<std::atomic<T>::is_always_lock_free
                                                    , AtomicParameter<U>
                                                    , ComplexParameter<U>>::type;

    inline static const std::string PARAMETER_ADDRESS = "value";


    explicit Variable(const std::string& id, ParameterHandler& parent, T value)
            : m_parameter_handler(id, parent)
              , m_value(value, PARAMETER_ADDRESS, m_parameter_handler) {}


    std::vector<T> process(const TimePoint&) override { return {m_value.get()}; }


    T get_value() { return m_value.get(); }


    void set_value(T value) { m_value.set(value); }


    ParameterType<T>& get_parameter_obj() { return m_value; }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


    std::vector<Generative*> get_connected() override { return {}; }


private:

    ParameterHandler m_parameter_handler;

    ParameterType<T> m_value;

};

#endif //SERIALIST_LOOPER_VARIABLE_H
