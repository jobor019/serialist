

#ifndef SERIALISTLOOPER_GENERATOR_H
#define SERIALISTLOOPER_GENERATOR_H

#include "generative.h"
#include "parameter_policy.h"
#include "sequence.h"
#include "vt_parameter.h"

template<typename T>
class Generator : public Node<T>
                  , public ParameterHandler {
public:
    Generator(const std::string& id
              , VTParameterHandler& parent
              , Node<double>* cursor = nullptr
              , Sequence<T>* map = nullptr)
            : ParameterHandler(id, parent), m_cursor(cursor), m_map(map) {}


    std::vector<T> process(const TimePoint& t) override {
        if (!m_cursor)
            return {};

        auto y = Node<T>::value_or(m_cursor, 0.0, t);

        if (!m_interpolation_strategy || !m_map) {
            if constexpr (std::is_arithmetic_v<T>) {
                return {static_cast<T>(y)};
            } else {
                return {};
            }
        }

        auto strategy = m_interpolation_strategy->process(t);

        if (strategy.empty())
            return {};


        return m_map->process(t, y, strategy.at(0));
    }


    std::vector<Generative*> get_connected() override {
        return Generative::collect_connected(m_cursor, m_interpolation_strategy, m_map);
    }


    void set_cursor(Node<double>* cursor) { m_cursor = cursor; }


    void set_interpolation_strategy(Node<InterpolationStrategy<T>>* interpolation_strategy) {
        m_interpolation_strategy = interpolation_strategy;
    }


    void set_map(Sequence<T>* map) { m_map = map; }


private:
    Node<double>* m_cursor;
    Node<InterpolationStrategy<T>>* m_interpolation_strategy;
    Sequence<T>* m_map;

};


#endif //SERIALISTLOOPER_GENERATOR_H
