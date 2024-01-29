

#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include "core/param/parameter_policy.h"
#include "key_state.h"
#include "gui/interaction/input_handler.h"
#include "interaction/interaction_visualizer.h"


class DummyTextInterface {
public:
    DummyTextInterface() = default;

    virtual ~DummyTextInterface() = default;

    DummyTextInterface(const DummyTextInterface&) = default;

    DummyTextInterface& operator=(const DummyTextInterface&) = default;

    DummyTextInterface(DummyTextInterface&&) noexcept = default;

    DummyTextInterface& operator=(DummyTextInterface&&) noexcept = default;

    virtual void set_text(const std::string& text) = 0;


    virtual int get_drag_type() const { return 0; }


    virtual void reset() = 0;


    virtual std::string get_identifier() const { return ""; }
};

class ValueInterface {
public:
    ValueInterface() = default;

    virtual ~ValueInterface() = default;

    ValueInterface(const ValueInterface&) = delete;

    ValueInterface& operator=(const ValueInterface&) = delete;

    ValueInterface(ValueInterface&&) noexcept = default;

    ValueInterface& operator=(ValueInterface&&) noexcept = default;

    virtual void set_value(float value) = 0;
};

class DummyDragInfo : public DragInfo {
public:
    explicit DummyDragInfo(const DummyTextInterface& ti) : text_interface(ti) {}


    const DummyTextInterface& text_interface;
};


// ==============================================================================================

class DefaultMode : public InputMode {
public:
    explicit DefaultMode(DummyTextInterface& text_component)
            : m_text_component(text_component) {}


    bool intercept_mouse() override {
        return false;
    }


    std::optional<int> mode_activated() override {
//        std::cout << "DEFAULT MODE ACTIVATED (text: " << m_text << ")" << std::endl;
        m_text_component.reset();
        return 0;
    }


    std::optional<int> mouse_state_changed(const MouseState&) override {
        return std::nullopt;
    }


    std::optional<int> mouse_position_changed(const MouseState&) override {
        return std::nullopt;
    }


    std::optional<int> input_event_registered(std::unique_ptr<InputEvent>) override {
        return std::nullopt;
    }


    void reset() override {}


private:
    DummyTextInterface& m_text_component;
    std::string m_text;
};


// ==============================================================================================

class DragAndDropMode : public InputMode {
public:
    enum class State {
        enabled = 1
        , hovering = 2
        , dragging_from = 3
        , dragging_to = 4
        , valid_target = 5
    };

    class Visualization : public InteractionVisualization {
    public:
        Visualization() {
            addChildComponent(m_enabled);
            addChildComponent(m_hover);
            addChildComponent(m_drag_to);
            addChildComponent(m_valid_target);
        }


        void resized() override {
            m_enabled.setBounds(getLocalBounds());
            m_hover.setBounds(getLocalBounds());
            m_drag_to.setBounds(getLocalBounds());
            m_valid_target.setBounds(getLocalBounds().reduced(2));
        }


        void state_changed(InputMode* active_mode, int state, AlphaMask& alpha) override {
            const auto is_active = active_mode && active_mode->is<DragAndDropMode>();
//            std::cout << "state changed: " << state << "(is active: " << is_active << ")\n";
            m_enabled.setVisible(is_active && state == static_cast<int>(State::enabled));
            m_hover.setVisible(is_active && state == static_cast<int>(State::hovering));
            m_drag_to.setVisible(is_active && state == static_cast<int>(State::dragging_to));
            m_valid_target.setVisible(is_active && (state == static_cast<int>(State::valid_target)
                                                    || state == static_cast<int>(State::dragging_to)));

            if (is_active && state == static_cast<int>(State::dragging_from)) {
                alpha.set_alpha(0.4f);
            } else {
                alpha.set_alpha(1.0f);
            }
        }


    private:
        BorderHighlight m_enabled{juce::Colours::darkgreen.withAlpha(0.6f), 2};
        BorderHighlight m_hover{juce::Colours::darkgreen, 4};
        FillHighlight m_drag_to{juce::Colours::darkturquoise.withAlpha(0.3f)};
        BorderHighlight m_valid_target{juce::Colours::palegoldenrod.withAlpha(0.8f)};
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    explicit DragAndDropMode(DummyTextInterface& dnd_component) : m_text_interface(dnd_component) {}


    bool intercept_mouse() override {
        return true;
    }


    DragBehaviour get_drag_behaviour() override {
        return DragBehaviour::drag_and_drop;
    }


    std::unique_ptr<DragInfo> get_drag_info() override {
        auto info = std::make_unique<DummyDragInfo>(m_text_interface);
//        m_source = info.get();
        return std::move(info);
    }


//    std::optional<juce::ScaledImage> get_drag_image() const override {
//        return juce::ScaledImage(m_text_interface.createComponentSnapshot(m_text_interface.getLocalBounds()));
//    }

    std::optional<int> item_dropped(const DragInfo&) override {
        std::cout << "!!!!!! ITEM DROPPED !!!!!!!!!" << std::endl;
        m_text_interface.set_text("DnD enabled");
        return static_cast<int>(State::enabled);
    }


