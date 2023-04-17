//
//
//#ifndef SERIALIST_LOOPER_LOOPER_H
//#define SERIALIST_LOOPER_LOOPER_H
//
//#include "phasor.h"
//#include "mapping.h"
//#include "graph_node.h"
//#include "interpolator.h"
//
//template<typename T>
//class Looper : public GraphNode<T> {
//public:
//    explicit Looper() = default;
//
//    explicit Looper(Mapping<T> mapping
//                    , double step_size = 1.0
//                    , double phase = 0.0
//                    , Phasor::Mode mode = Phasor::Mode::stepped
//                    , std::unique_ptr<Accessor<long>> accessor = std::make_unique<FirstAccessor<long>>()
//                    , std::unique_ptr<GraphNode<T>> add = nullptr
//                    , std::unique_ptr<GraphNode<T>> mul = nullptr)
//            : m_mapping(mapping)
//              , m_phasor{step_size, static_cast<double>(mapping.size()), phase, mode}
//              , m_accessor(std::move(accessor))
//              , m_add{std::move(add)}
//              , m_mul{std::move(mul)} {}
//
//
//    std::vector<T> process(const TimePoint& time) override {
//        if (m_mapping.empty()) {
//            return {};
//        }
//
//        double x = m_phasor.process(time.get_tick());
//        auto elements = m_accessor->get(static_cast<long>(std::floor(x)), m_mapping);
//
//        if constexpr (std::is_arithmetic_v<T>) {
//            T add = static_cast<T>(0);
//            T mul = static_cast<T>(1);
//
//            if (m_add)
//                add = m_add->process(time);
//            if (m_mul)
//                mul = m_mul->process(time);
//
//            for (auto& element: elements) {
//                element = element * mul + add;
//            }
//        }
//
//        return elements;
//    }
//
//
//    void set_mapping(Mapping<T> mapping) {
//        m_mapping = mapping;
//        m_phasor.set_max(static_cast<double>(mapping.size()));
//    }
//
//
//    void add_element(T element, long index = -1) {
//        m_mapping.add(std::move(element), index);
//
//        // Increment phasor range by number of elements inserted
//        m_phasor.set_max(m_phasor.get_max() + 1.0);
//
//        long insertion_point = index;
//        if (index < 0) {
//            insertion_point += m_mapping.size();
//        }
//
//        // If inserting before current value, increment to avoid risk of repeating the same value twice
//        auto current_phase = m_phasor.get_current_value();
//        if (static_cast<double>(insertion_point) < current_phase) {
//            current_phase += 1.0;
//            m_phasor.set_phase(current_phase, false);
//        }
//    }
//
//
//    void set_step_size(double step_size) {
//        m_phasor.set_step_size(step_size);
//    }
//
//
//    void set_phase(double phase) {
//        m_phasor.set_phase(phase, true);
//    }
//
//
//    void set_mode(Phasor::Mode mode) {
//        m_phasor.set_mode(mode);
//    }
//
//
//private:
//    Mapping<T> m_mapping;
//    Phasor m_phasor;
//    std::unique_ptr<Accessor<T>> m_accessor;
//
//
//    std::unique_ptr<GraphNode<T>> m_add;
//    std::unique_ptr<GraphNode<T>> m_mul;
//
//};
//
//#endif //SERIALIST_LOOPER_LOOPER_H
