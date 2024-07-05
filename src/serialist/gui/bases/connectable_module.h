
#ifndef SERIALISTLOOPER_CONNECTABLE_MODULE_H
#define SERIALISTLOOPER_CONNECTABLE_MODULE_H

#include <juce_gui_basics/juce_gui_basics.h>

namespace serialist {

class ConnectableModule {
public:
    ConnectableModule() = default;

    virtual ~ConnectableModule() = default;

    ConnectableModule(const ConnectableModule&) = delete;

    ConnectableModule& operator=(const ConnectableModule&) = delete;

    ConnectableModule(ConnectableModule&&) noexcept = default;

    ConnectableModule& operator=(ConnectableModule&&) noexcept = default;

    virtual bool connectable_to(juce::Component& component) = 0;

    virtual bool connect(ConnectableModule& connectable) = 0;

};

} // namespace serialist




#endif //SERIALISTLOOPER_CONNECTABLE_MODULE_H