    bool supports_drop_from(const DragInfo& source) const override {
        const auto* ti = dynamic_cast<const DummyDragInfo*>(&source);
        return ti &&
               ti->text_interface.get_drag_type() == m_text_interface.get_drag_type() &&
               &ti->text_interface != &m_text_interface;
    }


    std::optional<int> mouse_state_changed(const MouseState& mouse_state) override {
        if (mouse_state.is_dragging_from) {
            m_text_interface.set_text("Dragging (from)...");
            return static_cast<int>(State::dragging_from);

        } else if (mouse_state.is_dragging_to) {
            m_text_interface.set_text("Drop here!");
            return static_cast<int>(State::dragging_to);

        } else if (mouse_state.has_external_ongoing_drag) {
            m_text_interface.set_text("Can be dropped here!");
            return static_cast<int>(State::valid_target);

        } else if (!mouse_state.is_active_over_component()) {
            m_text_interface.set_text("DnD enabled");
            return static_cast<int>(State::enabled);

        } else if (mouse_state.is_active_over_component()) {
            m_text_interface.set_text("Click to start drag!");
            return static_cast<int>(State::hovering);

        } else {
            return std::nullopt;
        }
    }


    std::optional<int> mouse_position_changed(const MouseState&) override {
        return std::nullopt;
    }


    std::optional<int> input_event_registered(std::unique_ptr<InputEvent> input_event) override {
        if (input_event->is<DragDropped>()) {
            std::cout << "Dragdropped\n";
        }
        return std::nullopt;
    }


    void reset() override {
//        std::cout << m_text_interface.get_identifier() << "::resetting!\n";
        m_text_interface.reset();
    }


    const DummyTextInterface& get_text_interface() const {
        return m_text_interface;
    }


private:
    DummyTextInterface& m_text_interface;

//    DummyDragInfo* m_source = nullptr;
};


// ==============================================================================================

class DragEditMode : public InputMode {
public:
    enum class State {
        enabled = 1
        , hovering = 2
        , drag_editing = 3
    };

    class Visualization : public InteractionVisualization {
    public:
        Visualization() {
            addChildComponent(m_enabled);
            addChildComponent(m_hover);
            addChildComponent(m_dragging);
        }

        void resized() override {
            m_enabled.setBounds(getLocalBounds());
            m_hover.setBounds(getLocalBounds().reduced(2));
            m_dragging.setBounds(getLocalBounds());
        }

        void state_changed(InputMode* active_mode, int state, AlphaMask&) override {
            const auto is_active = active_mode && active_mode->is<DragEditMode>();
            m_enabled.setVisible(is_active && state == static_cast<int>(State::enabled));
            m_hover.setVisible(is_active && state == static_cast<int>(State::hovering));
            m_dragging.setVisible(is_active && state == static_cast<int>(State::drag_editing));
        }

    private:
        BorderHighlight m_enabled{juce::Colours::goldenrod.withAlpha(0.6f), 2};
        BorderHighlight m_hover{juce::Colours::gold, 4};
        FillHighlight m_dragging{juce::Colours::gold.withAlpha(0.2f)};
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    explicit DragEditMode(DummyTextInterface& text_interface
                          , ValueInterface& value_interface
                          , DragBehaviour drag_behaviour
                          , bool intercept_mouse)
            : m_text_interface(text_interface)
              , m_value_interface(value_interface)
              , m_drag_behaviour(drag_behaviour)
              , m_intercept(intercept_mouse) {}

    bool intercept_mouse() override {
        return m_intercept;
    }

    std::optional<int> mouse_state_changed(const MouseState& mouse_state) override {
        if (mouse_state.is_drag_editing) {
            m_text_interface.set_text("Dragging!");
            return static_cast<int>(State::drag_editing);
        }

        if (mouse_state.is_active_over_component()) {
            m_text_interface.set_text("Hovering...");
            return static_cast<int>(State::hovering);
        }

        if (!mouse_state.is_active_over_component()) {
            m_text_interface.set_text("DragEdit Enabled");
            return static_cast<int>(State::enabled);
        }

        return std::nullopt;
    }

    std::optional<int> mouse_position_changed(const MouseState& mouse_state) override {
        if (mouse_state.drag_displacement) {
            m_value_interface.set_value(-static_cast<float>(mouse_state.drag_displacement.value().getY()));
        }
        return std::nullopt;
    }

    DragBehaviour get_drag_behaviour() override {
        return m_drag_behaviour;
    }

    std::optional<int> input_event_registered(std::unique_ptr<InputEvent>) override {
        std::cout << "some input event??";
        return std::nullopt;
    }


    void reset() override {}


private:
    DummyTextInterface& m_text_interface;
    ValueInterface& m_value_interface;

    const DragBehaviour m_drag_behaviour;

    bool m_intercept;


};


// ==============================================================================================

class MoveMode : public InputMode {
public:
    enum class State {
        enabled = 0
        , hovering = 1
        , dragging = 2
    };

