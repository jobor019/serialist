

#ifndef SERIALISTLOOPER_NODE_COMPONENT_H
#define SERIALISTLOOPER_NODE_COMPONENT_H

#include "generative.h"
#include <juce_gui_extra/juce_gui_extra.h>


class NodeComponent : public juce::Component {
public:
    virtual Generative& get_generative() = 0;


};

#endif //SERIALISTLOOPER_NODE_COMPONENT_H
