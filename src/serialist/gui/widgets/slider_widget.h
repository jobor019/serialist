
#ifndef SERIALISTLOOPER_SLIDER_WIDGET_H
#define SERIALISTLOOPER_SLIDER_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "policies/parameter_policy.h"
#include "gui/interaction/input_handler.h"
#include "gui/interaction/interaction_visualizer.h"
#include "core/generatives/variable.h"
#include "core/collections/range.h"
#include "core/algo/exponential.h"
#include "core/types/facet.h"
#include "policies/policies.h"

namespace serialist {


class SliderValue : public ParameterHandler::Listener {
public:

    class ValueListener {
    public:
        ValueListener() = default;
        virtual ~ValueListener() = default;
        ValueListener(const ValueListener&) = delete;
        ValueListener& operator=(const ValueListener&) = delete;
        ValueListener(ValueListener&&) noexcept = default;
        ValueListener& operator=(ValueListener&&) noexcept = default;

        virtual void on_value_changed(SliderValue& v, double new_value) = 0;

    };

    class ConfigListener {
    public:
        ConfigListener() = default;
        virtual ~ConfigListener() = default;
        ConfigListener(const ConfigListener&) = delete;
        ConfigListener& operator=(const ConfigListener&) = delete;
        ConfigListener(ConfigListener&&)  noexcept = default;
        ConfigListener& operator=(ConfigListener&&)  noexcept = default;

        virtual void on_configuration_changed(SliderValue& v) = 0;
        virtual void on_bounds_changed(SliderValue& v) = 0;
        virtual void on_exponential_changed(SliderValue& v) = 0;
    };

    // ================


    struct Ids {
        Ids() = delete;
        static const inline std::string VALUE = "value";
        static const inline std::string BOUNDS = "bounds";
        static const inline std::string EXP = "exponential";
        static const inline std::string INTEGRAL = "integral";
        static const inline std::string LHB = "lower_hard_bound";
        static const inline std::string UHB = "upper_hard_bound";
    };

    // ================

    explicit SliderValue(double initial_value
                         , ParameterHandler& ph
                         , const DiscreteRange<double>& bounds = DiscreteRange<double>::from_size(0.0, 1.0, 200, true)
                         , bool is_integral = false
                         , std::optional<double> lower_hard_bound = std::nullopt
                         , std::optional<double> upper_hard_bound = std::nullopt
                         , const Exponential<double>& exponential = Exponential<double>(1.0))
            : m_parent(ph)
            , m_scaled_value(initial_value, Ids::VALUE, m_parent, this)
              , m_bounds(bounds, Ids::BOUNDS, m_parent, this)
              , m_exponential(exponential, Ids::EXP, m_parent, this)
              , m_is_integral(is_integral, Ids::INTEGRAL, m_parent, this)
              , m_lower_hard_bound(lower_hard_bound, Ids::LHB, m_parent, this)
              , m_upper_hard_bound(upper_hard_bound, Ids::UHB, m_parent, this)
              , m_raw_value(inverse(*m_scaled_value)) {
        assert(!*m_lower_hard_bound || !*m_upper_hard_bound || *m_lower_hard_bound < *m_upper_hard_bound);
    }

    SliderValue& with_bounds(const DiscreteRange<double>& bounds) {
        set_bounds(bounds, false);
        return *this;
    }

    SliderValue& with_lower_hard_bound(std::optional<double> lhb) {
        m_lower_hard_bound = lhb;
        return *this;
    }

    SliderValue& with_upper_hard_bound(std::optional<double> upper_hard_bound) {
        m_upper_hard_bound = upper_hard_bound;
        return *this;
    }

    SliderValue& with_exponent(double exponent) {
        set_exponent(exponent, false);
        return *this;
    }

    void add_listener(ValueListener& listener) noexcept {
        m_value_listeners.append(listener);
    }

    void add_listener(ConfigListener& listener) noexcept {
        m_config_listeners.append(listener);
    }


    void remove_listener(ValueListener& listener) noexcept {
        m_value_listeners.remove([&listener](const auto& handler) {
            return std::addressof(handler.get()) == std::addressof(listener);
        });
    }

    void remove_listener(ConfigListener& listener) noexcept {
        m_config_listeners.remove([&listener](const auto& handler) {
            return std::addressof(handler.get()) == std::addressof(listener);
        });
    }


