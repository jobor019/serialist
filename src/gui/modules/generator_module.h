

#ifndef SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
#define SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H

#include "base_module.h"
#include "generative_component.h"
#include "parameter_policy.h"
#include "generator.h"
#include "oscillator_module.h"
#include "text_sequence_module.h"
#include "interpolation_module.h"


template<typename T>
class GeneratorModule : public BaseModule {
public:
    enum class Layout {
        full
    };


    GeneratorModule(Generator<T>& generator
                    , std::unique_ptr<OscillatorModule> oscillator
                    , std::unique_ptr<InterpolationModule<T>> interpolator
                    , std::unique_ptr<TextSequenceModule<T>> sequence
                    , Variable<bool>& internal_enabled
                    , Layout layout = Layout::full)
            : m_generator(generator)
              , m_internal_oscillator(std::move(oscillator))
              , m_interpolator(std::move(interpolator))
              , m_internal_sequence(std::move(sequence))
              , m_header(generator.get_identifier_as_string(), internal_enabled) {
        (void) layout;

        if (!m_internal_oscillator || !m_interpolator || !m_internal_sequence)
            throw std::runtime_error("A GeneratorModule requires all internal modules to be initialized");

        addAndMakeVisible(m_internal_oscillator.get());
        addAndMakeVisible(m_interpolator.get());
        addAndMakeVisible(m_internal_sequence.get());

        addAndMakeVisible(m_header);
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
               + TextSequenceModule<T>::height_of(TextSequenceModule<T>::Layout::generator_internal)
               + InterpolationModule<T>::height_of(InterpolationModule<T>::Layout::generator_internal)
               + 2 * DC::OBJECT_Y_MARGINS_COLUMN;
    }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        resized();
    }


    Generative& get_generative() override {
        return m_generator;
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 2);
    }


    void resized() override {
        if (m_layout == Layout::full) {
            full_layout();
            BaseModule::resized();
        }
    }


private:
    void full_layout() {
        // layout
        auto oscillator_layout = OscillatorModule::Layout::generator_internal;
        auto sequence_layout = TextSequenceModule<T>::Layout::generator_internal;
        auto interpolator_layout = InterpolationModule<T>::Layout::generator_internal;

        m_internal_oscillator->set_layout(static_cast<int>(oscillator_layout));
        m_internal_sequence->set_layout(static_cast<int>(sequence_layout));
        m_interpolator->set_layout(static_cast<int>(interpolator_layout));

        auto bounds = getLocalBounds();

        m_header.setBounds(bounds.removeFromTop(HeaderWidget::height_of()));
        bounds.reduce(DC::COMPONENT_LR_MARGINS, DC::COMPONENT_UD_MARGINS);

        m_internal_oscillator->setBounds(bounds.removeFromTop(OscillatorModule::height_of(oscillator_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_internal_sequence->setBounds(bounds.removeFromTop(TextSequenceModule<T>::height_of(sequence_layout)));
        bounds.removeFromTop(DC::OBJECT_Y_MARGINS_COLUMN);
        m_interpolator->setBounds(bounds.removeFromTop(InterpolationModule<T>::height_of(interpolator_layout)));
    }


    Generator<T>& m_generator;

    std::unique_ptr<OscillatorModule> m_internal_oscillator;
    std::unique_ptr<InterpolationModule<T>> m_interpolator;
    std::unique_ptr<TextSequenceModule<T>> m_internal_sequence; // TODO: Replace with generic SequenceComponent

    HeaderWidget m_header;

    Layout m_layout = Layout::full;
};

#endif //SERIALISTLOOPER_NEW_GENERATOR_COMPONENT_H
