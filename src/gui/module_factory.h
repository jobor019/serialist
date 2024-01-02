

#ifndef SERIALISTLOOPER_MODULE_FACTORY_H
#define SERIALISTLOOPER_MODULE_FACTORY_H

#include <vector>
#include "state/generative_component.h"
#include "core/generatives/oscillator.h"
#include "core/generatives/variable.h"
#include "modules/oscillator_module.h"
#include "text_sequence_module.h"
#include "interpolation_module.h"
#include "note_source_module.h"
#include "generator_module.h"
#include "core/generation_graph.h"

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


    /**
     * Note: result must be destroyed in the order `component` -> `generatives`,
     *  otherwise will result in all sorts of errors (typically "Pure virtual function called" as the object
     *  managing the value tree is deleted before the module listening to the value tree)
     */
    [[nodiscard]] static std::optional<ComponentAndGeneratives>
    new_from_key(int key, GenerationGraph& graph) {
        if (key == KeyCodes::NEW_GENERATOR_KEY) {
            auto mng = new_generator<Facet, float>(graph);
            return {ComponentAndGeneratives::from_internal(std::move(mng))};
        } else if (key == KeyCodes::NEW_MIDI_SOURCE_KEY) {
            auto mng = new_note_source(graph);
            return {ComponentAndGeneratives::from_internal(std::move(mng))};
        } else if (key == KeyCodes::NEW_OSCILLATOR_KEY) {
            auto mng = new_oscillator(graph);
            return {ComponentAndGeneratives::from_internal(std::move(mng))};
        } else if (key == KeyCodes::NEW_PULSATOR_KEY) {
            auto mng = new_pulsator(graph);
            return {ComponentAndGeneratives::from_internal(std::move(mng))};
        }

        return std::nullopt;
    }


    [[nodiscard]] static ModuleAndGeneratives<NoteSourceModule>
    new_note_source(GenerationGraph& graph
                    , NoteSourceModule::Layout layout = NoteSourceModule::Layout::full) {

        auto& parent = graph.get_parameter_handler();

        auto pitch = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 6000.0f);
        auto velocity = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 100.0f);
        auto channel = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);
        auto enabled = std::make_unique<Variable<Facet, bool>>(graph.next_id(), parent, true);
        auto num_voices = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);

        auto [pulsator_module, pulsator_generatives] = new_pulsator(graph
                                                                    , PulsatorModule::Layout::note_source_internal
                                                                    , enabled.get()
                                                                    , num_voices.get());

        auto* pulsator = dynamic_cast<Node<TriggerEvent>*>(&pulsator_module->get_generative());

        assert(pulsator);

        auto note_source = std::make_unique<NoteSource>(graph.next_id(), parent, pulsator
                                                        , pitch.get(), velocity.get(), channel.get()
                                                        , enabled.get(), num_voices.get());

        note_source->set_midi_device(MidiConfig::get_instance().get_default_device_name());

        auto note_source_module = std::make_unique<NoteSourceModule>(
                *note_source, std::move(pulsator_module), *pitch, *velocity, *channel, *enabled, *num_voices, layout);

        auto generatives = collect(std::move(note_source), std::move(pitch), std::move(velocity)
                                   , std::move(channel), std::move(enabled), std::move(num_voices));
        std::move(pulsator_generatives.begin(), pulsator_generatives.end(), std::back_inserter(generatives));

        return {std::move(note_source_module), std::move(generatives)};
    }


    template<typename OutputType, typename SequenceStorageType>
    [[nodiscard]] static ModuleAndGeneratives<GeneratorModule<OutputType, SequenceStorageType>>
    new_generator(GenerationGraph& graph
                  , typename GeneratorModule<OutputType, SequenceStorageType>::Layout layout = GeneratorModule<
            OutputType, SequenceStorageType>::Layout::full) {

        auto& parent = graph.get_parameter_handler();

        auto [oscillator_module, oscillator_generatives] = new_oscillator(
                graph, OscillatorModule::Layout::generator_internal);
        auto [interpolator_module, interpolator_generatives] = new_interpolator(
                graph, InterpolationModule::Layout::generator_internal);
        auto [sequence_module, sequence_generatives] = new_text_sequence<OutputType, SequenceStorageType>(
                graph, TextSequenceModule<OutputType, SequenceStorageType>::Layout::full);

        auto oscillator = dynamic_cast<Node<Facet>*>(&oscillator_module->get_generative());
        auto interpolator = dynamic_cast<Node<InterpolationStrategy>*>(&interpolator_module->get_generative());
        auto sequence = dynamic_cast<Sequence<OutputType, SequenceStorageType>*>(&sequence_module->get_generative());

        assert(oscillator && interpolator && sequence);

        auto enabled = std::make_unique<Variable<Facet, bool>>(graph.next_id(), parent, true);
        auto num_voices = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);

        auto generator = std::make_unique<Generator<OutputType>>(
                graph.next_id(), parent, oscillator, interpolator, sequence, enabled.get());

        auto generator_module = std::make_unique<GeneratorModule<OutputType, SequenceStorageType>>(
                *generator
                , std::move(oscillator_module)
                , std::move(interpolator_module)
                , std::move(sequence_module)
                , *enabled
                , *num_voices
                , layout);

        auto generatives = collect(std::move(generator), std::move(enabled), std::move(num_voices));
        std::move(oscillator_generatives.begin(), oscillator_generatives.end(), std::back_inserter(generatives));
        std::move(interpolator_generatives.begin(), interpolator_generatives.end(), std::back_inserter(generatives));
        std::move(sequence_generatives.begin(), sequence_generatives.end(), std::back_inserter(generatives));

        return {std::move(generator_module), std::move(generatives)};
    }


    [[nodiscard]] static ModuleAndGeneratives<OscillatorModule>
    new_oscillator(GenerationGraph& graph
                   , OscillatorModule::Layout layout = OscillatorModule::Layout::full) {

        auto& parent = graph.get_parameter_handler();

        auto oscillator = std::make_unique<OscillatorNode>(graph.next_id(), parent);

        // TODO: Need to implement a module for the pulse_on to be able to replace/connect it
        auto internal_trigger = std::make_unique<UnitPulse>(graph.next_id(), parent);

        auto type = std::make_unique<Variable<Facet, OscillatorNode::Type>>(graph.next_id(), parent
                                                                            , OscillatorNode::Type::phasor);
        auto freq = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 0.25f);
        auto mul = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);
        auto add = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 0.0f);
        auto duty = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 0.5f);
        auto curve = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);
        auto enabled = std::make_unique<Variable<Facet, bool>>(graph.next_id(), parent, true);
        auto num_voices = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);

        oscillator->set_trigger(internal_trigger.get());
        oscillator->set_type(type.get());
        oscillator->set_freq(freq.get());
        oscillator->set_mul(mul.get());
        oscillator->set_add(add.get());
        oscillator->set_duty(duty.get());
        oscillator->set_curve(curve.get());
        oscillator->set_enabled(enabled.get());
        oscillator->set_num_voices(num_voices.get());

        auto oscillator_module = std::make_unique<OscillatorModule>(
                *oscillator, *type, *freq, *mul, *add, *duty, *curve, *enabled, *num_voices, layout);

        auto generatives = collect(std::move(oscillator), std::move(internal_trigger), std::move(type)
                                   , std::move(freq), std::move(mul), std::move(add), std::move(duty)
                                   , std::move(curve), std::move(enabled), std::move(num_voices));

        return {std::move(oscillator_module), std::move(generatives)};
    }


    template<typename OutputType, typename StorageType>
    [[nodiscard]] static ModuleAndGeneratives<TextSequenceModule<OutputType, StorageType>>
    new_text_sequence(
            GenerationGraph& graph
            , typename TextSequenceModule<OutputType, StorageType>::Layout layout = TextSequenceModule<OutputType
                                                                                                       , StorageType>::Layout::full) {

        auto& parent = graph.get_parameter_handler();

        auto sequence = std::make_unique<Sequence<OutputType, StorageType>>(graph.next_id(), parent);

        auto sequence_module = std::make_unique<TextSequenceModule<OutputType, StorageType>>(*sequence, layout);

        std::vector<std::unique_ptr<Generative>> generatives = collect(std::move(sequence));

        return {std::move(sequence_module), std::move(generatives)};
    }


    [[nodiscard]] static ModuleAndGeneratives<InterpolationModule>
    new_interpolator(GenerationGraph& graph
                     , InterpolationModule::Layout layout = InterpolationModule::Layout::full) {
        auto& parent = graph.get_parameter_handler();


        auto default_strategy = InterpolationStrategy::default_strategy();
        auto interpolation_adapter = std::make_unique<InterpolationAdapter>(graph.next_id(), parent);
        auto type = std::make_unique<Variable<Facet, InterpolationStrategy::Type>>(
                graph.next_id()
                , parent
                , default_strategy.get_type());
        auto pivot = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, default_strategy.get_pivot());

        interpolation_adapter->set_type(type.get());
        interpolation_adapter->set_pivot(pivot.get());


        auto interpolator_module = std::make_unique<InterpolationModule>(
                *interpolation_adapter, *type, *pivot, layout);

        auto generatives = collect(std::move(interpolation_adapter), std::move(type), std::move(pivot));

        return {std::move(interpolator_module), std::move(generatives)};

    }


    [[nodiscard]]
    static ModuleAndGeneratives<PulsatorModule> new_pulsator(GenerationGraph& graph
                                                             , PulsatorModule::Layout layout = PulsatorModule::Layout::full
                                                             , Variable<Facet, bool>* enabled = nullptr
                                                             , Variable<Facet, float>* num_voices = nullptr) {

        auto& parent = graph.get_parameter_handler();

        auto pulsator = std::make_unique<Pulsator>(graph.next_id(), parent);




        auto interval = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);
        auto duty = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);

        std::unique_ptr<Variable<Facet, bool>> internal_enabled = nullptr;
        if (!enabled) {
            internal_enabled = std::make_unique<Variable<Facet, bool>>(graph.next_id(), parent, true);
            enabled = internal_enabled.get();
        }

        std::unique_ptr<Variable<Facet, float>> internal_num_voices = nullptr;
        if (!num_voices) {
            internal_num_voices = std::make_unique<Variable<Facet, float>>(graph.next_id(), parent, 1.0f);
            num_voices = internal_num_voices.get();
        }

        pulsator->set_trigger_interval(interval.get());
        pulsator->set_duty_cycle(duty.get());
        pulsator->set_enabled(enabled);
        pulsator->set_num_voices(num_voices);

        auto pulsator_module = std::make_unique<PulsatorModule>(*pulsator, *interval, *duty
                                                                , *enabled, *num_voices, layout);

        auto generatives = collect_if(std::move(pulsator), std::move(interval), std::move(duty)
                                   , std::move(internal_enabled), std::move(internal_num_voices));

        return {std::move(pulsator_module), std::move(generatives)};
    }


private:

    template<typename... Args>
    static std::vector<std::unique_ptr<Generative>> collect(std::unique_ptr<Args> ... args) {
        std::vector<std::unique_ptr<Generative>> result;
        result.reserve(sizeof...(Args));

        (result.push_back(std::move(args)), ...);

        return result;
    }

    template<typename... Args>
    static std::vector<std::unique_ptr<Generative>> collect_if(std::unique_ptr<Args> ... args) {
        std::vector<std::unique_ptr<Generative>> result;

        ((args != nullptr ? result.push_back(std::move(args)) : void()), ...);

        return result;
    }

};

#endif //SERIALISTLOOPER_MODULE_FACTORY_H
