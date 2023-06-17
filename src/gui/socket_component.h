

#ifndef SERIALISTLOOPER_SOCKET_COMPONENT_H
#define SERIALISTLOOPER_SOCKET_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "socket_policy.h"
#include "generative_component.h"

template<typename T>
class SocketComponent : public juce::Component
                        , private juce::ValueTree::Listener {
public:
    explicit SocketComponent(Socket<T>& socket, std::unique_ptr<GenerativeComponent> default_component)
            : m_socket(socket)
              , m_default_component(std::move(default_component)) {
        if (!m_default_component) {
            throw std::runtime_error("A default component must be provided for the socket");
        }

        m_socket.add_value_tree_listener(*this);
        addAndMakeVisible(*m_default_component);
    }


    ~SocketComponent() override {
        m_socket.remove_value_tree_listener(*this);
    }


    SocketComponent(const SocketComponent&) = delete;
    SocketComponent& operator=(const SocketComponent&) = delete;
    SocketComponent(SocketComponent&&) noexcept = default;
    SocketComponent& operator=(SocketComponent&&) noexcept = default;


    Socket<T>& get_socket() {
        return m_socket;
    }


    GenerativeComponent& get_internal() {
        return *m_default_component;
    }

    void set_layout(int layout) {
        get_internal().set_layout(layout);
    }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_default_component->setBounds(getLocalBounds());
    }


private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {
        std::cout << "[TODO] VT changed in SocketComponent\n";
    }


    Socket<T>& m_socket;

    std::unique_ptr<GenerativeComponent> m_default_component;

};

#endif //SERIALISTLOOPER_SOCKET_COMPONENT_H
