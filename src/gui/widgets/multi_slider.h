
#ifndef SERIALISTLOOPER_MULTI_SLIDER_H
#define SERIALISTLOOPER_MULTI_SLIDER_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "interaction/drag_and_drop/drag_and_drop.h"
#include "interaction/input_handler.h"
#include "interaction/interaction_visualizer.h"
#include "slider_widget.h"


class MultiSlider : public juce::Component, public DropArea {
public:

    // TODO: Not sure if this should be the behaviour of the Slider or the MultiSlider
    //  (we could probably implement something similar directly in the Slider, i.e. a PaintMode where we don't
    //  require the MouseDown to be registered to allow paint editing
    class MultiEdit : public InputMode {
    public:
        explicit MultiEdit(MultiSlider& parent) : m_parent(parent) {}


        bool intercept_mouse() override {
            return true;
        }

        DragBehaviour get_drag_behaviour() override {
            return DragBehaviour::drag_edit;
        }

        std::optional<int> mouse_position_changed(const MouseState& mouse_state) override {
            if (mouse_state.is_drag_editing) {
                if (auto slider = dynamic_cast<Slider*>(m_parent.getComponentAt(*mouse_state.position))) {
                    auto height = static_cast<double>(m_parent.getHeight());
                    auto x = 1 - (static_cast<double>(mouse_state.position->y) / height);
                    slider->get_value().update_value(x);
                }
            }
            return std::nullopt;
        }

        void reset() override {}

    private:
        MultiSlider& m_parent;
    };


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    MultiSlider(InputHandler* parent, GlobalDragAndDropContainer& global_dnd_container)
            : m_visualizer(*this, {})
              , m_input_handler(parent, *this, default_map(*this), {}, &global_dnd_container) {}


    static InputModeMap default_map(MultiSlider& parent) {
        InputModeMap map;
        map.add(conditions::key('Q'), std::make_unique<MultiEdit>(parent));
        return map;
    }

    void resized() override {
        if (!m_sliders.empty()) {
            auto slider_width = static_cast<double>(getWidth()) / static_cast<double>(m_sliders.size());
            double x = 0.0;

            for (const auto& slider: m_sliders) {
                slider->setBounds(static_cast<int>(x) + 1, 0, static_cast<int>(slider_width) - 1, getHeight());
                x += slider_width;
            }
        }

        m_visualizer.setBounds(getLocalBounds());
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::slategrey);
    }

    void add_slider() {
        auto s = std::make_unique<Slider>(&m_input_handler, SliderValue(), Slider::Layout::vertical);
        addAndMakeVisible(*s);
        m_sliders.append(std::move(s));
    }

    DropListener& get_drop_listener() override {
        return m_input_handler;
    }


private:
    InteractionVisualizer m_visualizer;
    InputHandler m_input_handler;


    Vec<std::unique_ptr<Slider>> m_sliders;
};

#endif //SERIALISTLOOPER_MULTI_SLIDER_H
