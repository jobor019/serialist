
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/policies/policies.h"
#include "core/generatives/phase_pulsator.h"

using namespace serialist;

TEST_CASE("Phase: zero", "[phase]") {
    auto epsilon = GENERATE(1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-16);
    REQUIRE_THAT(Phase::zero(epsilon).get(), Catch::Matchers::WithinAbs(0, 1e-16));
}

TEST_CASE("Phase: one", "[phase]") {
    // Ensuring that Phase::one doesn't wrap around to 0.0 nor goes above 1.0
    auto epsilon = GENERATE(1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-16);
    CAPTURE(epsilon);
    REQUIRE(Phase::one(epsilon).get() > 0.9);
    REQUIRE(Phase::one(epsilon).get() < 1.0);
}

TEST_CASE("Phase: constructor", "[phase]") {
    // Values in range [0.0, 1.0)
    REQUIRE_THAT(Phase(0.0).get(), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(Phase(0.1).get(), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase(0.9999).get(), Catch::Matchers::WithinAbs(0.9999, 1e-8));

    // Values < 0.0
    REQUIRE_THAT(Phase(-0.1).get(), Catch::Matchers::WithinAbs(0.9, 1e-8));
    REQUIRE_THAT(Phase(-0.9).get(), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase(-1.2).get(), Catch::Matchers::WithinAbs(0.8, 1e-8));

    // Values >= 1.0
    REQUIRE_THAT(Phase(1.0).get(), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(Phase(1.1).get(), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase(1.9999).get(), Catch::Matchers::WithinAbs(0.9999, 1e-8));
}

TEST_CASE("Phase: arithmetic operators", "[phase]") {
    SECTION("Addition") {
        REQUIRE_THAT((Phase(0.1) + 0.0).get(), Catch::Matchers::WithinAbs(0.1, 1e-8));
        REQUIRE_THAT((Phase(0.1) + 0.2).get(), Catch::Matchers::WithinAbs(0.3, 1e-8));
        REQUIRE_THAT((Phase(0.1) + 0.9).get(), Catch::Matchers::WithinAbs(0.0, 1e-8));

        REQUIRE_THAT((Phase(0.0) + -0.1).get(), Catch::Matchers::WithinAbs(0.9, 1e-8));
    }
}



TEST_CASE("Phase: abs_delta_phase", "[phase]") {
    REQUIRE_THAT(Phase::abs_delta_phase(Phase{0.0}, Phase{0.1}), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase::abs_delta_phase(Phase{0.1}, Phase{0.0}), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase::abs_delta_phase(Phase{0.0}, Phase{0.9}), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase::abs_delta_phase(Phase{0.9}, Phase{0.0}), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase::abs_delta_phase(Phase{0.9}, Phase{0.8}), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Phase::abs_delta_phase(Phase{0.01}, Phase{0.5}), Catch::Matchers::WithinAbs(0.49, 1e-8));
    REQUIRE_THAT(Phase::abs_delta_phase(Phase{0.0}, Phase{0.51}), Catch::Matchers::WithinAbs(0.49, 1e-8));
}

TEST_CASE("Phase: direction", "[phase]") {
    REQUIRE(Phase::direction(Phase{0.0}, Phase{0.1}) == Phase::Direction::forward);
    REQUIRE(Phase::direction(Phase{0.9}, Phase{0.0}) == Phase::Direction::forward);

    REQUIRE(Phase::direction(Phase{0.1}, Phase{0.0}) == Phase::Direction::backward);
    REQUIRE(Phase::direction(Phase{0.0}, Phase{0.9}) == Phase::Direction::backward);

    REQUIRE(Phase::direction(Phase{0.0}, Phase{0.0}) == Phase::Direction::unchanged);
    REQUIRE(Phase::direction(Phase{0.9}, Phase{0.9}) == Phase::Direction::unchanged);
}

TEST_CASE("Phase: contains", "[phase]") {
    SECTION("Inferred forward direction") {
        SECTION("Without wraparound") {
            // Forward direction without wraparound: 0.2 -> 0.4
            // Clear forward direction (delta = 0.2, which is < 0.5)
            Phase start{0.2};
            Phase end{0.4};
            
            // Boundary cases
            REQUIRE(Phase::contains(start, end, start));  // Start point is included
            REQUIRE(Phase::contains(start, end, end));    // End point is included
            
            // Inside the range
            REQUIRE(Phase::contains(start, end, Phase{0.3}));
            REQUIRE(Phase::contains(start, end, Phase{0.25}));
            REQUIRE(Phase::contains(start, end, Phase{0.39}));
            
            // Outside the range
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.1}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.19}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.41}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.6}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.0}));
        }
        
        SECTION("With wraparound") {
            // Forward direction with wraparound: 0.9 -> 0.1
            // Clear forward direction with wraparound (delta = 0.2, which is < 0.5)
            Phase start{0.9};
            Phase end{0.1};
            
            // Boundary cases
            REQUIRE(Phase::contains(start, end, start));  // Start point is included
            REQUIRE(Phase::contains(start, end, end));    // End point is included
            
            // Inside the range - after start before wrap
            REQUIRE(Phase::contains(start, end, Phase{0.95}));
            REQUIRE(Phase::contains(start, end, Phase{0.99}));
            
            // Inside the range - after wrap before end
            REQUIRE(Phase::contains(start, end, Phase{0.0}));
            REQUIRE(Phase::contains(start, end, Phase{0.05}));
            REQUIRE(Phase::contains(start, end, Phase{0.09}));
            
            // Outside the range
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.11}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.3}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.7}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.89}));
            
            // Edge case - exactly at wrap point
            REQUIRE(Phase::contains(start, end, Phase{0.999999}));  // Just before 1.0
        }
    }

    SECTION("Inferred backward direction") {
        SECTION("Without wraparound") {
            // Backward direction without wraparound: 0.7 -> 0.3
            // Clear backward direction (delta = 0.4, which is < 0.5)
            Phase start{0.7};
            Phase end{0.3};
            
            // Boundary cases
            REQUIRE(Phase::contains(start, end, start));  // Start point is included
            REQUIRE(Phase::contains(start, end, end));    // End point is included
            
            // Inside the range
            REQUIRE(Phase::contains(start, end, Phase{0.6}));
            REQUIRE(Phase::contains(start, end, Phase{0.5}));
            REQUIRE(Phase::contains(start, end, Phase{0.4}));
            
            // Outside the range
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.2}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.29}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.71}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.8}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.0}));
        }
        
        SECTION("With wraparound") {
            // Backward direction with wraparound: 0.1 -> 0.8
            // Clear backward direction with wraparound (delta = 0.3, which is < 0.5)
            Phase start{0.1};
            Phase end{0.8};
            
            // Boundary cases
            REQUIRE(Phase::contains(start, end, start));  // Start point is included
            REQUIRE(Phase::contains(start, end, end));    // End point is included
            
            // Inside the range - after end before wrap
            REQUIRE(Phase::contains(start, end, Phase{0.9}));
            REQUIRE(Phase::contains(start, end, Phase{0.95}));
            
            // Inside the range - after wrap before start
            REQUIRE(Phase::contains(start, end, Phase{0.0}));
            REQUIRE(Phase::contains(start, end, Phase{0.05}));
            REQUIRE(Phase::contains(start, end, Phase{0.09}));
            
            // Outside the range
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.11}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.3}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.7}));
            REQUIRE_FALSE(Phase::contains(start, end, Phase{0.79}));
            
            // Edge case - exactly at wrap point
            REQUIRE(Phase::contains(start, end, Phase{0.999999}));  // Just before 1.0
        }
    }

    SECTION("Explicit forward direction") {
        SECTION("Without wraparound - small distance") {
            // Testing a small distance (0.2) with explicit forward direction
            REQUIRE(Phase::contains(Phase{0.2}, Phase{0.4}, Phase{0.2}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.2}, Phase{0.4}, Phase{0.3}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.2}, Phase{0.4}, Phase{0.4}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.2}, Phase{0.4}, Phase{0.1}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.2}, Phase{0.4}, Phase{0.5}, Phase::Direction::forward));
        }

        SECTION("Without wraparound - large distance") {
            // Testing a large distance (0.8) with explicit forward direction
            // This would normally be inferred as backward (shortest path), but we explicitly specify forward
            REQUIRE(Phase::contains(Phase{0.1}, Phase{0.9}, Phase{0.1}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.1}, Phase{0.9}, Phase{0.5}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.1}, Phase{0.9}, Phase{0.9}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.1}, Phase{0.9}, Phase{0.0}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.1}, Phase{0.9}, Phase{0.95}, Phase::Direction::forward));
        }

        SECTION("With wraparound - small distance") {
            // Testing a small distance (0.2) with wraparound and explicit forward direction
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.9}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.95}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.0}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.1}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.2}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.8}, Phase::Direction::forward));
        }

        SECTION("With wraparound - large distance") {
            // Testing a large distance (0.7) with wraparound and explicit forward direction
            // This would normally be inferred as backward (shortest path), but we explicitly specify forward
            REQUIRE(Phase::contains(Phase{0.6}, Phase{0.3}, Phase{0.6}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.6}, Phase{0.3}, Phase{0.8}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.6}, Phase{0.3}, Phase{0.0}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.6}, Phase{0.3}, Phase{0.2}, Phase::Direction::forward));
            REQUIRE(Phase::contains(Phase{0.6}, Phase{0.3}, Phase{0.3}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.6}, Phase{0.3}, Phase{0.4}, Phase::Direction::forward));
            REQUIRE_FALSE(Phase::contains(Phase{0.6}, Phase{0.3}, Phase{0.5}, Phase::Direction::forward));
        }
    }

    SECTION("Explicit backward direction") {
        SECTION("Without wraparound - small distance") {
            // Testing a small distance (0.2) with explicit backward direction
            REQUIRE(Phase::contains(Phase{0.4}, Phase{0.2}, Phase{0.4}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.4}, Phase{0.2}, Phase{0.3}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.4}, Phase{0.2}, Phase{0.2}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.4}, Phase{0.2}, Phase{0.1}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.4}, Phase{0.2}, Phase{0.5}, Phase::Direction::backward));
        }

        SECTION("Without wraparound - large distance") {
            // Testing a large distance (0.8) with explicit backward direction
            // This would normally be inferred as forward (shortest path), but we explicitly specify backward
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.9}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.7}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.5}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.3}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.1}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.0}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.9}, Phase{0.1}, Phase{0.95}, Phase::Direction::backward));
        }

        SECTION("With wraparound - small distance") {
            // Testing a small distance (0.3) with wraparound and explicit backward direction
            REQUIRE(Phase::contains(Phase{0.1}, Phase{0.8}, Phase{0.1}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.1}, Phase{0.8}, Phase{0.0}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.1}, Phase{0.8}, Phase{0.9}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.1}, Phase{0.8}, Phase{0.8}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.1}, Phase{0.8}, Phase{0.2}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.1}, Phase{0.8}, Phase{0.7}, Phase::Direction::backward));
        }

        SECTION("With wraparound - large distance") {
            // Testing a large distance (0.7) with wraparound and explicit backward direction
            // This would normally be inferred as forward (shortest path), but we explicitly specify backward
            REQUIRE(Phase::contains(Phase{0.3}, Phase{0.6}, Phase{0.3}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.3}, Phase{0.6}, Phase{0.0}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.3}, Phase{0.6}, Phase{0.9}, Phase::Direction::backward));
            REQUIRE(Phase::contains(Phase{0.3}, Phase{0.6}, Phase{0.6}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.3}, Phase{0.6}, Phase{0.5}, Phase::Direction::backward));
            REQUIRE_FALSE(Phase::contains(Phase{0.3}, Phase{0.6}, Phase{0.4}, Phase::Direction::backward));
        }
    }

    SECTION("Equal start/end") {
        // When start and end are equal, the interval is a single point
        Phase point{0.5};
        
        // The point itself should be contained
        REQUIRE(Phase::contains(point, point, point));
        
        // Any other point should not be contained
        REQUIRE_FALSE(Phase::contains(point, point, Phase{0.49}));
        REQUIRE_FALSE(Phase::contains(point, point, Phase{0.51}));
        REQUIRE_FALSE(Phase::contains(point, point, Phase{0.0}));
        
        // Test with explicit directions
        REQUIRE(Phase::contains(point, point, point, Phase::Direction::forward));
        REQUIRE(Phase::contains(point, point, point, Phase::Direction::backward));
        
        // Other points should not be contained regardless of direction
        REQUIRE_FALSE(Phase::contains(point, point, Phase{0.6}, Phase::Direction::forward));
        REQUIRE_FALSE(Phase::contains(point, point, Phase{0.4}, Phase::Direction::backward));
    }
}

