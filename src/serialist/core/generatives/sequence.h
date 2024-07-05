

#ifndef SERIALISTLOOPER_SEQUENCE_H
#define SERIALISTLOOPER_SEQUENCE_H

#include "core/policies.h"
#include "core/generative.h"
#include "core/param/parameter_keys.h"
#include "core/collections/vec.h"
#include "core/collections/voices.h"

namespace serialist {

template<typename OutputType, typename StoredType = OutputType>
class Sequence : public Node<OutputType> {
public:

    inline static const std::string SEQUENCE_TREE = "SEQUENCE";
    inline static const std::string CLASS_NAME = "sequence";


    Sequence(const std::string& id
                      , ParameterHandler& parent
                      , const Voices<StoredType>& initial_values = Voices<StoredType>::empty_like())
            : m_parameter_handler(id, parent)
              , m_sequence(SEQUENCE_TREE, m_parameter_handler, initial_values) {
        static_assert(std::is_constructible_v<OutputType, StoredType>
                      && std::is_constructible_v<StoredType, OutputType>
                      , "Cannot create a Sequence with incompatible types");

        m_parameter_handler.add_static_property(ParameterTypes::GENERATIVE_CLASS, CLASS_NAME);
    }

    Sequence(const std::string& id, ParameterHandler& parent, const StoredType& value)
    : Sequence(id, parent, Voices<StoredType>::singular(value)) {}


    Voices<OutputType> process() override {
        return m_sequence.get_voices();
    }


    std::vector<Generative*> get_connected() override {
        return {};
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    Voices<OutputType> get_values() {
        return m_sequence.get_voices();
    }

    Voices<StoredType> get_values_raw() {
        return m_sequence.get_voices_raw();
    }

    void set_values(const StoredType& value) {
        m_sequence.set(Voices<StoredType>::singular(value));
    }

    void set_values(const Voices<StoredType>& values) {
        m_sequence.set(values);
    }

    void set_value_at(std::size_t index, const Voice<StoredType>& value) {
        m_sequence.set_at(index, value);
    }



    SequenceParameter<OutputType, StoredType>& get_parameter_obj() {
        return m_sequence;
    }


private:
    ParameterHandler m_parameter_handler;

    SequenceParameter<OutputType, StoredType> m_sequence;

};


} // namespace serialist

#endif //SERIALISTLOOPER_SEQUENCE_H
