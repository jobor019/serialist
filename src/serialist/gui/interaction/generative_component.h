

#ifndef SERIALISTLOOPER_GENERATIVE_COMPONENT_H
#define SERIALISTLOOPER_GENERATIVE_COMPONENT_H

#include "core/generative.h"
#include "gui/look_and_feel.h"
#include <juce_gui_extra/juce_gui_extra.h>


class DimensionConstants {
public:
    static inline const int SLIDER_DEFAULT_HEIGHT = 14;
    static inline const int SLIDER_DEFAULT_WIDTH = 50;
    static inline const int SLIDER_COMPACT_WIDTH = 40;
    static inline const int FONT_HEIGHT = SerialistLookAndFeel::FONT_SIZE;
    static inline const int DEFAULT_LABEL_WIDTH = 40;

    static inline const int DEFAULT_SEQUENCE_HEIGHT = 46;

    static inline const int COMPONENT_LR_MARGINS = 5;
    static inline const int COMPONENT_UD_MARGINS = 8;

    static inline const int COMPONENT_HEADER_HEIGHT = 20;
    static inline const int HEADER_INTERNAL_MARGINS = 2;

    static inline const int OBJECT_X_MARGINS_COLUMN = 5;
    static inline const int OBJECT_Y_MARGINS_COLUMN = 4;
    static inline const int OBJECT_X_MARGINS_ROW = 5;
    static inline const int OBJECT_Y_MARGINS_ROW = 5;

    static inline const int LABEL_BELOW_MARGINS = 2;

    DimensionConstants() = delete;
};

using DC = DimensionConstants;

// ==============================================================================================

class GenerativeComponent : public juce::Component {
public:
    virtual Generative& get_generative() = 0;

    virtual void set_layout(int layout_id) = 0;

    GenerativeComponent() = default;
    ~GenerativeComponent() override = default;
    GenerativeComponent(const GenerativeComponent&) = delete;
    GenerativeComponent& operator=(const GenerativeComponent&) = delete;
    GenerativeComponent(GenerativeComponent&&) noexcept = delete;
    GenerativeComponent& operator=(GenerativeComponent&&) noexcept = delete;


};

#endif //SERIALISTLOOPER_GENERATIVE_COMPONENT_H
