

#ifndef SERIALISTLOOPER_MODULE_FACTORY_H
#define SERIALISTLOOPER_MODULE_FACTORY_H

#include <vector>
#include "generative_component.h"
#include "oscillator.h"
#include "variable.h"
#include "modules/oscillator_module.h"

template<typename Module>
using ModuleAndGeneratives = std::pair<std::unique_ptr<Module>, std::vector<std::unique_ptr<Generative>>>;

class ModuleFactory {
public:
    ModuleFactory() = delete;


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
        auto enabled = std::make_unique<Variable<bool>>(id + "::enabled", parent, 0.5f);

        oscillator->set_type(type.get());
        oscillator->set_freq(freq.get());
        oscillator->set_mul(mul.get());
        oscillator->set_add(add.get());
        oscillator->set_duty(duty.get());
        oscillator->set_curve(curve.get());
        oscillator->set_enabled(enabled.get());

        auto oscillator_module = std::make_unique<OscillatorModule>(*oscillator
                                                                    , *type
                                                                    , *freq
                                                                    , *mul
                                                                    , *add
                                                                    , *duty
                                                                    , *curve
                                                                    , *enabled
                                                                    , layout);

        std::vector<std::unique_ptr<Generative>> generatives;
        generatives.push_back(std::move(oscillator));
        generatives.push_back(std::move(type));
        generatives.push_back(std::move(freq));
        generatives.push_back(std::move(mul));
        generatives.push_back(std::move(add));
        generatives.push_back(std::move(duty));
        generatives.push_back(std::move(curve));
        generatives.push_back(std::move(enabled));

        return {std::move(oscillator_module), std::move(generatives)};
    }


private:

};

#endif //SERIALISTLOOPER_MODULE_FACTORY_H