    class Visualization : public InteractionVisualization {
    public:
        Visualization() {
            addChildComponent(m_enabled);
            addChildComponent(m_hover);
            addChildComponent(m_ongoing_move);
        }


        void mouseMove(const juce::MouseEvent&) override {
            std::cout << "VISUALIZATION\n";
        }

//        void paint(juce::Graphics &g) override {
//            g.fillAll(juce::Colours::palegoldenrod.withAlpha(0.5f));
//        }


        void resized() override {
            m_enabled.setBounds(getLocalBounds());
            m_hover.setBounds(getLocalBounds());
            m_ongoing_move.setBounds(getLocalBounds());
        }


        void state_changed(InputMode* active_mode, int state, AlphaMask&) override {
            auto is_move = active_mode && active_mode->is<MoveMode>();
            m_enabled.setVisible(is_move && state == static_cast<int>(MoveMode::State::enabled));
            m_hover.setVisible(is_move && (state == static_cast<int>(MoveMode::State::hovering) ||
                                           state == static_cast<int>(MoveMode::State::dragging)));
            m_ongoing_move.setVisible(is_move && state == static_cast<int>(MoveMode::State::dragging));
        }


    private:
        BorderHighlight m_enabled{juce::Colours::indigo.withAlpha(0.6f), 1};
        BorderHighlight m_hover{juce::Colours::yellowgreen, 2};
        FillHighlight m_ongoing_move{juce::Colours::palegoldenrod.withAlpha(0.1f)};

    };


    explicit MoveMode(juce::Component& managed_component
                      , DummyTextInterface& text_interface)
            : m_managed_component(managed_component)
              , m_text_component(text_interface) {}


    bool intercept_mouse() override {
        return true;
    }


    std::optional<int> mode_activated() override {
        m_text_component.set_text("Drag to move!");
        return std::nullopt;
    }


    std::optional<int> mouse_state_changed(const MouseState& mouse_state) override {

        if (!mouse_state.is_over_component()) {
            return static_cast<int>(State::enabled);

        } else if (mouse_state.is_down && !m_mouse_down_position) {
            assert(mouse_state.position);
            m_mouse_down_position = mouse_state.position;
            return static_cast<int>(State::dragging);

        } else if (!mouse_state.is_down && m_mouse_down_position) {
            m_mouse_down_position = std::nullopt;
            return static_cast<int>(State::hovering);

        } else if (!mouse_state.is_down && mouse_state.is_active_over_component()) {
            return static_cast<int>(State::hovering);

        } else {
            return std::nullopt;
        }
    }


    std::optional<int> mouse_position_changed(const MouseState& mouse_state) override {
        if (m_mouse_down_position) {
            assert(mouse_state.position);
            auto bounds = m_managed_component.getBounds();
            bounds += mouse_state.position.value() - m_mouse_down_position.value();
            m_managed_component.setBounds(bounds);
        }
        return std::nullopt;

    }


    std::optional<int> input_event_registered(std::unique_ptr<InputEvent>) override {
        std::cout << "some input event??";
        return std::nullopt;
    }


    void reset() override {
        m_mouse_down_position = std::nullopt;
    }


private:
    juce::Component& m_managed_component;
    DummyTextInterface& m_text_component;
    std::optional<juce::Point<int>> m_mouse_down_position = std::nullopt;
};


// ==============================================================================================

class MovableComponent : public juce::Component, public DummyTextInterface {
public:
    MovableComponent(const std::string& default_text
                     , Vec<std::unique_ptr<InputCondition>> move_conditions
                     , GlobalDragAndDropContainer& dnd_container
                     , const std::string& identifier
                     , bool add_fallback_mode)
            : m_default_text(default_text)
              , m_input_handler(nullptr, *this
                                , default_modes(*this, std::move(move_conditions), add_fallback_mode)
                                , dnd_container
                                , {std::ref(m_visualizer)}
                                , identifier) {
        addAndMakeVisible(m_visualizer);
    }


    static InputModeMap default_modes(MovableComponent& c
                                      , Vec<std::unique_ptr<InputCondition>> conditions
                                      , bool add_fallback_mode) {
        InputModeMap map;
        map.add(std::move(conditions), std::make_unique<MoveMode>(c, c));
        if (add_fallback_mode)
            map.add(std::make_unique<NoKeyCondition>(), std::make_unique<DefaultMode>(c));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{std::make_unique<MoveMode::Visualization>()};
    }


    void mouseMove(const juce::MouseEvent&) override {}


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::orchid);
        g.setColour(juce::Colours::whitesmoke);
        g.drawFittedText(m_active_text, getLocalBounds(), juce::Justification::centred, 1);
    }


    void resized() override {
        m_visualizer.setBounds(getLocalBounds());
    }


    void set_text(const std::string& text) override {
        m_active_text = text;
        repaint();
    }


    void reset() override {
        set_text(m_default_text);
    }


private:
    std::string m_active_text;
    std::string m_default_text;

