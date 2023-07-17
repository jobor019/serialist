

#ifndef SERIALISTLOOPER_MODULE_FACTORY_H
#define SERIALISTLOOPER_MODULE_FACTORY_H

#include <vector>
#include "generative_component.h"
#include "oscillator.h"
#include "variable.h"
#include "modules/oscillator_module.h"
#include "text_sequence_module.h"
#include "interpolation_module.h"
#include "note_source_module.h"
#include "generator_module.h"

template<typename ModuleType>
using ModuleAndGeneratives = std::pair<std::unique_ptr<ModuleType>, std::vector<std::unique_ptr<Generative>>>;

class ComponentAndGeneratives {
public:
    ComponentAndGeneratives(std::unique_ptr<GenerativeComponent> comp
                            , std::vector<std::unique_ptr<Generative>> gens
                            , int w
                            , int h)
            : component(std::move(comp)), generatives(std::move(gens)), width(w), height(h) {}


    template<typename ModuleType>
    static ComponentAndGeneratives from_internal(ModuleAndGeneratives<ModuleType> mng) {
        return {downcast(std::move(mng.first))
                , std::move(mng.second)
                , ModuleType::width_of()
                , ModuleType::height_of()};
    }


    template<typename ModuleType>
    static std::unique_ptr<GenerativeComponent> downcast(std::unique_ptr<ModuleType> derived_object) {
        static_assert(std::is_base_of_v<GenerativeComponent, ModuleType>
                      , "ModuleType must inherit from GenerativeComponent");

        std::unique_ptr<GenerativeComponent> target_generative = nullptr;

        auto* object_as_generative = dynamic_cast<GenerativeComponent*>(derived_object.get());

        if (object_as_generative) {
            target_generative.reset(object_as_generative);
            (void) derived_object.release(); // (void) is just to remove clang-tidy warning
            return target_generative;

        } else {
            return nullptr;
        }
    }


    std::unique_ptr<GenerativeComponent> component;
    std::vector<std::unique_ptr<Generative>> generatives;
    int width;
    int height;
};


class ModuleFactory {
public:

    using KeyCodes = ConfigurationLayerKeyboardShortcuts;

    ModuleFactory() = delete;


    static std::optional<ComponentAndGeneratives>
    new_from_key(int key, ModularGenerator& modular_generator) {
        if (key == KeyCodes::NEW_GENERATOR_KEY) {
            auto mng = new_generator<float>(modular_generator);
            return {ComponentAndGeneratives::from_internal(std::move(mng))};
        } else if (key == KeyCodes::NEW_MIDI_SOURCE_KEY) {
            auto mng = new_midi_note_source(modular_generator);
            return {ComponentAndGeneratives::from_internal(std::move(mng))};
        } else if (key == KeyCodes::NEW_OSCILLATOR_KEY) {
            auto mng = new_oscillator(modular_generator);
            return {ComponentAndGeneratives::from_internal(std::move(mng))};
        }

        return std::nullopt;
    }


