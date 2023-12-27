
#ifndef SERIALISTLOOPER_BAR_SLIDER_H
#define SERIALISTLOOPER_BAR_SLIDER_H

#include "multi_slider.h"

template<typename T>
class BarSlider : public MultiSliderElement<T> {
public:
    explicit BarSlider(const Voice<T>& initial_value = {}) : MultiSliderElement<T>() {
        static_assert(std::is_arithmetic_v<T>);
        m_value = initial_value;
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::rebeccapurple);
        g.setColour(SerialistLookAndFeel::TEXT_C);
        g.drawFittedText(std::to_string(m_value.first_or(0))
                         , MultiSliderElement<T>::getLocalBounds()
                         , juce::Justification::centred, 1);
    }


    const Voice<T>& get_rendered_value() const override {
        return m_value;
    }

    const Voice<T>& get_actual_value() const override {
        return m_value;
    }


    void set_value(const Voice<T>& value) override {
        m_value = value;
    }


protected:
    bool update_state(const MouseState<>& mouse_state) override {
        if (mouse_state.is_down and !mouse_state.is_dragging) {
            m_value[0] += 1;
            return true;
        }
        return false;
    }


    void on_resized() override {
        // TODO
    }


private:
    Voice<T> m_value;


};


// ==============================================================================================

template<typename T>
class BarSliderConfig : public MultiSliderConfig<T> {
public:
    BarSliderConfig() : MultiSliderConfig<T>() {
        static_assert(std::is_arithmetic_v<T>);
    }


    std::unique_ptr<MultiSliderElement<T>> create_new_slider(const Voice<T>& initial_value) override {
        std::unique_ptr<MultiSliderElement<T>> slider = std::make_unique<BarSlider<T>>(initial_value);
        return std::move(slider);
    }


};

#endif //SERIALISTLOOPER_BAR_SLIDER_H
