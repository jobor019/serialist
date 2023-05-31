

#ifndef SERIALISTLOOPER_GENERATOR_H
#define SERIALISTLOOPER_GENERATOR_H

#include "generative.h"
#include "parameter_policy.h"
#include "sequence.h"
#include "socket_policy.h"

template<typename T>
class Generator : public Node<T> {
public:
    Generator(const std::string& id
              , ParameterHandler& parent
              , Node<double>* cursor = nullptr
              , Node<InterpolationStrategy<T>>* interp = nullptr
              , Sequence<T>* map = nullptr)
            : Node<T>(id, parent)
              , m_cursor("cursor", *this, cursor)
              , m_interpolation_strategy("interp", *this, interp)
              , m_map("map", *this, map) {}


    std::vector<T> process(const TimePoint& t) override {
        if (!m_cursor.is_connected())
            return {};

        auto y = m_cursor.process_or(t, 0.0);

        if (!m_interpolation_strategy.is_connected() || !m_map.is_connected()) {
            if constexpr (std::is_arithmetic_v<T>) {
                return {static_cast<T>(y)};
            } else {
                return {};
            }
        }

        auto strategy = m_interpolation_strategy.process(t);

        if (strategy.empty())
            return {};


        return m_map.process(t, y, strategy.at(0));
    }


    std::vector<Generative*> get_connected() override {
        return Generative::collect_connected(m_cursor.get_connected()
                                             , m_interpolation_strategy.get_connected()
                                             , m_map.get_connected());
    }


    void set_cursor(Node<double>* cursor) { m_cursor = cursor; }


    void set_interpolation_strategy(Node<InterpolationStrategy<T>>* interpolation_strategy) {
        m_interpolation_strategy = interpolation_strategy;
    }


    void set_map(Sequence<T>* map) { m_map = map; }


private:
    Socket<double> m_cursor;
    Socket<InterpolationStrategy<T>> m_interpolation_strategy;
    DataSocket<T> m_map;

};


#endif //SERIALISTLOOPER_GENERATOR_H
