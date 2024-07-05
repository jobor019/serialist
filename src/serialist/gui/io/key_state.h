

#ifndef GUIUTILS_KEY_STATE_H
#define GUIUTILS_KEY_STATE_H

#include <juce_gui_extra/juce_gui_extra.h>

namespace serialist {

class KeyState {
public:

    // TODO: Doesn't handle command + keys very well

    /**
     * @return true if state changed, otherwise false
     */
    bool register_key_press(const juce::KeyPress& k) {
        if (std::find(m_held_keys.begin(), m_held_keys.end(), k.getKeyCode()) == m_held_keys.end()) {
            m_held_keys.push_back(k.getKeyCode());
            m_modifiers = k.getModifiers();
            return true;
        }

        return false;
    }


    /**
 * @return true if state changed, otherwise false
 */
    bool register_key_release() {
        auto new_end = std::remove_if(m_held_keys.begin()
                                      , m_held_keys.end()
                                      , [](int k) { return !juce::KeyPress::isKeyCurrentlyDown(k); });

        bool has_changed = new_end != m_held_keys.end();

        m_held_keys.erase(new_end, m_held_keys.end());

        return has_changed;
    }


    /**
     * @return true if state changed, otherwise false
     */
    bool register_modifiers(const juce::ModifierKeys& m) {
        bool has_changed = m.getRawFlags() != m_modifiers.getRawFlags();
        m_modifiers = m;
        return has_changed;
    }


    void print_held() {
        std::cout << "Held keys: ";
        for (auto& key: m_held_keys) {
            std::cout << key << " ";
        }
        std::cout << "(modifiers: " << m_modifiers.getRawFlags() << ")\n";
    }


    const juce::ModifierKeys& get_modifiers() const {
        return m_modifiers;
    }


    const std::vector<int>& get_held_keys() const {
        return m_held_keys;
    }


    bool has_held_keys() {
        return !m_held_keys.empty();
    }


    bool is_down(int key_code) {
        return std::find(m_held_keys.begin(), m_held_keys.end(), key_code) != m_held_keys.end();
    }


    bool is_down_exclusive(int key_code) {
        return is_down(key_code) && m_held_keys.size() == 1;
    }

    bool flush() {
        m_held_keys.clear();
        m_modifiers = juce::ModifierKeys();
        return true;
    }


private:
    std::vector<int> m_held_keys;
    juce::ModifierKeys m_modifiers;

};


// ==============================================================================================

/**
 * Note: Not thread-safe! Assumes all key operations are handled on the same thread!!
 */
class GlobalKeyState {
public:

    class Listener {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

    private:
        virtual void modifier_keys_changed() {}
        virtual void key_pressed() {}
        virtual void key_released() {}

        friend class GlobalKeyState;
    };


    static GlobalKeyState& get_instance() {
        static GlobalKeyState instance;
        return instance;
    }


    ~GlobalKeyState() = default;

    GlobalKeyState(GlobalKeyState const&) = delete;

    void operator=(GlobalKeyState const&) = delete;

    GlobalKeyState(GlobalKeyState&&) noexcept = delete;

    GlobalKeyState& operator=(GlobalKeyState&&) noexcept = delete;


    static void add_listener(Listener& listener) {
        auto& self = get_instance();
        if (std::find(self.m_listeners.begin(), self.m_listeners.end(), &listener) == self.m_listeners.end()) {
            self.m_listeners.push_back(&listener);
        }
    }


    static void remove_listener(Listener& listener) {
        auto& self = get_instance();
        self.m_listeners.erase(std::remove(self.m_listeners.begin(), self.m_listeners.end(), &listener)
                               , self.m_listeners.end());
    }


    static bool register_key_press(const juce::KeyPress& k) {
        auto& self = get_instance();
        bool changed = self.m_key_state.register_key_press(k);

        if (changed) {
            for (auto& listener: self.m_listeners) {
                listener->key_pressed();
            }
        }
        return changed;
    }


    static bool register_key_release() {
        auto& self = get_instance();
        bool changed = self.m_key_state.register_key_release();

        if (changed) {
            for (auto& listener: self.m_listeners) {
                listener->key_released();
            }
        }
        return changed;
    }


    static bool register_modifiers(const juce::ModifierKeys& m) {
        auto& self = get_instance();
        bool changed = self.m_key_state.register_modifiers(m);

        if (changed) {
            for (auto& listener: self.m_listeners) {
                listener->modifier_keys_changed();
            }
        }
        return changed;
    }

