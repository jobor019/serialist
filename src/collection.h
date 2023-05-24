

#ifndef SERIALISTLOOPER_COLLECTION_H
#define SERIALISTLOOPER_COLLECTION_H


#include "parameter_policy.h"
#include "generative.h"

template<typename T>
class Collection : public Node<T>
                   , public ParameterHandler {
public:

    inline static const std::string PARAMETER_ADDRESS = "collection";


    explicit Collection(const std::string& id, VTParameterHandler& parent, const std::vector<T>& initial_values = {})
            : ParameterHandler(id, parent), m_collection(initial_values, PARAMETER_ADDRESS, *this) {}


    std::vector<T> process(const TimePoint&, double y, const InterpolationStrategy<T> strategy) {
        return m_collection.clone_values();
    }


    ParametrizedCollection<T>& get_parameter_obj() {
        return m_collection;
    }


    std::vector<Generative*> get_connected() override {
        return {};
    }


private:
    ParametrizedCollection<T> m_collection;

};

#endif //SERIALISTLOOPER_COLLECTION_H
