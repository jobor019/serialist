
#ifndef SERIALISTLOOPER_SLIDER_WIDGET_H
#define SERIALISTLOOPER_SLIDER_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "interaction/input_handler.h"
#include "interaction/interaction_visualizer.h"
#include "core/generatives/variable.h"


class Slider : public juce::Component {
public:
    struct KeyCodes {
        static const int fine_tune = static_cast<int>('Q');
        static const int adjust_range = static_cast<int>('W');
    };

    // ================

    enum class Layout {
        horizontal, vertical
    };

    // ================



    // ================

    class Listener {
    public:
        Listener() = default;

        virtual ~Listener() = default;

        Listener(const Listener&) = delete;

        Listener& operator=(const Listener&) = delete;

        Listener(Listener&&) noexcept = default;

        Listener& operator=(Listener&&) noexcept = default;

        virtual void on_value_changed(double new_value) = 0;

        virtual void on_bounds_changed(const Bounds& bounds) = 0;
    };

    // ================

    class OngoingDrag {
    public:
        explicit OngoingDrag(int displacement_range_px)
                : displacement_increment(displacement_range_px > 0
                                         ? 1.0 / static_cast<double>(displacement_range_px)
                                         : 0.0) {}


        double get_delta(int displacement_px) {
            auto delta = -static_cast<double>(displacement_px - current_displacement_px) * displacement_increment;
            current_displacement_px = displacement_px;

            return delta;
        }

        int current_displacement_px = 0;
        double displacement_increment;
    };


    // ================

    class EditMode : public InputMode {
    public:
        // TODO: Having a range in pixels is probably not ideal when moving between screens with different resolutions
        static const int RANGE_PX = 200;

        enum class State {
            non_hover
            , hover
            , editing
        };

        explicit EditMode(Slider& slider) : m_slider(slider) {}

        bool intercept_mouse() override { return false; }

        DragBehaviour get_drag_behaviour() override {
            return DragBehaviour::hide_and_restore;
        }

        std::optional<int> mouse_state_changed(const MouseState& mouse_state) override {
            if (!m_ongoing_drag && mouse_state.is_drag_editing) {
                drag_started();
            } else if (m_ongoing_drag && !mouse_state.is_drag_editing) {
                drag_ended();
            }

            if (mouse_state.is_drag_editing) {
                set_value(mouse_state);
                return static_cast<int>(State::editing);
            } else if (mouse_state.is_active_over_component()) {
                return static_cast<int>(State::hover);
            } else {
                return static_cast<int>(State::non_hover);
            }
        }

        std::optional<int> mouse_position_changed(const MouseState& mouse_state) override {
            if (mouse_state.is_drag_editing) {
                set_value(mouse_state);
            }

            return std::nullopt;
        }


        void reset() override {
            m_ongoing_drag = std::nullopt;
        }

    private:
        void drag_started() {
            // TODO: Temp solution for linker issue of static const declared in header file + make_optional
            auto px = RANGE_PX;
            m_ongoing_drag.emplace(px);
        }

        void drag_ended() {
            m_ongoing_drag = std::nullopt;
        }


        void set_value(const MouseState& mouse_state) {
            assert(m_ongoing_drag);

            auto delta = m_ongoing_drag->get_delta(mouse_state.drag_displacement->getY());
            m_slider.update_raw_value(m_slider.m_raw_value + delta, true);
        }

        Slider& m_slider;

        std::optional<OngoingDrag> m_ongoing_drag = std::nullopt;
    };

    // ================

    class FineTuneMode : public InputMode {
    public:
        explicit FineTuneMode(Slider& slider) : m_slider(slider) {}

        bool intercept_mouse() override { return false; }

        void reset() override {} // TODO

        // TODO

    private:
        Slider& m_slider;
    };

    // ================

    class RangeAdjustmentMode : public InputMode {
    public:
        explicit RangeAdjustmentMode(Slider& slider) : m_slider(slider) {}

        bool intercept_mouse() override { return true; }

        void reset() override {} // TODO

        // TODO

    private:
        Slider& m_slider;
    };

    // ================

    class ExponentMode : public InputMode {
    public:
        // TODO: Mode for setting the exponent of the scaling
    };

    // ================

    class TypeEnterMode : public InputMode {
        // TODO: Mode for manually entering value by typing
    };

    // ================

    class EditVisualization : public InteractionVisualization {
    public:
        EditVisualization() {
            addChildComponent(m_border);
            addChildComponent(m_fill);
        }

        void resized() override {
            m_border.setBounds(getLocalBounds());
            m_fill.setBounds(getLocalBounds());
        }

        void state_changed(InputMode* active_mode, int state, AlphaMask&) override {
            const auto is_active = active_mode && (active_mode->is<EditMode>() || active_mode->is<FineTuneMode>());
            m_border.setVisible(is_active && (state == static_cast<int>(EditMode::State::hover)
                                              || state == static_cast<int>(EditMode::State::editing)));
            m_fill.setVisible(is_active && state == static_cast<int>(EditMode::State::editing));
        }


    private:
        // TODO: Test SerialistLookAndFeel
        BorderHighlight m_border{juce::Colours::gold};
        FillHighlight m_fill{juce::Colours::gold.withAlpha(0.2f)};
    };

    // ================

