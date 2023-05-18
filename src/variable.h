

#ifndef SERIALIST_LOOPER_VARIABLE_H
#define SERIALIST_LOOPER_VARIABLE_H


#include "generative.h"
#include "parameter_policy.h"

template<typename T>
class Variable : public Node<T>
                 , public ParameterHandler {
public:

    inline static const std::string PARAMETER_ADDRESS = "value";


    explicit Variable(T value, const std::string& id, VTParameterHandler& parent)
            : ParameterHandler(id, parent), m_value(value, PARAMETER_ADDRESS, *this) {}


    std::vector<T> process(const TimePoint&) override { return {m_value.get()}; }


    T get_value() const { return m_value.get(); }


    void set_value(T value) { m_value.set(value); }


    AtomicParameter<T>& get_parameter_obj() {
        return m_value;
    }


    std::vector<Generative*> get_connected() override {
        return {};
    }


private:

    AtomicParameter<T> m_value;

};

#endif //SERIALIST_LOOPER_VARIABLE_H