    void load_state(const DeserializationData& dd) {
        m_scaled_value.load_state(dd);
        m_bounds.load_state(dd);
        m_exponential.load_state(dd);
        m_is_integral.load_state(dd);
        m_lower_hard_bound.load_state(dd);
        m_upper_hard_bound.load_state(dd);
    }

    void on_parameter_changed(serialist::VTParameterHandler&, const std::string &id) override {
        if (id == Ids::VALUE) {
            notify_value_change();
        } else if (id == Ids::BOUNDS) {
            ();
        }
    }


    double update_value(double raw_value, bool notify = true) {
        m_raw_value = utils::clip(raw_value, 0.0, 1.0);
        m_scaled_value = process(raw_value);

        if (notify) {
            notify_value_change();
        }

        return *m_scaled_value;
    }

    void set_scaled_value(double scaled_value, bool notify = true) {
        m_scaled_value = m_bounds->clip(scaled_value);
        m_raw_value = inverse(*m_scaled_value);

        if (notify) {
            notify_value_change();
        }
    }

    void set_exponent(double exponent, bool notify = true) {
        auto exponential = m_exponential.get();
        exponential.set_exponent(exponent);
        m_exponential.set(exponential);

        if (notify) {
            for (const auto& listener: m_config_listeners) {
                listener.get().on_exponential_changed(*this);
            }
        }

        // TODO: Update either raw value or scaled value (if exponent changes, one of them has to change)
    }

    void set_bounds(const DiscreteRange<double>& bounds, bool notify = true) {
        std::optional<double> new_start = std::nullopt;
        std::optional<double> new_end = std::nullopt;

        // Note: comparison with `min` rather than `start` in case of an excluded start of range
        if (*m_lower_hard_bound && bounds.get_min() < m_lower_hard_bound->value()) {
            new_start = bounds.get_min();
        }

        // Note: comparison with `max` rather than `end` in case of an excluded end of range
        if (*m_upper_hard_bound && bounds.get_max() > m_upper_hard_bound->value()) {
            new_end = bounds.get_max();
        }

        if (new_start || new_end) {
            m_bounds = bounds.new_adjusted(new_start, new_end);
        } else {
            m_bounds = bounds;
        }

        if (notify) {
            for (const auto& listener: m_config_listeners) {
                listener.get().on_bounds_changed(*this);
            }
        }

        if (!m_bounds->contains(*m_scaled_value)) {
            m_scaled_value = process(m_raw_value);
            if (notify) {
                notify_value_change();
            }
        }

    }

    double get_scaled_value() const { return *m_scaled_value; }

    const DiscreteRange<double>& get_bounds() const { return *m_bounds; }

    const Exponential<double>& get_exponential() const { return *m_exponential; }

    bool is_integral() const { return *m_is_integral; }

    const std::optional<double>& get_lower_hard_bound() const { return *m_lower_hard_bound; }

    const std::optional<double>& get_upper_hard_bound() const { return *m_upper_hard_bound; }

    double get_raw_value() const { return m_raw_value; }

    double get_quantized_raw_value() const {
        return static_cast<double>(m_bounds->map_index(m_raw_value)) / static_cast<double>(m_bounds->size() - 1);
    }

    int get_num_decimals() const {
        if (is_integral())
            return 0;

        if (m_bounds->get_step_size() < 1e-8) {
            return 0;
        }

        return static_cast<int>(std::ceil(std::log10(1.0 / m_bounds->get_step_size())));
    }


private:
    double process(double raw_value) const {
        auto v = m_bounds->map(m_exponential.get().apply(raw_value));

        if (*m_is_integral)
            return std::floor(v);

        return v;
    }

    double inverse(double scaled_value) const {
        return m_exponential->inverse(m_bounds->inverse(scaled_value));
    }

    void notify_value_change() {
        for (const auto& listener: m_value_listeners) {
            listener.get().on_value_changed(*this, *m_scaled_value);
        }
    }

    ParameterHandler& m_parent;

    GuiParameter<double> m_scaled_value;

    // TODO: Not sure if DiscreteRange is the best solution here.
    //       A SliderValue should be able to be unbounded in either direction (when used as a numbox)
    GuiParameter<DiscreteRange<double>> m_bounds;
    GuiParameter<Exponential<double>> m_exponential;

    GuiParameter<bool> m_is_integral;
    // TODO: snap_to_nearest bool + double

    GuiParameter<std::optional<double>> m_lower_hard_bound;
    GuiParameter<std::optional<double>> m_upper_hard_bound;

    double m_raw_value;