TEST_CASE("Phase: contains_directed", "[phase]") {
    // Testing with explicit interval_direction=forward
    // Position in interval (without wraparound) in the right direction
    REQUIRE(Phase::contains_directed( Phase{0.2}, Phase{0.4}, Phase{0.3},
        Phase::Direction::forward, Phase::Direction::forward));
    
    // Position in interval (without wraparound) in the wrong direction
    REQUIRE_FALSE(Phase::contains_directed(
        Phase{0.2}, Phase{0.4}, Phase{0.3},
        Phase::Direction::backward, Phase::Direction::forward));
    
    // Position in interval (with wraparound) in the right direction
    REQUIRE(Phase::contains_directed( Phase{0.9}, Phase{0.1}, Phase{0.0},
        Phase::Direction::forward, Phase::Direction::forward));
    
    // Position in interval (with wraparound) in the wrong direction
    REQUIRE_FALSE(Phase::contains_directed(
        Phase{0.9}, Phase{0.1}, Phase{0.0}, 
        Phase::Direction::backward, Phase::Direction::forward));
    
    // Testing with explicit interval_direction=backward
    // Position in interval (without wraparound) in the right direction
    REQUIRE(Phase::contains_directed(
        Phase{0.7}, Phase{0.3}, Phase{0.5}, 
        Phase::Direction::backward, Phase::Direction::backward));
    
    // Position in interval (without wraparound) in the wrong direction
    REQUIRE_FALSE(Phase::contains_directed(
        Phase{0.7}, Phase{0.3}, Phase{0.5}, 
        Phase::Direction::forward, Phase::Direction::backward));
    
    // Position in interval (with wraparound) in the right direction
    REQUIRE(Phase::contains_directed(
        Phase{0.1}, Phase{0.8}, Phase{0.0}, 
        Phase::Direction::backward, Phase::Direction::backward));
    
    // Position in interval (with wraparound) in the wrong direction
    REQUIRE_FALSE(Phase::contains_directed(
        Phase{0.1}, Phase{0.8}, Phase{0.0}, 
        Phase::Direction::forward, Phase::Direction::backward));
}

TEST_CASE("Phase: wraps_around", "[phase]") {
    REQUIRE(Phase::wraps_around(Phase{0.9}, Phase{0.1}, Phase::Direction::forward));
    REQUIRE(Phase::wraps_around(Phase{0.99999}, Phase{0.0}, Phase::Direction::forward));

    REQUIRE_FALSE(Phase::wraps_around(Phase{0.0}, Phase{0.001}, Phase::Direction::forward));
    REQUIRE_FALSE(Phase::wraps_around(Phase{0.1}, Phase{0.9}, Phase::Direction::forward));

    REQUIRE(Phase::wraps_around(Phase{0.1}, Phase{0.9}, Phase::Direction::backward));
    REQUIRE(Phase::wraps_around(Phase{0.0}, Phase{0.99999}, Phase::Direction::backward));

    REQUIRE_FALSE(Phase::wraps_around(Phase{0.00001}, Phase{0.0}, Phase::Direction::backward));
    REQUIRE_FALSE(Phase::wraps_around(Phase{0.99999}, Phase{0.99998}, Phase::Direction::backward));

    REQUIRE_FALSE(Phase::wraps_around(Phase{0.0}, Phase{0.0}));
}