    InteractionVisualizer m_visualizer{*this, default_visualizations()};
    InputHandler m_input_handler;//{nullptr, *this, modes(*this), {std::ref(m_visualizer)}};
};


// ==============================================================================================

class DndBaseComponent : public juce::Component, public DummyTextInterface, public DropArea {
public:
    DndBaseComponent(const std::string& default_text
                     , InputModeMap input_mode_map
                     , Vec<std::unique_ptr<InteractionVisualization>> visualizations
                     , GlobalDragAndDropContainer& dnd_container
                     , const std::string& identifier
                     , const juce::Colour& bg_color
                     , InputHandler* parent = nullptr)
            : m_default_text(default_text)
              , m_text(default_text)
              , m_visualizer(*this, std::move(visualizations))
              , m_input_handler(parent, *this
                                , std::move(input_mode_map)
                                , dnd_container
                                , {std::ref(m_visualizer)}
                                , identifier)
              , m_identifier(identifier)
              , m_background(bg_color) {
        addAndMakeVisible(m_visualizer);
    }


    void resized() override {
        m_visualizer.setBounds(getLocalBounds());
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(m_background);
        g.setColour(juce::Colours::whitesmoke);
        g.drawFittedText(m_text, getLocalBounds(), juce::Justification::centred, 1);
    }


    DropListener& get_drop_listener() override {
        return m_input_handler;
    }


    void set_text(const std::string& text) override {
        m_text = text;
//        std::cout << m_identifier << "::text set to " << m_text << std::endl;
        repaint();
    }


    void reset() override {
        m_text = m_default_text;
//        std::cout << m_identifier << "::text set to " << m_default_text << " (reset)" << std::endl;
        repaint();
    }


    std::string get_identifier() const override {
        return m_identifier;
    }


    InputHandler& get_input_handler() {
        return m_input_handler;
    }


private:
    std::string m_default_text;
    std::string m_text;

    InteractionVisualizer m_visualizer;
    InputHandler m_input_handler;

    std::string m_identifier;

    juce::Colour m_background;

};

// ==============================================================================================


class DragAndDroppableComponent : public DndBaseComponent {
public:
    DragAndDroppableComponent(const std::string& default_text
                              , Vec<std::unique_ptr<InputCondition>> dnd_conditions
                              , GlobalDragAndDropContainer& dnd_container
                              , const std::string& identifier
                              , InputHandler* parent = nullptr)
            : DndBaseComponent(default_text
                               , default_modes(*this, std::move(dnd_conditions))
                               , default_visualizations()
                               , dnd_container
                               , identifier
                               , juce::Colours::lightskyblue
                               , parent) {}


    static InputModeMap default_modes(DragAndDroppableComponent& managed_component
                                      , Vec<std::unique_ptr<InputCondition>> conditions) {
        InputModeMap map;
        map.add(std::move(conditions), std::make_unique<DragAndDropMode>(managed_component));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{std::make_unique<DragAndDropMode::Visualization>()};
    }
};


// ==============================================================================================

class ThreeStateComponent : public DndBaseComponent {
public:
    ThreeStateComponent(const std::string& default_text
                        , Vec<std::unique_ptr<InputCondition>> move_conditions
                        , Vec<std::unique_ptr<InputCondition>> dnd_conditions
                        , GlobalDragAndDropContainer& dnd_container
                        , const std::string& identifier
                        , juce::Colour color = juce::Colours::darkseagreen
                        , InputHandler* parent = nullptr)
            : DndBaseComponent(default_text
                               , default_modes(*this, std::move(move_conditions), std::move(dnd_conditions))
                               , default_visualizations()
                               , dnd_container
                               , identifier
                               , color
                               , parent) {}


    static InputModeMap default_modes(ThreeStateComponent& c
                                      , Vec<std::unique_ptr<InputCondition>> move_conditions
                                      , Vec<std::unique_ptr<InputCondition>> dnd_conditions) {
        InputModeMap map;
        map.add(std::move(dnd_conditions), std::make_unique<DragAndDropMode>(c));
        map.add(std::move(move_conditions), std::make_unique<MoveMode>(c, c));
        map.add(std::make_unique<NoKeyCondition>(), std::make_unique<DefaultMode>(c));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{
                std::make_unique<DragAndDropMode::Visualization>()
                , std::make_unique<MoveMode::Visualization>()
        };
    }
};


// ==============================================================================================

class IncompatibleDndComponent : public DndBaseComponent {
public:
    IncompatibleDndComponent(const std::string& default_text
                             , Vec<std::unique_ptr<InputCondition>> dnd_conditions
                             , GlobalDragAndDropContainer& dnd_container
                             , const std::string& identifier
                             , InputHandler* parent = nullptr)
            : DndBaseComponent(default_text
                               , default_modes(*this, std::move(dnd_conditions))
                               , default_visualizations()
                               , dnd_container
                               , identifier
                               , juce::Colours::coral
                               , parent) {}


