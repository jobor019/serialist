

#ifndef SERIALIST_LOOPER_SELECTOR_H
#define SERIALIST_LOOPER_SELECTOR_H

#include <vector>
#include <cmath>

template<typename T>
class Selector {
public:
    virtual ~Selector() = default;

    virtual std::vector<T> get(std::vector<T> elements) = 0;
};


// ==============================================================================================

template<typename T>
class IdentitySelector : public Selector<T> {
public:

    std::vector<T> get(std::vector<T> elements) override {
        return elements;
    }
};


// ==============================================================================================

template<typename T>
class NthSelector : public Selector<T> {
public:

    explicit NthSelector(int nth) : m_nth(nth) {}


    std::vector<T> get(std::vector<T> elements) override {
        // Handle negative indices
        auto sub_element_index = m_nth;
        if (sub_element_index < 0) {
            sub_element_index += elements.size();
        }

        if (sub_element_index < 0 || static_cast<std::size_t>(sub_element_index) >= elements.size()) {
            return {};
        }

        return {elements.at(static_cast<std::size_t>(sub_element_index))};
    }

private:
    int m_nth;
};


// ==============================================================================================

template<typename T>
class FirstSelector : public NthSelector<T> {
public:
    FirstSelector() : NthSelector<T>(0) {}
};


// ==============================================================================================

template<typename T>
class NthsSelector : public Selector<T> {
public:
    explicit NthsSelector(std::vector<int> nths) : m_nths(std::move(nths)) {
        throw std::runtime_error("not implemented yet");
    }

private:
    std::vector<int> m_nths;
};


// ==============================================================================================

template<typename T>
class RandomSelector : public Selector<T> {
public:
    explicit RandomSelector() {
        // TODO: Initialize seed
        throw std::runtime_error("not implemented yet");
    }

private:
    // TODO: Seed
};

#endif //SERIALIST_LOOPER_SELECTOR_H