    static ModuleAndGeneratives<NoteSourceModule>
    new_midi_note_source(ModularGenerator& mg
                         , NoteSourceModule::Layout layout = NoteSourceModule::Layout::full) {

        auto& parent = mg.get_parameter_handler();

        auto note_source = std::make_unique<MidiNoteSource>(mg.next_id(), mg.get_parameter_handler());

        note_source->set_midi_device(MidiConfig::get_instance().get_default_device_name());

        auto onset = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(1.0));
        auto duration = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(1.0));
        auto pitch = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(6000));
        auto velocity = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(100));
        auto channel = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(1));
        auto enabled = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(true));

        note_source->set_onset(onset.get());
        note_source->set_duration(duration.get());
        note_source->set_pitch(pitch.get());
        note_source->set_velocity(velocity.get());
        note_source->set_channel(channel.get());
        note_source->set_enabled(enabled.get());

        auto note_source_module = std::make_unique<NoteSourceModule>(
                *note_source, *onset, *duration, *pitch, *velocity, *channel, *enabled, layout);

        auto generatives = collect(std::move(note_source), std::move(onset), std::move(duration), std::move(pitch)
                                   , std::move(velocity), std::move(channel), std::move(enabled));

        return {std::move(note_source_module), std::move(generatives)};
    }


    template<typename T>
    static ModuleAndGeneratives<GeneratorModule<T>>
    new_generator(ModularGenerator& mg
                  , typename GeneratorModule<T>::Layout layout = GeneratorModule<T>::Layout::full) {

        auto& parent = mg.get_parameter_handler();

        auto [oscillator_module, oscillator_generatives] = new_oscillator(
                mg, OscillatorModule::Layout::generator_internal);
        auto [interpolator_module, interpolator_generatives] = new_interpolator(
                mg, InterpolationModule::Layout::generator_internal);
        auto [sequence_module, sequence_generatives] = new_text_sequence<T>(
                mg, TextSequenceModule<T>::Layout::full);

        auto oscillator = dynamic_cast<Node<Facet>*>(&oscillator_module->get_generative());
        auto interpolator = dynamic_cast<Node<InterpolationStrategy>*>(&interpolator_module->get_generative());
        auto sequence = dynamic_cast<Sequence<T>*>(&sequence_module->get_generative());

        assert(oscillator && interpolator && sequence);

        auto enabled = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(true));

        auto generator = std::make_unique<Generator<T>>(
                mg.next_id(), parent, oscillator, interpolator, sequence, enabled.get());

        auto generator_module = std::make_unique<GeneratorModule<T>>(
                *generator
                , std::move(oscillator_module)
                , std::move(interpolator_module)
                , std::move(sequence_module)
                , *enabled
                , layout);

        std::vector<std::unique_ptr<Generative>> generatives;
        // TODO: Generalize with `collect`
        generatives.push_back(std::move(generator));
        generatives.push_back(std::move(enabled));

        std::move(oscillator_generatives.begin(), oscillator_generatives.end(), std::back_inserter(generatives));

        std::move(interpolator_generatives.begin(), interpolator_generatives.end(), std::back_inserter(generatives));
        std::move(sequence_generatives.begin(), sequence_generatives.end(), std::back_inserter(generatives));

        return {std::move(generator_module), std::move(generatives)};
    }


    static ModuleAndGeneratives<OscillatorModule>
    new_oscillator(ModularGenerator& mg
                   , OscillatorModule::Layout layout = OscillatorModule::Layout::full) {

        auto& parent = mg.get_parameter_handler();

        auto oscillator = std::make_unique<Oscillator>(mg.next_id(), parent);
        auto type = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Oscillator::type_to_facet(
                Oscillator::Type::phasor));
        auto freq = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(0.25));
        auto mul = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(1.0));
        auto add = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(0.0));
        auto duty = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(0.5));
        auto curve = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(1.0));
        auto enabled = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(true));

        oscillator->set_type(type.get());
        oscillator->set_freq(freq.get());
        oscillator->set_mul(mul.get());
        oscillator->set_add(add.get());
        oscillator->set_duty(duty.get());
        oscillator->set_curve(curve.get());
        oscillator->set_enabled(enabled.get());

        auto oscillator_module = std::make_unique<OscillatorModule>(
                *oscillator, *type, *freq, *mul, *add, *duty, *curve, *enabled, layout);

        auto generatives = collect(std::move(oscillator), std::move(type), std::move(freq), std::move(mul)
                                   , std::move(add), std::move(duty), std::move(curve), std::move(enabled));

        return {std::move(oscillator_module), std::move(generatives)};
    }


    template<typename T>
    static ModuleAndGeneratives<TextSequenceModule<T>>
    new_text_sequence(ModularGenerator& mg
                      , typename TextSequenceModule<T>::Layout layout = TextSequenceModule<T>::Layout::full) {
        auto& parent = mg.get_parameter_handler();

        auto sequence = std::make_unique<Sequence<T>>(mg.next_id(), parent);

        auto enabled = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(true));
        sequence->set_enabled(enabled.get());

        auto sequence_module = std::make_unique<TextSequenceModule<T>>(*sequence, *enabled, layout);

        std::vector<std::unique_ptr<Generative>> generatives = collect(std::move(sequence), std::move(enabled));

        return {std::move(sequence_module), std::move(generatives)};
    }


    static ModuleAndGeneratives<InterpolationModule>
    new_interpolator(ModularGenerator& mg
                     , typename InterpolationModule::Layout layout = InterpolationModule::Layout::full) {
        auto& parent = mg.get_parameter_handler();


        auto default_strategy = InterpolationStrategy::default_strategy();
        auto interpolation_adapter = std::make_unique<InterpolationAdapter>(mg.next_id(), parent);
        auto type = std::make_unique<Variable<Facet>>(
                mg.next_id()
                , parent
                , InterpolationStrategy::type_to_facet(default_strategy.get_type()));
        auto pivot = std::make_unique<Variable<Facet>>(mg.next_id(), parent, Facet(default_strategy.get_pivot()));

        interpolation_adapter->set_type(type.get());
        interpolation_adapter->set_pivot(pivot.get());


        auto interpolator_module = std::make_unique<InterpolationModule>(
                *interpolation_adapter, *type, *pivot, layout);

        auto generatives = collect(std::move(interpolation_adapter), std::move(type), std::move(pivot));

        return {std::move(interpolator_module), std::move(generatives)};

    }


private:

    template<typename... Args>
    static std::vector<std::unique_ptr<Generative>> collect(std::unique_ptr<Args> ... args) {
        std::vector<std::unique_ptr<Generative>> result;
        result.reserve(sizeof...(Args));

        (result.push_back(std::move(args)), ...);

        return result;
    }

};

#endif //SERIALISTLOOPER_MODULE_FACTORY_H
