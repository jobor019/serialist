

#ifndef SERIALISTLOOPER_SOCKET_WIDGET_H
#define SERIALISTLOOPER_SOCKET_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "socket_policy.h"
#include "generative_component.h"
#include "keyboard_shortcuts.h"
#include "connectable.h"

template<typename T>
class SocketWidget : public juce::Component
                     , private juce::ValueTree::Listener {
public:
    explicit SocketWidget(Socket<T>& socket, std::unique_ptr<GenerativeComponent> default_widget)
            : m_socket(socket)
              , m_default_widget(std::move(default_widget)) {

        if (!m_default_widget) {
            throw std::runtime_error("A default component must be provided for the socket");
        }

        m_socket.add_value_tree_listener(*this);
        addAndMakeVisible(*m_default_widget);

        m_default_widget->addMouseListener(this, true);
    }


    ~SocketWidget() override {
        m_socket.remove_value_tree_listener(*this);
    }


    SocketWidget(const SocketWidget&) = delete;
    SocketWidget& operator=(const SocketWidget&) = delete;
    SocketWidget(SocketWidget&&) noexcept = default;
    SocketWidget& operator=(SocketWidget&&) noexcept = default;


    Socket<T>& get_socket() {
        return m_socket;
    }


    GenerativeComponent& get_internal() {
        return *m_default_widget;
    }


//    bool connectable_to(GenerativeComponent& generative_component) override {
//        return m_socket.is_connectable(generative_component.get_generative());
//    }


//    bool on_connect(GenerativeComponent& generative_component) override {
//        return m_socket.try_connect(generative_component.get_generative());
//    }


    void set_layout(int layout) {
        get_internal().set_layout(layout);
    }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_default_widget->setBounds(getLocalBounds());
//        Connectable::resized();
    }


private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {
        std::cout << "[TODO] VT changed in SocketWidget\n";
    }


    Socket<T>& m_socket;

    std::unique_ptr<GenerativeComponent> m_default_widget;

};

#endif //SERIALISTLOOPER_SOCKET_WIDGET_H
