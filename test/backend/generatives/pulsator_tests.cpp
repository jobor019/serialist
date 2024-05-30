#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/generatives/pulsator.h"

constexpr inline double EPSILON = 1e-6;

TEST_CASE("Pulsator ctors") {
    Pulsator p;
    PulsatorWrapper<> w;
}

static Pulsator init_pulsator(DomainDuration duration
                              , std::optional<DomainDuration> offset
                              , double legato
                              , PulsatorMode mode) {
    Pulsator p;
    p.set_duration(duration);
    if (offset)
        p.set_offset(*offset);
    p.set_legato_amount(legato);
    p.set_mode(mode);
    return p;
}

// ==============================================================================================
// FREE PERIODIC
// ==============================================================================================

TEST_CASE("legato = 1.0, free pulsation") {
    auto duration = DomainDuration(2.0, DomainType::ticks);
    double legato = 1.0;
    auto mode = PulsatorMode::free_periodic;
    TimePoint time(0.0);
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    Pulsator p = init_pulsator(duration, std::nullopt, legato, mode);


    auto triggers = p.process(time);
    REQUIRE(triggers.size() == 1);
    REQUIRE(Trigger::contains_pulse_on(triggers, first_id));

    for (std::size_t i = first_id; i < 10; ++i) {
        time += duration;

        REQUIRE(p.process(time - EPSILON).empty());

        triggers = p.process(time + EPSILON);
        REQUIRE(triggers.size() == 2);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));
        REQUIRE(Trigger::contains_pulse_off(triggers, i));
    }
}

TEST_CASE("legato < 1.0, free pulsation") {
    auto duration = DomainDuration(2.0, DomainType::ticks);
    double legato = 0.5;
    auto mode = PulsatorMode::free_periodic;
    TimePoint time(0.0);
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    Pulsator p = init_pulsator(duration, std::nullopt, legato, mode);

    auto triggers = p.process(time);
    REQUIRE(Trigger::contains_pulse_on(triggers, first_id));

    for (std::size_t i = first_id; i < 10; ++i) {
        time += legato * duration;
        REQUIRE(p.process(time - EPSILON).empty());

        // pulse off before pulse on
        triggers = p.process(time + EPSILON);
        REQUIRE(triggers.size() == 1);
        REQUIRE(Trigger::contains_pulse_off(triggers, i));

        time += (1 - legato) * duration;
        REQUIRE(p.process(time - EPSILON).empty());
        triggers = p.process(time + EPSILON);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));
    }
}

TEST_CASE("legato > 1.0, free pulsation") {
    auto duration = DomainDuration(2.0, DomainType::ticks);
    double legato = 1.2;
    auto mode = PulsatorMode::free_periodic;
    TimePoint time(0.0);
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    Pulsator p = init_pulsator(duration, std::nullopt, legato, mode);
    REQUIRE(Trigger::contains_pulse_on(p.process(time), first_id));

    time += duration;
    for (std::size_t i = first_id; i < 10; ++i) {
        REQUIRE(p.process(time - EPSILON).empty());

        // next pulse on before previous pulse off
        auto triggers = p.process(time + EPSILON);
        REQUIRE(triggers.size() == 1);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));

        double increment_factor = (legato - 1);
        time += increment_factor * duration;
        REQUIRE(p.process(time - EPSILON).empty());
        triggers = p.process(time + EPSILON);
        REQUIRE(Trigger::contains_pulse_off(triggers, i));

        time += (1 - increment_factor) * duration;
    }
}

// ==============================================================================================
// TRANSPORT LOCKED
// ==============================================================================================


TEST_CASE("Grid Pulsation (ticks) without offset") {
    auto duration = DomainDuration(2.0, DomainType::ticks);
    auto offset = DomainDuration(0.0, DomainType::ticks);
    double legato = 1.0;
    auto mode = PulsatorMode::transport_locked;
    TimePoint time(0.0);
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    auto p = init_pulsator(duration, offset, legato, mode);

    REQUIRE(p.process(time - EPSILON).empty());
    auto triggers = p.process(time + EPSILON);
    REQUIRE(triggers.size() == 1);
    REQUIRE(Trigger::contains_pulse_on(triggers, first_id));

    for (std::size_t i = first_id; i < 10; ++i) {
        time += duration.get_value();
        REQUIRE(p.process(time - EPSILON).empty());

        triggers = p.process(time + EPSILON);
        REQUIRE(triggers.size() == 2);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));
        REQUIRE(Trigger::contains_pulse_off(triggers, i));
    }
}


