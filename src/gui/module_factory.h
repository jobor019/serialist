

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

template<typename ModuleType>
using ModuleAndGeneratives = std::pair<std::unique_ptr<ModuleType>, std::vector<std::unique_ptr<Generative>>>;

class ModuleFactory {
public:
    ModuleFactory() = delete;


    static ModuleAndGeneratives<NoteSourceModule>
    new_midi_note_source(const std::string& id
                         , ParameterHandler& parent
                         , NoteSourceModule::Layout layout = NoteSourceModule::Layout::full) {
        auto note_source = std::make_unique<MidiNoteSource>(id, parent);

        note_source->set_midi_device(MidiConfig::get_instance().get_default_device_name());

        auto onset = std::make_unique<Variable<float>>(id + "::onset", parent, 1.0f);
        auto duration = std::make_unique<Variable<float>>(id + "::duration", parent, 1.0f);
        auto pitch = std::make_unique<Variable<int>>(id + "::pitch", parent, 6000);
        auto velocity = std::make_unique<Variable<int>>(id + "::velocity", parent, 100);
        auto channel = std::make_unique<Variable<int>>(id + "::channel", parent, 1);
        auto enabled = std::make_unique<Variable<bool>>(id + "::enabled", parent, true);

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

        return std::make_pair(std::move(note_source_module), std::move(generatives));
    }


    static ModuleAndGeneratives<OscillatorModule>
    new_oscillator(const std::string& id
                   , ParameterHandler& parent
                   , OscillatorModule::Layout layout = OscillatorModule::Layout::full) {
        auto oscillator = std::make_unique<Oscillator>(id, parent);
        auto type = std::make_unique<Variable<Oscillator::Type>>(id, parent, Oscillator::Type::phasor);
        auto freq = std::make_unique<Variable<float>>(id + "::freq", parent, 0.25f);
        auto mul = std::make_unique<Variable<float>>(id + "::mul", parent, 1.0f);
        auto add = std::make_unique<Variable<float>>(id + "::add", parent, 0.0f);
        auto duty = std::make_unique<Variable<float>>(id + "::duty", parent, 0.5f);
        auto curve = std::make_unique<Variable<float>>(id + "::curve", parent, 1.0f);
        auto enabled = std::make_unique<Variable<bool>>(id + "::enabled", parent, true);

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
    new_text_sequence(const std::string& id
                      , ParameterHandler& parent
                      , typename TextSequenceModule<T>::Layout layout = TextSequenceModule<T>::Layout::full) {
        auto sequence = std::make_unique<Sequence<T>>(id, parent);

        auto enabled = std::make_unique<Variable<bool>>(id + "::enabled", parent, true);
        sequence->set_enabled(enabled.get());

        auto sequence_module = std::make_unique<TextSequenceModule<T>>(*sequence, *enabled, layout);

        std::vector<std::unique_ptr<Generative>> generatives = collect(std::move(sequence), std::move(enabled));

        return {std::move(sequence_module), std::move(generatives)};
    }


    template<typename T>
    static ModuleAndGeneratives<InterpolationModule<T>>
    new_interpolator(const std::string& id
                     , ParameterHandler& parent
                     , typename InterpolationModule<T>::Layout layout = InterpolationModule<T>::Layout::full) {
        auto interpolator = std::make_unique<Variable<InterpolationStrategy<T>>>(id
                                                                                 , parent
                                                                                 , InterpolationStrategy<T>());

        auto interpolator_module = std::make_unique<InterpolationModule<T>>(*interpolator, layout);

        std::vector<std::unique_ptr<Generative>> generatives = collect(std::move(interpolator));

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
