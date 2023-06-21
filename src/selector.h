

#ifndef SERIALIST_LOOPER_SELECTOR_H
#define SERIALIST_LOOPER_SELECTOR_H

#include <vector>
#include <cmath>

#include "parameter_policy.h"
#include "generative.h"
#include "transport.h"


template<typename T>
class Selector : public Node<T> {
public:
    enum class Type {
        all
        , nth
        , random
        , probabilistic
    };


    std::vector<T> process(const TimePoint& t) override {
        auto material = m_selection_material->process(t);
        auto mask = m_selection_mask->process(t);
        auto select_from_end = m_select_from_end->process(t);
        auto type = m_type->process(t);

        switch (type) {
            case Type::all:
                return all(material, mask, select_from_end);
            case Type::nth:
                return nth(material, mask, select_from_end);
            case Type::random:
                return random(material, mask, select_from_end);
            case Type::probabilistic:
                return proba(material, mask, select_from_end);
            default:
                throw std::runtime_error("Invalid selector type detected");
        }

    }


    std::vector<Generative*> get_connected() override {
        return Generative::collect_connected(m_type, m_selection_mask, m_select_from_end, m_selection_material);
    }


private:
    std::vector<T> all(const std::vector<T>& material, const std::vector<double>&, bool) {
        return material;
    }


    std::vector<T> nth(const std::vector<T>& material, const std::vector<double>& mask, bool select_from_end) {
        // Select any element enabled in mask (mask > 0.5),
        // handling boundaries correctly. negative indices if select_from_end
        (void) material;(void) mask;(void) select_from_end;
        throw std::runtime_error("not implemented"); // TODO
    }


    std::vector<T> random(const std::vector<T>& material, const std::vector<double>& mask) {
        (void) material;(void) mask;
        // random select N elements (N = mask.size())
        throw std::runtime_error("not implemented"); // TODO
    }


    std::vector<T> proba(const std::vector<T>& material, const std::vector<double>& mask, bool select_from_end) {
        (void) material;(void) mask;(void) select_from_end;
        // weighted random, i.e. a probability for each element by index, selecting [0..N] elements
        throw std::runtime_error("not implemented"); // TODO
    }


    Node<Type>* m_type; // TODO: Change into Parameter
    Node<std::vector<double>>* m_selection_mask; // probabilities in `probabilistic`, binary in `nth`
    Node<bool>* m_select_from_end;

    Node<T>* m_selection_material;

};


#endif //SERIALIST_LOOPER_SELECTOR_H
