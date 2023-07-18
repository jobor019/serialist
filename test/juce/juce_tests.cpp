#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "parameter_policy.h"
#include "variable.h"
#include "sequence.h"


TEST_CASE("Connectivity") {
        juce::UndoManager um;
    auto handler = ParameterHandler(um);
//    auto mg = ModularGenerator(handler);

//    std::cout << std::is_convertible_v<Facet, float> << "\n";
    auto v = Variable<Facet, float>("jhhe", handler, 1.0f);
    auto seq = Sequence<Facet, float>("seq", handler);



//    juce::MessageManager::getInstance();
//    juce::MessageManager::getInstance()->runDispatchLoop();
//    auto lnf = std::make_unique<SerialistLookAndFeel>();
//    SerialistLookAndFeel::setup_look_and_feel_colors(*lnf);
//    juce::Desktop::getInstance().setDefaultLookAndFeel(lnf.get());
//
//    juce::UndoManager um;
//    auto handler = ParameterHandler(um);
//    auto mg = ModularGenerator(handler);
//
//    auto osc = ModuleFactory::new_oscillator(mg);
//
//    auto osc_module = std::move(osc.first);
//    std::vector<std::unique_ptr<Generative>> generatives = std::move(osc.second);
//
//    osc_module.reset();
//
//    Generative* oscillator_as_generative = nullptr;
//    for (auto& g : generatives) {
//        if (g->get_parameter_handler().get_value_tree().getProperty("class").toString().toStdString() == "oscillator_as_generative") {
//            oscillator_as_generative = g.get();
//        }
//    }

//    auto* oscillator = dynamic_cast<Oscillator*>(oscillator_as_generative);

//    if (!oscillator) {
//        throw std::runtime_error("Couldn't find oscillator_as_generative");
//    }
//
//    std::cout << oscillator->get_curve().is_connectable(*oscillator_as_generative) << "\n";

//    juce::MessageManager::deleteInstance();


}