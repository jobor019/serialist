

#ifndef SERIALIST_LOOPER_VARIABLE_H
#define SERIALIST_LOOPER_VARIABLE_H


#include "core/generative.h"
#include "serialist/core/policies/policies.h"
#include "core/param/parameter_keys.h"

namespace serialist {

template<typename OutputType, typename StoredType = OutputType>
class Variable : public Node<OutputType> {
public:
    using ParameterType = typename std::conditional_t<utils::is_always_lock_free_v<StoredType>
                                                    , AtomicParameter<StoredType>
                                                    , ComplexParameter<StoredType>>;

    inline static const std::string PARAMETER_ADDRESS = "value";
    inline static const std::string CLASS_NAME = "variable";

    explicit Variable(const std::string& id, ParameterHandler& parent, StoredType value)
            : m_parameter_handler(Generative::specification(id, CLASS_NAME), parent)
              , m_value(value, PARAMETER_ADDRESS, m_parameter_handler) {

        static_assert((std::is_constructible_v<OutputType, StoredType>
                       && std::is_constructible_v<StoredType, OutputType>)
                      || std::is_enum_v<StoredType>
                      , "Cannot create a Variable with incompatible types");
    }


    Voices<OutputType> process() override {
        return Voices<OutputType>::singular(static_cast<OutputType>(m_value.get()));
    }


    void disconnect_if(Generative&) override { /* unused */ }


    StoredType get_value() { return m_value.get(); }


    void set_value(StoredType value) { m_value.set(value); }


    ParameterType& get_parameter_obj() { return m_value; }


    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }


    std::vector<Generative*> get_connected() override { return {}; }


private:

    ParameterHandler m_parameter_handler;

    ParameterType m_value;

};

} // namespace serialist

#endif //SERIALIST_LOOPER_VARIABLE_H