inline void test_grid_pulsation(const DomainDuration& duration, const DomainDuration& offset, double legato) {
    TimePoint time(0.0);
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    auto p = init_pulsator(duration, offset, legato, PulsatorMode::transport_locked);

    REQUIRE(p.process(time - EPSILON).empty());

    time += offset;
    REQUIRE(p.process(time - EPSILON).empty());

    auto triggers = p.process(time + EPSILON);
    REQUIRE(Trigger::contains_pulse_on(triggers, first_id));

    for (std::size_t i = first_id; i < 10; ++i) {
        time += duration;
//        std::cout << time.to_string() << std::endl;
        REQUIRE(p.process(time - EPSILON).empty());

        triggers = p.process(time + EPSILON);
        REQUIRE(triggers.size() == 2);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));
        REQUIRE(Trigger::contains_pulse_off(triggers, i));
    }
}


TEST_CASE("Grid Pulsation (ticks) with offset 0.5 ticks") {
    auto duration = DomainDuration(2.0, DomainType::ticks);
    auto offset = DomainDuration(0.5, DomainType::ticks);
    auto legato = 1.0;

    test_grid_pulsation(duration, offset, legato);
}

TEST_CASE("Grid Pulsation (ticks) with offset 0.1 ticks") {
    auto duration = DomainDuration(1.0, DomainType::ticks);
    auto offset = DomainDuration(0.1, DomainType::ticks);
    auto legato = 1.0;

    test_grid_pulsation(duration, offset, legato);
}

TEST_CASE("Grid Pulsation (bars) with offset 0.25 bars") {
    auto duration = DomainDuration(1.0, DomainType::bars);
    auto offset = DomainDuration(0.25, DomainType::bars);
    auto legato = 1.0;

    test_grid_pulsation(duration, offset, legato);
}

TEST_CASE("Grid Pulsation (bars) with duration 0.37 bars, offset 0.0  bars") {
    auto duration = DomainDuration(0.37, DomainType::bars);
    auto offset = DomainDuration(0.0, DomainType::bars);
    auto legato = 1.0;

    test_grid_pulsation(duration, offset, legato);
}

TEST_CASE("Grid Pulsation (bars) with duration 1 bars, offset 0.63  bars") {
    auto duration = DomainDuration(1, DomainType::bars);
    auto offset = DomainDuration(0.63, DomainType::bars);
    auto legato = 1.0;

    test_grid_pulsation(duration, offset, legato);
}

// ==============================================================================================
// FREE TRIGGERED
// ==============================================================================================

TEST_CASE("Free Triggered: non-overlapping pulses") {
    auto duration = DomainDuration(2.0, DomainType::ticks);
    auto time = TimePoint::zero();

    auto p = init_pulsator(duration, std::nullopt, 1.0, PulsatorMode::free_triggered);

    REQUIRE(p.process(time).empty());
    REQUIRE(p.process(time).empty());

    std::cout << TriggerIds::get_instance().peek_next_id() << "\n";

    p.set_triggers(Voice<Trigger>::singular(Trigger::pulse_on()));


    std::cout << TriggerIds::get_instance().peek_next_id() << "\n";

    auto triggers = p.process(time);
    REQUIRE(triggers.size() == 1);
    REQUIRE(Trigger::contains_pulse_on(triggers));

    REQUIRE(p.process(time + EPSILON).empty());

    std::cout << TriggerIds::get_instance().peek_next_id() << "\n";

    for (std::size_t i = TriggerIds::get_instance().peek_next_id(); i < 10; ++i) {
        time += duration;
        REQUIRE(p.process(time - EPSILON).empty());

        p.set_triggers(Voice<Trigger>::singular(Trigger::pulse_on()));
        // process at `time` to ensure that the next pulse is scheduled at exactly `time`
        triggers = p.process(time);
        // process at `time + EPSILON` to ensure that previous pulse off always is accounted for
        triggers.extend(p.process(time + EPSILON));

        REQUIRE(triggers.size() == 2);
        REQUIRE(Trigger::contains_pulse_off(triggers, i - 1));
        REQUIRE(Trigger::contains_pulse_on(triggers, i));
    }
}