    class RangeAdjustmentVisualization : public InteractionVisualization, public Listener {
    public:
        explicit RangeAdjustmentVisualization(Slider& parent) : m_parent(parent) {
            m_parent.add_listener(*this);
            addChildComponent(m_fill);

        }

        ~RangeAdjustmentVisualization() override {
            m_parent.remove_listener(*this);
        }

        void resized() override {
            m_fill.setBounds(getLocalBounds());
        }

        void state_changed(InputMode* active_mode, int state, AlphaMask& alpha) override {
            (void) state;
            (void) alpha; // TODO
            m_fill.setVisible(active_mode && active_mode->is<RangeAdjustmentMode>());
        }

        void on_bounds_changed(const Slider::Bounds& bounds) override {
            (void) bounds; // TODO
        }

        void on_value_changed(double) override {}

    private:
        Slider& m_parent;

        FillHighlight m_fill{juce::Colours::brown.withAlpha(0.8f)};
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    explicit Slider(InputHandler* parent_input_handler
                    , double initial_value = 0.0
                    , const Bounds& bounds = Bounds()
                    , Layout layout = Layout::horizontal
                    , ConditionVec&& edit_mode_conditions = conditions::no_key()
                    , ConditionVec&& fine_tune_mode_conditions = conditions::key(KeyCodes::fine_tune)
                    , ConditionVec&& adjust_range_mode_conditions = conditions::key(KeyCodes::adjust_range))
            : m_interaction_visualizer(*this, default_visualizations(*this))
              , m_input_handler(parent_input_handler
                                , *this
                                , default_modes(*this
                                                , std::move(edit_mode_conditions)
                                                , std::move(fine_tune_mode_conditions)
                                                , std::move(adjust_range_mode_conditions))
                                , {m_interaction_visualizer}
                                , nullptr)
              , m_bounds(bounds)
              , m_raw_value(m_bounds.inverse(initial_value))
              , m_scaled_value(initial_value)

              , m_layout(layout) {
        addAndMakeVisible(m_interaction_visualizer);
    }

    static InputModeMap default_modes(Slider& managed_component
                                      , ConditionVec edit
                                      , ConditionVec fine_tune
                                      , ConditionVec adjust_range) {
        InputModeMap map;
        map.add(std::move(edit), std::make_unique<EditMode>(managed_component));
        map.add(std::move(fine_tune), std::make_unique<FineTuneMode>(managed_component));
        map.add(std::move(adjust_range), std::make_unique<RangeAdjustmentMode>(managed_component));
        return map;
    }


    static VisualizationVec default_visualizations(Slider& managed_component) {
        VisualizationVec v;
        v.append(std::make_unique<EditVisualization>());
        v.append(std::make_unique<RangeAdjustmentVisualization>(managed_component));
        return v;
    }


    void add_listener(Listener& listener) { m_listeners.append(&listener); }

    void remove_listener(Listener& listener) { m_listeners.remove(&listener); }

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colours::whitesmoke);
        g.drawFittedText(juce::String(m_scaled_value, 2), getLocalBounds(), juce::Justification::centred, 1);
    }

    void resized() override {
        m_interaction_visualizer.setBounds(getLocalBounds());
    }

    void set_value(double v, bool notify_listeners = true) {
        m_raw_value = m_bounds.inverse(m_scaled_value);
        update_scaled_value_only(v, notify_listeners);
    }

    void set_min(double v, bool notify_listeners = true) {
        m_bounds.set_lower_bound(v);
        if (notify_listeners) notify_bounds_change();
        repaint();
    }

    void set_max(double v, bool notify_listeners = true) {
        m_bounds.set_upper_bound(v);
        if (notify_listeners) notify_bounds_change();
        repaint();
    }

    void set_layout(Layout layout) {
        m_layout = layout;
        repaint();
    }

    double get_value() const { return m_scaled_value; }

    const Bounds& get_bounds() const { return m_bounds; }


private:
    void update_raw_value(double raw_value, bool notify_listeners = true) {
        m_raw_value = utils::clip(raw_value, {0.0}, {1.0});
        update_scaled_value_only(m_bounds.apply(m_raw_value), notify_listeners);
    }

    void update_scaled_value_only(double v, bool notify_listeners = true) {
        auto previous_value = m_scaled_value;

        m_scaled_value = m_bounds.clip(v);

        // TODO: For slider with very small values, an explicit epsilon as a fraction of
        //  the slider's range is probably a better idea
        if (!utils::equals(m_scaled_value, previous_value)) {
            if (notify_listeners) {
                notify_value_change();
            }

            repaint();
        }
    }

    void notify_value_change() const {
        for (auto* listener: m_listeners) {
            listener->on_value_changed(m_scaled_value);
        }
    }

    void notify_bounds_change() const {
        for (auto* listener: m_listeners) {
            listener->on_bounds_changed(m_bounds);
        }
    }

    Vec<Listener*> m_listeners;

    InteractionVisualizer m_interaction_visualizer;
    InputHandler m_input_handler;

    Bounds m_bounds;


    Layout m_layout;


};


// ==============================================================================================

class SliderWidget {
private:
    Slider m_slider;
    juce::Label m_label;
    Variable<Facet> m_variable;
};

#endif //SERIALISTLOOPER_SLIDER_WIDGET_H
