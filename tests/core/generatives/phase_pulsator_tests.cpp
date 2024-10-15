
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/policies/policies.h"
#include "core/generatives/phase_pulsator.h"

using namespace serialist;


TEST_CASE("PhasePulsator ctors") {
    PhasePulsator p;
    PhasePulsatorWrapper<> w;
}


TEST_CASE("PhasePulsatorWrapper default settings") {
    PhasePulsatorWrapper<double> w;
    auto delta_time = PhasePulsator::MINIMUM_SEGMENT_DURATION * 1.01;
    auto delta_phase = PhasePulsator::DISCONTINUITY_THRESHOLD * 0.99;

    auto current_phase = Phase(0.0);
    auto previous_phase = Phase(0.0);
    auto time = TimePoint();

    w.cursor.set_values(current_phase.get());
    w.pulsator_node.update_time(time);

    auto triggers = w.pulsator_node.process();
    REQUIRE(triggers.size() == 1);
    REQUIRE(triggers[0].size() == 1);
    REQUIRE(Trigger::contains_pulse_on(triggers[0]));
    std::size_t num_triggers = 1;

    for (auto i = 0; i < 1000; ++i) {
        previous_phase = current_phase;
        current_phase += delta_phase;
        time += delta_time;

        w.cursor.set_values(current_phase.get());
        w.pulsator_node.update_time(time);

        triggers = w.pulsator_node.process();

        if (previous_phase.get() > current_phase.get()) {
            // cursor wrapped around from ~1.0 to ~0.0
            REQUIRE(triggers.size() == 1);
            REQUIRE(triggers[0].size() == 2);
            REQUIRE(Trigger::contains_pulse_on(triggers[0]));
            REQUIRE(Trigger::contains_pulse_off(triggers[0]));
            ++num_triggers;

        } else {
            REQUIRE(triggers.is_empty_like());
        }
    }

    REQUIRE(num_triggers > 2);
}


template<typename T = double>
void evaluate_n_static_cycles(PhasePulsatorWrapper<T>& w, std::size_t num_cycles, double legato) {
    assert(legato >= 0.0 && legato < 1.0); // While legato may be >= 1.0, this test won't be able to handle such a case

    auto delta_time = PhasePulsator::MINIMUM_SEGMENT_DURATION * 1.01;
    auto delta_phase = PhasePulsator::DISCONTINUITY_THRESHOLD * 0.99;

    auto current_phase = Phase(0.0);
    auto previous_phase = Phase(0.0);
    auto time = TimePoint();

    std::size_t num_pulse_on = 0;
    std::size_t num_pulse_off = 0;

    w.cursor.set_values(current_phase.get());
    w.pulsator_node.update_time(time);

    auto triggers = w.pulsator_node.process();
    REQUIRE(triggers.size() == 1);
    REQUIRE(triggers[0].size() == 1);
    REQUIRE(Trigger::contains_pulse_on(triggers[0]));
    auto previous_trigger_phase = current_phase;

    for (auto i = 0; i < 1000; ++i) {
        previous_phase = current_phase;
        current_phase += delta_phase;
        time += delta_time;

        std::cout << i << ": " << current_phase.get() << std::endl;
        w.cursor.set_values(current_phase.get());
        w.pulsator_node.update_time(time);

        triggers = w.pulsator_node.process();

        if (!triggers.is_empty_like()) {
            triggers.print();
        }

        auto pulse_off_phase = utils::modulo(previous_trigger_phase.get() + legato, 1.0);

        if (utils::in(pulse_off_phase, previous_phase.get(), current_phase.get(), true, true)) {
            REQUIRE(!triggers.is_empty_like());
            REQUIRE(triggers[0].size() == 1);
            REQUIRE(Trigger::contains_pulse_off(triggers[0]));
            ++num_pulse_off;


        } else if (previous_phase.get() > current_phase.get()) {
            // cursor wrapped around from ~1.0 to ~0.0
            REQUIRE(!triggers.is_empty_like());
            REQUIRE(triggers[0].size() == 1);

            REQUIRE(Trigger::contains_pulse_on(triggers[0]));
            previous_trigger_phase = current_phase;
            ++num_pulse_on;

        } else {
            REQUIRE(triggers.is_empty_like());
        }
    }

    REQUIRE(num_pulse_on > 0);
    REQUIRE(num_pulse_off > 0);
    REQUIRE(std::fabs(num_pulse_on - num_pulse_off) < 2);
}


TEST_CASE("PhasePulsatorWrapper legato 0.5") {
    PhasePulsatorWrapper<double> w;
    double legato = 0.5;
    w.legato_amount.set_values(legato);

    evaluate_n_static_cycles(w, 100, legato);
}

//TEST_CASE("PhasePulsatorWrapper legato 0.9 empty corpus") {
//    PhasePulsatorWrapper<double> w;
//    double legato = 0.9;
//    w.duration.set_values(0.0);
//    w.legato_amount.set_values(legato);
//
//    evaluate_n_static_cycles(w, 100, legato);
//}

// TODO: Need to implement separate test case for this
//TEST_CASE("PhasePulsatorWrapper legato 1.1") {
//    PhasePulsatorWrapper<double> w;
//    double legato = 1.1;
//    w.legato_amount.set_values(legato);
//
//    evaluate_n_static_cycles(w, 100, legato);
//}