    static InputModeMap default_modes(DndBaseComponent& c
                                      , Vec<std::unique_ptr<InputCondition>> dnd_conditions) {
        InputModeMap map;
        map.add(std::move(dnd_conditions), std::make_unique<DragAndDropMode>(c));
        map.add(std::make_unique<NoKeyCondition>(), std::make_unique<DefaultMode>(c));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{
                std::make_unique<DragAndDropMode::Visualization>()
        };
    }


    int get_drag_type() const override {
        return 1;
    }
};

// ==============================================================================================

class NestedDndComponent : public ThreeStateComponent {
public:
    NestedDndComponent(const std::string& parent_text
                       , const std::string& child_text
                       , Vec<std::unique_ptr<InputCondition>> move_conditions
                       , Vec<std::unique_ptr<InputCondition>> dnd_conditions
                       , Vec<std::unique_ptr<InputCondition>> child_move_conditions
                       , Vec<std::unique_ptr<InputCondition>> child_dnd_conditions
                       , GlobalDragAndDropContainer& dnd_container
                       , const std::string& parent_identifier
                       , const std::string& child_identifier)
            : ThreeStateComponent(parent_text
                                  , std::move(move_conditions)
                                  , std::move(dnd_conditions)
                                  , dnd_container
                                  , parent_identifier)
              , m_child(child_text
                        , std::move(child_move_conditions)
                        , std::move(child_dnd_conditions)
                        , dnd_container
                        , child_identifier
                        , juce::Colours::darkkhaki
                        , &get_input_handler()) {
        addAndMakeVisible(m_child, 0);
    }


    void resized() override {
        m_child.setBounds(getLocalBounds().removeFromBottom(getHeight() / 4));
        ThreeStateComponent::resized();
    }


private:
    ThreeStateComponent m_child;
};


// ==============================================================================================

class DragEditableComponent : public juce::Component, public DummyTextInterface, public ValueInterface {
public:
    DragEditableComponent(const std::string& default_text
                          , Vec<std::unique_ptr<InputCondition>> move_conditions
                          , Vec<std::unique_ptr<InputCondition>> drag_edit_conditions
                          , DragBehaviour drag_behaviour
                          , GlobalDragAndDropContainer& dnd_container
                          , const std::string& identifier
                          , bool add_fallback_mode
                          , bool intercept_mouse
                          , InputHandler* parent = nullptr)
            : m_default_text(default_text)
              , m_input_handler(parent, *this
                                , default_modes(*this
                                                , std::move(move_conditions)
                                                , std::move(drag_edit_conditions)
                                                , drag_behaviour
                                                , add_fallback_mode
                                                , intercept_mouse)
                                , dnd_container
                                , {std::ref(m_visualizer)}
                                , identifier) {
        addAndMakeVisible(m_visualizer);
    }


    static InputModeMap default_modes(DragEditableComponent& c
                                      , Vec<std::unique_ptr<InputCondition>> move_conditions
                                      , Vec<std::unique_ptr<InputCondition>> drag_edit_conditions
                                      , DragBehaviour drag_behaviour
                                      , bool add_fallback_mode
                                      , bool intercept_mouse) {
        InputModeMap map;
        map.add(std::move(move_conditions), std::make_unique<MoveMode>(c, c));
        map.add(std::move(drag_edit_conditions), std::make_unique<DragEditMode>(c, c, drag_behaviour, intercept_mouse));
        if (add_fallback_mode)
            map.add(std::make_unique<NoKeyCondition>(), std::make_unique<DefaultMode>(c));
        return map;
    }


    static Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return Vec<std::unique_ptr<InteractionVisualization>>{
                std::make_unique<MoveMode::Visualization>()
                , std::make_unique<DragEditMode::Visualization>()
        };
    }


    void mouseMove(const juce::MouseEvent&) override {}


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::silver);
        g.setColour(juce::Colours::paleturquoise);
        g.fillRect(getLocalBounds().removeFromBottom(static_cast<int>(m_value)));

        g.setColour(juce::Colours::darkslategrey);
        g.drawFittedText(m_active_text, getLocalBounds(), juce::Justification::centred, 1);
    }


    void resized() override {
        m_visualizer.setBounds(getLocalBounds());
    }


    void set_text(const std::string& text) override {
        m_active_text = text;
        repaint();
    }


    void reset() override {
        set_text(m_default_text);
    }

    void set_value(float new_value) override {
        m_value = new_value;
        repaint();
    }


private:
    std::string m_active_text;
    std::string m_default_text;

    float m_value = 0.0f;

    InteractionVisualizer m_visualizer{*this, default_visualizations()};
    InputHandler m_input_handler;


};


// ==============================================================================================

