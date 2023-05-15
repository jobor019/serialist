

#ifndef SERIALISTLOOPER_NODE_COMPONENT_H
#define SERIALISTLOOPER_NODE_COMPONENT_H

#include "generative.h"
#include <juce_gui_extra/juce_gui_extra.h>


class NodeComponent : public juce::Component {
public:
    virtual Generative& get_generative() = 0;

    NodeComponent() = default;
    NodeComponent(const NodeComponent&) = delete;
    NodeComponent& operator=(const NodeComponent&) = delete;
    NodeComponent(NodeComponent&&)  noexcept = delete;
    NodeComponent& operator=(NodeComponent&&)  noexcept = delete;


};

#endif //SERIALISTLOOPER_NODE_COMPONENT_H
