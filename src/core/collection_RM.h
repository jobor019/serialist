//
//
//#ifndef SERIALISTLOOPER_COLLECTION_H
//#define SERIALISTLOOPER_COLLECTION_H
//
//
//#include "parameter_policy.h"
//#include "generative.h"
//#include "parameter_keys.h"
//
//template<typename T>
//class Collection : public Node<T> {
//public:
//
//    inline static const std::string PARAMETER_ADDRESS = "collection";
//    inline static const std::string CLASS_NAME = "collection";
//
//
//    explicit Collection(const std::string& id, ParameterHandler& parent, const std::vector<T>& initial_values = {})
//            : parameter_handler(id, parent)
//              , m_collection(PARAMETER_ADDRESS, parameter_handler, initial_values) {
//        parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
//    }
//
//
//    void disconnect_if(Generative&) override{}
//
//    std::vector<T> process(const TimePoint&, double y, const InterpolationStrategy strategy) {
//        return m_collection.clone_values();
//    }
//
//
//    ParametrizedCollection<T>& get_parameter_obj() {
//        return m_collection;
//    }
//
//
//    std::vector<Generative*> get_connected() override {
//        return {};
//    }
//
//
//private:
//    ParameterHandler parameter_handler;
//    ParametrizedCollection<T> m_collection;
//
//
//};
//
//#endif //SERIALISTLOOPER_COLLECTION_H