    Vec<std::reference_wrapper<ValueListener>> m_value_listeners;
    Vec<std::reference_wrapper<ConfigListener>> m_config_listeners;
};


// ==============================================================================================

class Slider : public juce::Component, public SliderValue::ValueListener {
public:
    struct KeyCodes {
        static const int fine_tune = static_cast<int>('Q');
        static const int adjust_range = static_cast<int>('W');
    };

    // ================

    enum class Layout {
        horizontal, vertical
    };


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

        explicit EditMode(SliderValue& value) : m_value(value) {}

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
            m_value.update_value(m_value.get_raw_value() + delta, true);
        }

        SliderValue& m_value;

        std::optional<OngoingDrag> m_ongoing_drag = std::nullopt;
    };

    // ================

    class FineTuneMode : public InputMode {
    public:
        explicit FineTuneMode(SliderValue& value) : m_value(value) {}

        bool intercept_mouse() override { return false; }

        void reset() override {} // TODO

        // TODO

    private:
        SliderValue& m_value;
    };

    // ================

    class RangeAdjustmentMode : public InputMode {
    public:
        explicit RangeAdjustmentMode(SliderValue& value) : m_value(value) {}

        bool intercept_mouse() override { return true; }

        void reset() override {} // TODO

        // TODO

    private:
        SliderValue& m_value;
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

    class RangeAdjustmentVisualization : public InteractionVisualization, public SliderValue::ValueListener {
    public:
        explicit RangeAdjustmentVisualization(SliderValue& value) : m_value(value) {
            m_value.add_listener(*this);
//            addChildComponent(m_fill);

        }

        ~RangeAdjustmentVisualization() override {
            m_value.remove_listener(*this);
        }

        void paint(juce::Graphics &g) override {
            g.setColour(juce::Colours::tan);
            g.fillAll();
            g.setColour(juce::Colours::whitesmoke);
            g.drawRect(getLocalBounds());

            auto bounds = getLocalBounds();
            auto div = getWidth() / 3;
            auto num_decimals = m_value.get_num_decimals();

            g.drawFittedText(juce::String(m_value.get_bounds().get_start()
                                          , num_decimals)
                                          , bounds.removeFromLeft(div)
                                          , juce::Justification::centred, 1);

            g.drawFittedText(juce::String(m_value.get_bounds().size())
                                          , bounds.removeFromLeft(div)
                                          , juce::Justification::centred, 1);

            g.drawFittedText(juce::String(m_value.get_bounds().get_end()
                                          , num_decimals)
                                          , bounds.removeFromLeft(div)
                                          , juce::Justification::centred, 1);
        }



        void resized() override {
//            m_fill.setBounds(getLocalBounds());
        }

        void state_changed(InputMode* active_mode, int state, AlphaMask& alpha) override {
            (void) state;
            (void) alpha; // TODO
            setVisible(active_mode && active_mode->is<RangeAdjustmentMode>());
//            m_fill.setVisible(active_mode && active_mode->is<RangeAdjustmentMode>());
        }


        void on_bounds_changed(const DiscreteRange<double>&) override {
            repaint();
        }

        void on_value_changed(double) override {}

    private:
        SliderValue& m_value;

//        FillHighlight m_fill{juce::Colours::brown.withAlpha(0.8f)};
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    explicit Slider(InputHandler* parent_input_handler
                    , const SliderValue& value = SliderValue()
                    , Layout layout = Layout::horizontal
                    , ConditionVec&& edit_mode_conditions = conditions::no_key()
                    , ConditionVec&& fine_tune_mode_conditions = conditions::key(KeyCodes::fine_tune)
                    , ConditionVec&& adjust_range_mode_conditions = conditions::key(KeyCodes::adjust_range))
            : m_value(value)
              , m_interaction_visualizer(*this, default_visualizations(m_value))
              , m_input_handler(parent_input_handler
                                , *this
                                , default_modes(m_value
                                                , std::move(edit_mode_conditions)
                                                , std::move(fine_tune_mode_conditions)
                                                , std::move(adjust_range_mode_conditions))
                                , {m_interaction_visualizer}
                                , nullptr)
              , m_layout(layout) {
        m_value.add_listener(*this);
        addAndMakeVisible(m_interaction_visualizer);
    }

    static InputModeMap default_modes(SliderValue& value
                                      , ConditionVec edit
                                      , ConditionVec fine_tune
                                      , ConditionVec adjust_range) {
        InputModeMap map;
        map.add(std::move(edit), std::make_unique<EditMode>(value));

        if (!fine_tune.empty())
            map.add(std::move(fine_tune), std::make_unique<FineTuneMode>(value));

        if (!adjust_range.empty())
            map.add(std::move(adjust_range), std::make_unique<RangeAdjustmentMode>(value));

        return map;
    }


    static VisualizationVec default_visualizations(SliderValue& value) {
        VisualizationVec v;
        v.append(std::make_unique<EditVisualization>());
        v.append(std::make_unique<RangeAdjustmentVisualization>(value));
        return v;
    }

    void on_value_changed(double) override {
        repaint();
    }


//    void add_listener(Listener& listener) { m_listeners.append(&listener); }
//
//    void remove_listener(Listener& listener) { m_listeners.remove(&listener); }

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colours::cornflowerblue);

        if (m_layout == Layout::horizontal) {
            paint_horizontal_bar(g);
        } else {
            paint_vertical_bar(g);
        }

        g.setColour(juce::Colours::whitesmoke);
        g.drawFittedText(juce::String(m_value.get_scaled_value(), static_cast<int>(m_value.get_num_decimals()))
                         , getLocalBounds().reduced(4)
                         , juce::Justification::centred, 1);

        g.drawRect(getLocalBounds());
    }

    void paint_horizontal_bar(juce::Graphics& g) {
        auto component_width = getWidth();
        auto slider_width = static_cast<double>(component_width) * m_value.get_quantized_raw_value();
        g.fillRect(getLocalBounds().withTrimmedRight(component_width - static_cast<int>(slider_width)));

    }

    void paint_vertical_bar(juce::Graphics& g) {
        auto component_height = getHeight();
        auto slider_height = static_cast<double>(component_height) * m_value.get_quantized_raw_value();
        g.fillRect(getLocalBounds().withTrimmedTop(component_height - static_cast<int>(slider_height)));
    }

    void resized() override {
        m_interaction_visualizer.setBounds(getLocalBounds());
    }

