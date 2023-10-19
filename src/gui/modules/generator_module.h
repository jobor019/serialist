

#ifndef SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
#define SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H

#include "interaction_visualizations.h"
#include "generative_component.h"
#include "core/param/parameter_policy.h"
#include "core/generatives/generator.h"
#include "oscillator_module.h"
#include "text_sequence_module.h"
#include "interpolation_module.h"
#include "module_bases.h"


template<typename OutputType, typename InternalSequenceType = OutputType>
class GeneratorModule : public NodeBase<OutputType> {
public:
    enum class Layout {
        full
    };


    GeneratorModule(Generator<OutputType>& generator
                    , std::unique_ptr<OscillatorModule> oscillator
                    , std::unique_ptr<InterpolationModule> interpolator
                    , std::unique_ptr<TextSequenceModule<OutputType, InternalSequenceType>> sequence
                    , Variable<Facet, bool>& internal_enabled
                    , Variable<Facet, float>& internal_num_voices
                    , Layout layout = Layout::full)
            : NodeBase<OutputType>(generator, &internal_enabled, &internal_num_voices)
              , m_oscillator_socket(generator.get_cursor(), std::move(oscillator))
              , m_interpolator(generator.get_interpolation_strategy(), std::move(interpolator))
              , m_sequence_socket(generator.get_sequence(), std::move(sequence))
              , m_layout(layout) {
        NodeBase<OutputType>::addAndMakeVisible(m_oscillator_socket);
        NodeBase<OutputType>::addAndMakeVisible(m_interpolator);
        NodeBase<OutputType>::addAndMakeVisible(m_sequence_socket);

    }

    static int width_of(Layout layout = Layout::full) {
        (void) layout;
        return OscillatorModule::width_of(OscillatorModule::Layout::generator_internal);
    }


    static int height_of(Layout layout = Layout::full) {
        (void) layout;
        return HeaderWidget::height_of()
               + 2 * DC::COMPONENT_UD_MARGINS
               + OscillatorModule::height_of(OscillatorModule::Layout::generator_internal)
               + DC::DEFAULT_SEQUENCE_HEIGHT
               + InterpolationModule::height_of(InterpolationModule::Layout::generator_internal)
               + 2 * DC::OBJECT_Y_MARGINS_COLUMN;
    }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        NodeBase<OutputType>::resized();
    }


private:
    void on_resized(juce::Rectangle<int>& bounds) override {
        if (m_layout == Layout::full) {
            full_layout(bounds);
        }
    }

    void full_layout(juce::Rectangle<int> bounds) {
        auto oscillator_layout = OscillatorModule::Layout::generator_internal;
        auto sequence_layout = TextSequenceModule<OutputType>::Layout::generator_internal;
        auto interpolator_layout = InterpolationModule::Layout::generator_internal;

        m_oscillator_socket.set_layout(static_cast<int>(oscillator_layout));
        m_sequence_socket.set_layout(static_cast<int>(sequence_layout));
        m_interpolator.set_layout(static_cast<int>(interpolator_layout));

        m_oscillator_socket.setBounds(bounds.removeFromTop(OscillatorModule::height_of(oscillator_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_sequence_socket.setBounds(bounds.removeFromTop(TextSequenceModule<OutputType>::height_of(sequence_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_interpolator.setBounds(bounds.removeFromTop(InterpolationModule::height_of(interpolator_layout)));

    }


    SocketWidget<Facet> m_oscillator_socket;
    SocketWidget<InterpolationStrategy> m_interpolator;
    DataSocketWidget<OutputType> m_sequence_socket;

    Layout m_layout = Layout::full;
};

#endif //SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
