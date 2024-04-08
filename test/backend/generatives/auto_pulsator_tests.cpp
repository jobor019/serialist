#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/generatives/auto_pulsator.h"

TEST_CASE("AutoPulsator ctor") {
    AutoPulsator p;
}

static AutoPulsator init_pulsator(double duration, double legato, double time) {
    AutoPulsator p;
    p.set_duration(duration);
    p.set_legato_amount(legato);
    p.start(time, std::nullopt);
    return p;
}

const inline double epsilon = 1e-8;

TEST_CASE("legato = 1.0") {
    double duration = 2.0;
    double legato = 1.0;
    double time = 0.0;
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    AutoPulsator p = init_pulsator(duration, legato, time);
    REQUIRE(p.poll(time - epsilon).empty());
    auto triggers = p.poll(time + epsilon);
    REQUIRE(triggers.size() == 1);
    REQUIRE(Trigger::contains_pulse_on(triggers, first_id));

    for (std::size_t i = first_id; i < 10; ++i) {
        time += duration;
        REQUIRE(p.poll(time - epsilon).empty());

        triggers = p.poll(time + epsilon);
        REQUIRE(triggers.size() == 2);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));
        REQUIRE(Trigger::contains_pulse_off(triggers, i));
    }
}

TEST_CASE("legato < 1.0") {
    double duration = 2.0;
    double legato = 0.5;
    double time = 0.0;
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    AutoPulsator p = init_pulsator(duration, legato, time);
    REQUIRE(Trigger::contains_pulse_on(p.poll(time + epsilon), first_id));

    for (std::size_t i = first_id; i < 10; ++i) {
        time += legato * duration;
        REQUIRE(p.poll(time - epsilon).empty());

        // pulse off before pulse on
        auto triggers = p.poll(time + epsilon);
        REQUIRE(triggers.size() == 1);
        REQUIRE(Trigger::contains_pulse_off(triggers, i));

        time += (1 - legato) * duration;
        REQUIRE(p.poll(time - epsilon).empty());
        triggers = p.poll(time + epsilon);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));
    }
}

TEST_CASE("legato > 1.0") {
    double duration = 2.0;
    double legato = 1.2;
    double time = 0.0;
    std::size_t first_id = TriggerIds::get_instance().peek_next_id();

    AutoPulsator p = init_pulsator(duration, legato, time);
    REQUIRE(Trigger::contains_pulse_on(p.poll(time + epsilon), first_id));

    time += duration;
//    int i = 0;
    for (std::size_t i = first_id; i < 10; ++i) {
        REQUIRE(p.poll(time - epsilon).empty());

        // next pulse on before previous pulse off
        auto triggers = p.poll(time + epsilon);
        REQUIRE(triggers.size() == 1);
        REQUIRE(Trigger::contains_pulse_on(triggers, i + 1));

        double increment_factor = (legato - 1);
        time += increment_factor * duration;
        REQUIRE(p.poll(time - epsilon).empty());
        triggers = p.poll(time + epsilon);
        REQUIRE(Trigger::contains_pulse_off(triggers, i));

        time += (1 - increment_factor) * duration;
    }
}

TEST_CASE("Start/Stop/Flush") {
    auto p = AutoPulsator(1.0, 0.5);

    double time = 0.0;


    SECTION("Stop immediately after starting") {
        p.start(time, std::nullopt);
        auto flushed = p.stop();
        REQUIRE(flushed.empty());

        // stop again
        flushed = p.stop();
        REQUIRE(flushed.empty());
    }

    SECTION("Stop after first poll") {
        p.start(time, std::nullopt);
        REQUIRE(Trigger::contains_pulse_on(p.poll(time + epsilon)));
        auto flushed = p.stop();
        REQUIRE(flushed.size() == 1);
        REQUIRE(Trigger::contains_pulse_off(flushed));

        // stop again
        flushed = p.stop();
        REQUIRE(flushed.empty());
    }

    SECTION("Repeated start") {
        p.start(time, std::nullopt);
        REQUIRE(Trigger::contains_pulse_on(p.poll(time + epsilon)));
        p.start(time, std::nullopt);
        auto flushed = p.stop();
        REQUIRE(flushed.size() == 1);
        REQUIRE(Trigger::contains_pulse_off(flushed));
    }

    SECTION("start - stop - start") {
        p.start(time, std::nullopt);
        REQUIRE(Trigger::contains_pulse_on(p.poll(time + epsilon)));
        auto flushed = p.stop();
        REQUIRE(flushed.size() == 1);
        REQUIRE(Trigger::contains_pulse_off(flushed));

        p.start(time, std::nullopt);
        REQUIRE(Trigger::contains_pulse_on(p.poll(time + epsilon)));
        time += 1.0;
        auto triggers = p.poll(time + epsilon);
        REQUIRE(triggers.size() == 2);
        REQUIRE(Trigger::contains_pulse_off(triggers));
        REQUIRE(Trigger::contains_pulse_on(triggers));

    }

    SECTION("stop without any pulse_off") {
        p.start(time, std::nullopt);
        REQUIRE(Trigger::contains_pulse_on(p.poll(time + epsilon)));
        time += 0.5;
        auto triggers = p.poll(time + epsilon);
        REQUIRE(triggers.size() == 1);
        REQUIRE(Trigger::contains_pulse_off(triggers));

        auto flushed = p.stop();
        REQUIRE(flushed.empty());

    }
}

TEST_CASE("Export/Import") {

    AutoPulsator p1;
    AutoPulsator p2;

    auto time = 0.0;

    p1.start(time, std::nullopt);
    REQUIRE(Trigger::contains_pulse_on(p1.poll(time + epsilon), TriggerIds::FIRST_ID));
    p2.import_pulses(p1.export_pulses());

    REQUIRE(p1.stop().empty());

    p2.start(time, std::nullopt);
    REQUIRE(Trigger::contains_pulse_on(p2.poll(time + epsilon), TriggerIds::FIRST_ID + 1));

    time += 1.0;

    auto triggers = p2.poll(time - epsilon);
    REQUIRE(triggers.empty());

    triggers = p2.poll(time + epsilon);
    REQUIRE(triggers.size() == 3); // imported pulse_off, default pulse_off, next pulse_on

    REQUIRE(Trigger::contains_pulse_off(triggers, TriggerIds::FIRST_ID + 1));
    REQUIRE(Trigger::contains_pulse_on(triggers, TriggerIds::FIRST_ID + 2));
}