class MultiNestedComponent : public DragAndDroppableComponent {
public:
    class InnerComponent : public ThreeStateComponent {
    public:
        InnerComponent(const std::string& parent_text
                       , const std::string& child_tsc_text
                       , const std::string& child_ade_text
                       , Vec<std::unique_ptr<InputCondition>> move_conditions
                       , Vec<std::unique_ptr<InputCondition>> dnd_conditions
                       , Vec<std::unique_ptr<InputCondition>> child_tsc_move_conditions
                       , Vec<std::unique_ptr<InputCondition>> child_tsc_dnd_conditions
                       , Vec<std::unique_ptr<InputCondition>> child_ade_drag_edit_conditions
                       , GlobalDragAndDropContainer& dnd_container
                       , const std::string& parent_identifier
                       , const std::string& child_tsc_identifier
                       , const std::string& child_ade_identifier
                       , bool child_ade_intercept
                       , InputHandler& parent)
                : ThreeStateComponent(parent_text
                                      , std::move(move_conditions)
                                      , std::move(dnd_conditions)
                                      , dnd_container
                                      , parent_identifier
                                      , juce::Colours::darkseagreen
                                      , &parent)
                  , m_tsc(child_tsc_text
                          , std::move(child_tsc_move_conditions)
                          , std::move(child_tsc_dnd_conditions)
                          , dnd_container
                          , child_tsc_identifier
                          , juce::Colours::darkkhaki
                          , &get_input_handler())
                  , m_ade(child_ade_text
                          , {}
                          , std::move(child_ade_drag_edit_conditions)
                          , DragBehaviour::hide_and_restore
                          , dnd_container
                          , child_ade_identifier
                          , true
                          , child_ade_intercept
                          , &get_input_handler()) {
            addAndMakeVisible(m_tsc, 0);
            addAndMakeVisible(m_ade, 0);
        }


        void resized() override {
            auto bounds = getLocalBounds().removeFromBottom(getHeight() / 3);
            m_tsc.setBounds(bounds.removeFromLeft(getLocalBounds().getWidth() / 2));
            m_ade.setBounds(bounds);
            ThreeStateComponent::resized();
        }

    private:
        ThreeStateComponent m_tsc;
        DragEditableComponent m_ade;
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    MultiNestedComponent(const std::string& parent_text
                         , const std::string& child_inner_text
                         , const std::string& child_ade_text
                         , Vec<std::unique_ptr<InputCondition>> dnd_conditions
                         , Vec<std::unique_ptr<InputCondition>> child_inner_move_conditions
                         , Vec<std::unique_ptr<InputCondition>> child_inner_dnd_conditions
                         , Vec<std::unique_ptr<InputCondition>> child_drag_edit_conditions
                         , GlobalDragAndDropContainer& dnd_container
                         , const std::string& parent_identifier
                         , const std::string& child_inner_identifier
                         , const std::string& child_ade_identifier
                         , const std::string& gc_tsc_text
                         , const std::string& gc_ade_text
                         , Vec<std::unique_ptr<InputCondition>> gc_tsc_move_conditions
                         , Vec<std::unique_ptr<InputCondition>> gc_tsc_dnd_conditions
                         , Vec<std::unique_ptr<InputCondition>> gc_ade_drag_edit_conditions
                         , const std::string& gc_tsc_identifier
                         , const std::string& gc_ade_identifier
                         , bool c_ade_intercept
                         , bool gc_ade_intercept)
            : DragAndDroppableComponent(
            parent_text
            , std::move(dnd_conditions)
            , dnd_container
            , parent_identifier)
              , m_inner(
                    child_inner_text
                    , gc_tsc_text
                    , gc_ade_text
                    , std::move(child_inner_move_conditions)
                    , std::move(child_inner_dnd_conditions)
                    , std::move(gc_tsc_move_conditions)
                    , std::move(gc_tsc_dnd_conditions)
                    , std::move(gc_ade_drag_edit_conditions)
                    , dnd_container
                    , child_inner_identifier
                    , gc_tsc_identifier
                    , gc_ade_identifier
                    , gc_ade_intercept
                    , get_input_handler()
            )
              , m_drag_edit(
                    child_ade_text
                    , {}
                    , std::move(child_drag_edit_conditions)
                    , DragBehaviour::hide_and_restore
                    , dnd_container
                    , child_ade_identifier
                    , false
                    , c_ade_intercept
                    , &get_input_handler()) {
        addAndMakeVisible(m_inner, 0);
        addAndMakeVisible(m_drag_edit, 0);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        bounds.removeFromRight(getWidth() / 4);
        bounds.removeFromTop(getWidth() / 4);

        m_inner.setBounds(bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.8)));
        m_drag_edit.setBounds(bounds);
        DragAndDroppableComponent::resized();
    }


private:
    InnerComponent m_inner;
    DragEditableComponent m_drag_edit;
};

// ==============================================================================================

#define MOVE_COMPONENTS 1
#define DND_COMPONENTS 1
#define INCOMPATIBLE_DND_COMPONENTS 1
#define TS_COMPONENTS 1
#define NESTED_COMPONENTS 1
#define RAW_DRAG_EDIT_COMPONENT 1
#define ALWAYS_DNDABLE_COMPONENT 1
#define MULTI_NESTED_COMPONENT 1

