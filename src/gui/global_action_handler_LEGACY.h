
#ifndef JUCEGUIPLAYGROUND_GLOBAL_ACTION_HANDLER_H
#define JUCEGUIPLAYGROUND_GLOBAL_ACTION_HANDLER_H


#include <juce_gui_basics/juce_gui_basics.h>


class Action {
public:
    Action(int action_id, juce::Component& source) : m_action_id(action_id), m_source(source) {}


    juce::Component& get_source() const { return m_source; }


    int get_action_id() const { return m_action_id; }


private:
    int m_action_id;
    juce::Component& m_source;

};


// ==============================================================================================

class GlobalActionHandler {
public:

    class Listener {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void on_action_change(Action*) = 0;
    };


    static GlobalActionHandler& get_instance() {
        static GlobalActionHandler instance;
        return instance;
    }


    ~GlobalActionHandler() = default;
    GlobalActionHandler(GlobalActionHandler const&) = delete;
    void operator=(GlobalActionHandler const&) = delete;
    GlobalActionHandler(GlobalActionHandler&&) noexcept = delete;
    GlobalActionHandler& operator=(GlobalActionHandler&&) noexcept = delete;


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


    static void register_action(std::unique_ptr<Action> action) {
        auto& self = get_instance();
        self.m_ongoing_action = std::move(action);
        self.notify();
    }


    static void terminate_ongoing_action() {
        auto& self = get_instance();
        if (self.m_ongoing_action) {
            self.m_ongoing_action = nullptr;
            self.notify();
        }
    }


private:
    GlobalActionHandler() = default;


    void notify() {
        for (auto& listener: m_listeners) {
            if (listener) {
                listener->on_action_change(m_ongoing_action.get());
            }
        }
    }


    std::vector<Listener*> m_listeners;

    std::unique_ptr<Action> m_ongoing_action = nullptr;


};


#endif //JUCEGUIPLAYGROUND_GLOBAL_ACTION_HANDLER_H
