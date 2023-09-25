

#ifndef SERIALISTLOOPER_SEQUENCE_H
#define SERIALISTLOOPER_SEQUENCE_H

#include "parameter_policy.h"
#include "socket_policy.h"
#include "generative.h"
#include "parameter_keys.h"

template<typename OutputType, typename StoredType = OutputType>
class Sequence : public Node<OutputType> {
public:

    inline static const std::string SEQUENCE_TREE = "SEQUENCE";
    inline static const std::string CLASS_NAME = "sequence";


    explicit Sequence(const std::string& id
                      , ParameterHandler& parent
                      , const std::vector<std::vector<StoredType>>& initial_values = {})
            : m_parameter_handler(id, parent)
              , m_sequence(SEQUENCE_TREE, m_parameter_handler, initial_values) {
        static_assert(std::is_constructible_v<OutputType, StoredType>
                      && std::is_constructible_v<StoredType, OutputType>
                      , "Cannot create a Sequence with incompatible types");

        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
    }


    explicit Sequence(const std::string& id, ParameterHandler& parent, const std::vector<StoredType>& initial_values)
            : Sequence(id, parent, std::vector<std::vector<StoredType>>{initial_values}) {}


    explicit Sequence(const std::string& id, ParameterHandler& parent, const StoredType& initial_values)
            : Sequence(id, parent, std::vector<std::vector<StoredType>>{{initial_values}}) {}


    Voices<OutputType> process() override {
        return m_sequence.get_voices();
    }


    std::vector<Generative*> get_connected() override {
        return {};
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    void disconnect_if(Generative&) override { /* unused */ }


    std::vector<std::vector<StoredType>> get_values() {
        return m_sequence.get();
    }


    void set_values(const std::vector<std::vector<StoredType>>& values) {
        m_sequence.set(values);
    }



    void set_transposed(const std::vector<StoredType>& values) {
        m_sequence.set(VoiceUtils::transpose(values));
    }


    SequenceParameter<OutputType, StoredType>& get_parameter_obj() {
        return m_sequence;
    }


private:
    ParameterHandler m_parameter_handler;

    SequenceParameter<OutputType, StoredType> m_sequence;

};


#endif //SERIALISTLOOPER_SEQUENCE_H