class StatePlaygroundComponent : public juce::Component {
public:
    StatePlaygroundComponent()
            : m_dnd_container(*this)
#if MOVE_COMPONENTS
            , m_always_movable_component(
                    "Always Movable"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<AlwaysTrueCondition>()}
                    , m_dnd_container
                    , "M1"
                    , false
            )
              , m_conditioned_movable_component(
                    "Movable (Q)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , m_dnd_container
                    , "M2"
                    , true
            )
#endif
#if DND_COMPONENTS
            , m_dnd_component1(
                    "Drag & Drop (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "D1"
            )
              , m_dnd_component2(
                    "Drag & Drop (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "D2"
            )
#endif
#if TS_COMPONENTS
            , m_three_state_component(
                    "Move (Q) / DnD (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "TS1"
            )
#endif
#if INCOMPATIBLE_DND_COMPONENTS
            , m_incompatible_dnd_component1(
                    "DnD Category 2 (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "IC1"
            )
              , m_incompatible_dnd_component2(
                    "DnD Category 2 (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "IC2"
            )
#endif
#if NESTED_COMPONENTS
            , m_nested_component(
                    "DnD Parent (W) / Move (Q)"
                    , "Child (W) / (E)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('E')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , m_dnd_container
                    , "PAR1"
                    , "CH1"
            )
#endif
#if RAW_DRAG_EDIT_COMPONENT
            , m_drag_edit_component1(
                    "Drag Edit (R) / Drag (Q)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('R')}
                    , DragBehaviour::drag_edit
                    , m_dnd_container
                    , "DE1"
                    , true
                    , true
            )
              , m_drag_edit_component2(
                    "Hide&Restore (R)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('R')}
                    , DragBehaviour::hide_and_restore
                    , m_dnd_container
                    , "DE2"
                    , true
                    , true
            )
              , m_drag_edit_component3(
                    "Always Drag Editable"
                    , Vec<std::unique_ptr<InputCondition>>{}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<AlwaysTrueCondition>()}
                    , DragBehaviour::hide_and_restore
                    , m_dnd_container
                    , "DE3"
                    , false
                    , true
            )
#endif
#if ALWAYS_DNDABLE_COMPONENT
            , m_always_dndable_component1(
                    "Always DnD:able"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<AlwaysTrueCondition>()}
                    , m_dnd_container
                    , "ADND1"
            )
              , m_always_dndable_component2(
                    "Always DnD:able"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<AlwaysTrueCondition>()}
                    , m_dnd_container
                    , "ADND2"
            )
#endif
#if MULTI_NESTED_COMPONENT
            , m_multi_nested_component(
                    "DnD"
                    , "DnD/Drag"
                    , "Drag Edit (Always)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<AlwaysTrueCondition>()}
                    , m_dnd_container
                    , "Mp"
                    , "Mc1"
                    , "Mc2"
                    , "TS"
                    , "DE (W)"
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
                    , "Mgc1"
                    , "Mgc2"
                    , false
                    , true
            )
#endif
//              , m_component_with_slider(
//                    "W/Q with Slider"
//                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('Q')}
//                    , Vec<std::unique_ptr<InputCondition>>{std::make_unique<KeyCondition>('W')}
//                    , m_dnd_container
//                    , "SLI1"
//            )
    {
#if MOVE_COMPONENTS
        addAndMakeVisible(m_always_movable_component);
        addAndMakeVisible(m_conditioned_movable_component);
#endif
#if DND_COMPONENTS
        addAndMakeVisible(m_dnd_component1);
        addAndMakeVisible(m_dnd_component2);
#endif
#if TS_COMPONENTS
        addAndMakeVisible(m_three_state_component);
#endif
#if INCOMPATIBLE_DND_COMPONENTS
        addAndMakeVisible(m_incompatible_dnd_component1);
        addAndMakeVisible(m_incompatible_dnd_component2);
#endif
#if NESTED_COMPONENTS
        addAndMakeVisible(m_nested_component);
#endif
#if RAW_DRAG_EDIT_COMPONENT
        addAndMakeVisible(m_drag_edit_component1);
        addAndMakeVisible(m_drag_edit_component2);
        addAndMakeVisible(m_drag_edit_component3);
#endif
#if ALWAYS_DNDABLE_COMPONENT
        addAndMakeVisible(m_always_dndable_component1);
        addAndMakeVisible(m_always_dndable_component2);
#endif
#if MULTI_NESTED_COMPONENT
        addAndMakeVisible(m_multi_nested_component);
#endif

//        addAndMakeVisible(m_component_with_slider);

        addAndMakeVisible(m_dnd_container);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::mediumpurple);
    }


    void resized() override {
        auto bounds = getLocalBounds().reduced(50);

        auto col = bounds.removeFromLeft(100);
#if MOVE_COMPONENTS
        m_always_movable_component.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
        m_conditioned_movable_component.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
#endif
#if DND_COMPONENTS
        m_dnd_component1.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
        m_dnd_component2.setBounds(col.removeFromTop(100));
#endif
        bounds.removeFromLeft(50);

        col = bounds.removeFromLeft(100);

#if TS_COMPONENTS
        m_three_state_component.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
#endif
#if NESTED_COMPONENTS
        m_nested_component.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
#endif
#if INCOMPATIBLE_DND_COMPONENTS
        m_incompatible_dnd_component1.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
        m_incompatible_dnd_component2.setBounds(col.removeFromTop(100));
#endif
        bounds.removeFromLeft(50);

        col = bounds.removeFromLeft(100);
#if RAW_DRAG_EDIT_COMPONENT
        m_drag_edit_component1.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
        m_drag_edit_component2.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
        m_drag_edit_component3.setBounds(col.removeFromTop(100));
#endif
        bounds.removeFromLeft(50);

        col = bounds.removeFromLeft(100);

#if ALWAYS_DNDABLE_COMPONENT
        m_always_dndable_component1.setBounds(col.removeFromTop(100));
        col.removeFromTop(50);
        m_always_dndable_component2.setBounds(col.removeFromTop(100));
#endif
#if MULTI_NESTED_COMPONENT
        bounds.removeFromLeft(50);

        col = bounds.removeFromLeft(200);
        m_multi_nested_component.setBounds(col.removeFromTop(200));
#endif

        m_dnd_container.setBounds(getLocalBounds());
    }


