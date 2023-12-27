
#include "core/collections/vec.h"
#include "multi_slider_element.h"

#ifndef SERIALISTLOOPER_MULTI_SLIDER_ACTION_H
#define SERIALISTLOOPER_MULTI_SLIDER_ACTION_H

template<typename T>
class MultiSliderAction {

public:
    MultiSliderAction() = default;
    virtual ~MultiSliderAction() = default;
    MultiSliderAction(const MultiSliderAction&) = delete;
    MultiSliderAction& operator=(const MultiSliderAction&) = delete;
    MultiSliderAction(MultiSliderAction&&) noexcept = default;
    MultiSliderAction& operator=(MultiSliderAction&&) noexcept = default;

    virtual Vec<std::reference_wrapper<MultiSliderElement<T>>>
    peek(const Vec<std::unique_ptr<MultiSliderElement<T>>>&) = 0;

    virtual Vec<std::unique_ptr<MultiSliderElement<T>>>&
    execute(Vec<std::unique_ptr<MultiSliderElement<T>>>&) = 0;

    virtual Vec<std::reference_wrapper<juce::Component>> get_temporary_components() = 0;


    static std::reference_wrapper<MultiSliderElement<T>>
    to_reference(const std::unique_ptr<MultiSliderElement<T>>& mse) {
        return std::ref(*mse);
    }


    static Vec<std::reference_wrapper<MultiSliderElement<T>>>
    to_reference(const Vec<std::unique_ptr<MultiSliderElement<T>>>& vec) {
        auto refs = Vec<std::reference_wrapper<MultiSliderElement<T>>>::allocated(vec.size());
        for (const auto& v: vec) {
            refs.append(to_reference(v));
        }
        return refs;
    }
};


// ==============================================================================================

template<typename T>
class MultiSliderInsert : public MultiSliderAction<T> {
public:
    MultiSliderInsert(long index, std::unique_ptr<MultiSliderElement<T>> slider)
            : m_insertion_index(index), m_insertion_slider(std::move(slider)) {
        assert(is_valid());
    }


    Vec<std::reference_wrapper<MultiSliderElement<T>>> peek(const Vec<std::unique_ptr<MultiSliderElement<T>>>& vec) override {
        assert(is_valid());
        return MultiSliderAction<T>::to_reference(vec)
                .insert(m_insertion_index, MultiSliderAction<T>::to_reference(m_insertion_slider));
    }


    Vec<std::unique_ptr<MultiSliderElement<T>>>& execute(Vec<std::unique_ptr<MultiSliderElement<T>>>& vec) override {
        assert(is_valid());
        return vec.insert(m_insertion_index, std::move(m_insertion_slider));
    }

    Vec<std::reference_wrapper<juce::Component>> get_temporary_components() override {
        assert(is_valid());

        Vec<std::reference_wrapper<juce::Component>> components;
        if (m_insertion_slider) {
            components.append(MultiSliderAction<T>::to_reference(m_insertion_slider));
        }
        return components;
    }


    bool is_valid() {
        return static_cast<bool>(m_insertion_slider);
    }


private:
    long m_insertion_index;
    std::unique_ptr<MultiSliderElement<T>> m_insertion_slider;
};


// ==============================================================================================

// TODO: Is this really a class? Rendering-wise it's no real change, the rendering is local for the MultiSliderElement
template<typename T>
class MultiSliderDelete {};


// ==============================================================================================

template<typename T>
class MultiSliderMove {
    // TODO: Probably a good function to implement in Vec!
private:
    std::size_t m_index_from;
    std::size_t m_index_to;
};


// ==============================================================================================

template<typename T>
class MultiSliderSwap {
    // TODO: Probably a good function to implement in Vec!
private:
    std::size_t m_index_from;
    std::size_t m_index_to;
};

// ==============================================================================================
template<typename T>
class MultiSliderDuplicate {
private:
    std::size_t m_index_from;
    std::size_t m_index_to;
};




#endif //SERIALISTLOOPER_MULTI_SLIDER_ACTION_H
