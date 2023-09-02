
#ifndef SERIALISTLOOPER_PULSATOR_MODULE_H
#define SERIALISTLOOPER_PULSATOR_MODULE_H

#include "generative_component.h"
#include "connectable_module.h"
#include "slider_widget.h"

class PulsatorModule : public GenerativeComponent
, public juce::DragAndDropTarget
, public ConnectableModule {
public:

    using SliderLayout = SliderWidget::Layout;

    enum class Layout  {
        full
        , note
    };

    // TODO: Continue
};

#endif //SERIALISTLOOPER_PULSATOR_MODULE_H