//    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails &) override {
//        std::cout << "drag operation endede\n";
//    }


private:
    GlobalDragAndDropContainer m_dnd_container;

#if MOVE_COMPONENTS
    MovableComponent m_always_movable_component;
    MovableComponent m_conditioned_movable_component;
#endif

    // TODO: This should have an InputHandler and pass it to its children
    //  (verify that no-state input handler works as intended)

#if DND_COMPONENTS
    DragAndDroppableComponent m_dnd_component1;
    DragAndDroppableComponent m_dnd_component2;
#endif

#if TS_COMPONENTS
    ThreeStateComponent m_three_state_component;
#endif

#if INCOMPATIBLE_DND_COMPONENTS
    IncompatibleDndComponent m_incompatible_dnd_component1;
    IncompatibleDndComponent m_incompatible_dnd_component2;
#endif

#if NESTED_COMPONENTS
    NestedDndComponent m_nested_component;
#endif

#if RAW_DRAG_EDIT_COMPONENT
    DragEditableComponent m_drag_edit_component1;
    DragEditableComponent m_drag_edit_component2;
    DragEditableComponent m_drag_edit_component3;
#endif
#if ALWAYS_DNDABLE_COMPONENT
    DragAndDroppableComponent m_always_dndable_component1;
    DragAndDroppableComponent m_always_dndable_component2;
#endif
#if MULTI_NESTED_COMPONENT
    MultiNestedComponent m_multi_nested_component;
#endif

//    DndComponentWithSlider m_component_with_slider;
};


// ==============================================================================================

class MainComponent : public MainKeyboardFocusComponent
                      , private juce::HighResolutionTimer
                      , private juce::ValueTree::Listener {
public:

    unsigned int ROOT_ID = 1;


    MainComponent()
            : m_some_handler(m_undo_manager) {

        addAndMakeVisible(m_playground);

        setSize(900, 600);
    }

//    static Vec<TriggerableState> states() {
//        std::unique_ptr<Condition> cond = std::make_unique<KeyCondition>('q');
//        TriggerableState{std::make_unique<KeyCondition>('q')
//                      , State{ModuleIds::ConfigurationLayerComponent, StateIds::Configuration::Move}}
//    }


    void resized() override {
        m_playground.setBounds(getLocalBounds());
    }


    void hiResTimerCallback() override {
        callback_count += 1;
        if (callback_count % 500 == 0) {
        }
    }


    void globalFocusChanged(juce::Component* focusedComponent) override {
        if (focusedComponent) {
//            std::cout << "focused component dims " << focusedComponent->getWidth() << " "
//                      << focusedComponent->getHeight() << "\n";
        } else {
//            std::cout << "nullptr\n";
        }
        MainKeyboardFocusComponent::globalFocusChanged(focusedComponent);
    }


    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {
//        std::cout << m_some_handler.get_value_tree().toXmlString() << "\n";
    }


private:
    std::unique_ptr<juce::LookAndFeel> m_lnf;
    juce::UndoManager m_undo_manager;
    ParameterHandler m_some_handler;

    StatePlaygroundComponent m_playground;


    long callback_count = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


// ==============================================================================================

class StatePlayground : public juce::JUCEApplication {
public:

    StatePlayground() = default;


    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }


    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }


    bool moreThanOneInstanceAllowed() override { return true; }


    void initialise(const juce::String& commandLine) override {
        juce::ignoreUnused(commandLine);

        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }


    void shutdown() override {
        mainWindow = nullptr;
    }


    void systemRequestedQuit() override {
        quit();
    }


    void anotherInstanceStarted(const juce::String& commandLine) override {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused(commandLine);
    }


    class MainWindow : public juce::DocumentWindow {
    public:
        explicit MainWindow(const juce::String& name)
                : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }


        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }


    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;

};

START_JUCE_APPLICATION (StatePlayground)