    static void flush() {
        bool modifiers_changed = GlobalKeyState::get_modifiers().getRawFlags() != 0;
        bool key_state_changed = !GlobalKeyState::get_held_keys().empty();

        auto& self = get_instance();
        self.m_key_state.flush();

        if (modifiers_changed) {
            for (auto& listener: self.m_listeners) {
                listener->modifier_keys_changed();
            }
        }

        if (key_state_changed) {
            for (auto& listener: self.m_listeners) {
                listener->key_released();
            }
        }
    }


    static void print_held() { return get_instance().m_key_state.print_held(); }


    static const juce::ModifierKeys& get_modifiers() { return get_instance().m_key_state.get_modifiers(); }


    static const std::vector<int>& get_held_keys() { return get_instance().m_key_state.get_held_keys(); }


    static bool has_held_keys() { return get_instance().m_key_state.has_held_keys(); }


    static bool is_down(int key_code) { return get_instance().m_key_state.is_down(key_code); }


    static bool is_down_exclusive(int key_code) { return get_instance().m_key_state.is_down_exclusive(key_code); }

    template<typename... Args>
    static bool any_is_down_exclusive(Args... args) {
        if constexpr (sizeof...(args) > 0) {
            return (is_down_exclusive(args) || ...);
        }

        return false;
    }


private:
    GlobalKeyState() = default;

    KeyState m_key_state;
    std::vector<Listener*> m_listeners;
};


// ==============================================================================================

/**
 * Component that registers all key presses as long as it's focused, and automatically grabs focus if focus is dropped
 *  (e.g. if a Label is focused and then focus of that label is lost)
 *
 *  Note that for this to work properly, all components should use `setWantsKeyboardFocus(false)` and use
 *   GlobalKeyState::Listener for detecting keypressed, rather than juce::Component::keyPressed, etc.,
 *   unless they explicitly need keyboard focus in a stateful manner that should block all other keyboard-related
 *   events (e.g. when editing a text editor)
 */
class MainKeyboardFocusComponent : public juce::Component
                                   , public GlobalKeyState::Listener
                                   , public juce::FocusChangeListener
                                   , public juce::Timer {
public:
    MainKeyboardFocusComponent() {
        setWantsKeyboardFocus(true);
        GlobalKeyState::add_listener(*this);
        juce::Desktop::getInstance().addFocusChangeListener(this);
        startTimer(1000);
    }


    ~MainKeyboardFocusComponent() override {
        GlobalKeyState::remove_listener(*this);
        juce::Desktop::getInstance().removeFocusChangeListener(this);
    }


protected:
    void globalFocusChanged(juce::Component* focusedComponent) override {
        if (focusedComponent == nullptr) {
            grabKeyboardFocus();
        }
    }

    void timerCallback() override {
        if (!juce::Process::isForegroundProcess()) {
            GlobalKeyState::flush();
        }
    }


private:
    bool keyPressed(const juce::KeyPress& key) final {
        GlobalKeyState::register_key_press(key);
        return true;
    }


    bool keyStateChanged(bool isKeyDown) final {
        if (!isKeyDown) {
            GlobalKeyState::register_key_release();
        }
        return true;
    }


    void modifierKeysChanged(const juce::ModifierKeys& modifiers) final {
        GlobalKeyState::register_modifiers(modifiers);
    }
};


// ==============================================================================================


/**
 *  Base class (non-virtual) for modal windows (Popup menus, dialogs, etc.) that should update the global key state
 *  when they are focused. Example usage:
 *
 *  @code
 *    class MyModalWindow : public juce::Component
 *                        , public GlobalKeyPressRegistrar {
 *    public:
 *        MyWindow() {
 *            addKeyListener(this);
 *        }
 *
 *   @endcode
 */
class GlobalKeyPressRegistrar : public juce::KeyListener {
private:
    bool keyPressed(const juce::KeyPress &key, juce::Component*) final {
        GlobalKeyState::register_key_press(key);
        return true;
    }


    bool keyStateChanged(bool isKeyDown, juce::Component*) final {
        // Since juce::KeyListener doesn't implement modifier keys changed,
        // we need to check the modifiers each time we get a key state change.
        //
        // In addition, since (at least) juce::NSViewComponentPeer::redirectModKeyChange
        // updates the global modifiers AFTER registering the key press, we need to use
        // juce::ComponentPeer::getCurrentModifiersRealtime() here
        // as juce::ModifierKeys::currentModifiers hasn't been updated yet

        GlobalKeyState::register_modifiers(juce::ComponentPeer::getCurrentModifiersRealtime());

        if (!isKeyDown) {
            GlobalKeyState::register_key_release();
        }
        return true;
    }
};

} // namespace serialist

#endif //GUIUTILS_KEY_STATE_H