//    void set_value(double v, bool notify_listeners = true) {
//        m_raw_value = m_bounds.inverse(m_scaled_value);
//        update_scaled_value_only(v, notify_listeners);
//    }
//
//    void set_min(double v, bool notify_listeners = true) {
//        m_bounds.set_lower_bound(v);
//        if (notify_listeners) notify_bounds_change();
//        repaint();
//    }
//
//    void set_max(double v, bool notify_listeners = true) {
//        m_bounds.set_upper_bound(v);
//        if (notify_listeners) notify_bounds_change();
//        repaint();
//    }

    void set_layout(Layout layout) {
        m_layout = layout;
        repaint();
    }

    SliderValue& get_value()  { return m_value; }


private:
//    void update_raw_value(double raw_value, bool notify_listeners = true) {
//        m_raw_value = utils::clip(raw_value, {0.0}, {1.0});
//        update_scaled_value_only(m_bounds.apply(m_raw_value), notify_listeners);
//    }
//
//    void update_scaled_value_only(double v, bool notify_listeners = true) {
//        auto previous_value = m_scaled_value;
//
//        m_scaled_value = m_bounds.clip(v);
//
//        // TODO: For slider with very small values, an explicit epsilon as a fraction of
//        //  the slider's range is probably a better idea
//        if (!utils::equals(m_scaled_value, previous_value)) {
//            if (notify_listeners) {
//                notify_value_change();
//            }
//
//            repaint();
//        }
//    }

//    void notify_value_change() const {
//        for (auto* listener: m_listeners) {
//            listener->on_value_changed(m_scaled_value);
//        }
//    }
//
//    void notify_bounds_change() const {
//        for (auto* listener: m_listeners) {
//            listener->on_bounds_changed(m_bounds);
//        }
//    }

//    Vec<Listener*> m_listeners;

    SliderValue m_value;

    InteractionVisualizer m_interaction_visualizer;
    InputHandler m_input_handler;


    Layout m_layout;


};


// ==============================================================================================

class SliderWidget : public GenerativeComponent, public SliderValue::ValueListener {
public:
    //    SliderWidget(InputHandler*, juce::ValueTree& gui_state_vt, Variable<Facet> variable, ...)

private:
    ParameterHandler m_ph;

    Slider m_value;
    juce::Label m_label;
    Variable<Facet> m_variable;
};

} // namespace serialist

#endif //SERIALISTLOOPER_SLIDER_WIDGET_H
