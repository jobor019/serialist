#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/generatives/operator.h"


TEST_CASE("OperatorWrapper ctor") {
    auto op = OperatorWrapper();
}

TEST_CASE("Operator - addition") {
    auto op = OperatorWrapper<double>();
    op.type.set_values(Operator::Type::add);



    SECTION("scalar x scalar") {
        op.lhs.set_values(123.0);
        op.rhs.set_values(456.0);
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 1);
        REQUIRE(output[0].size() == 1);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(579.0, 1e-8));
    }

    SECTION("vector x scalar") {
        op.lhs.set_values(Voices<double>::singular({111, 222}));
        op.rhs.set_values(333);
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 1);
        REQUIRE(output[0].size() == 2);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(444, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][1]), Catch::Matchers::WithinAbs(555, 1e-8));
    }

    SECTION("scalar x vector") {
        op.lhs.set_values(333);
        op.rhs.set_values(Voices<double>::singular({111, 222}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 1);
        REQUIRE(output[0].size() == 2);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(444, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][1]), Catch::Matchers::WithinAbs(555, 1e-8));
    }

    SECTION("vector x vector - same size") {
        op.lhs.set_values(Voices<double>::singular({111, 222}));
        op.rhs.set_values(Voices<double>::singular({333, 444}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 1);
        REQUIRE(output[0].size() == 2);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(444, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][1]), Catch::Matchers::WithinAbs(666, 1e-8));
    }

    SECTION("vector x vector - lhs bigger") {
        op.lhs.set_values(Voices<double>::singular({1, 2, 10}));
        op.rhs.set_values(Voices<double>::singular({1, 2}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 1);
        REQUIRE(output[0].size() == 3);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(2, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][1]), Catch::Matchers::WithinAbs(4, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][2]), Catch::Matchers::WithinAbs(11, 1e-8));
    }

    SECTION("vector x vector - rhs bigger") {
        op.lhs.set_values(Voices<double>::singular({1, 2}));
        op.rhs.set_values(Voices<double>::singular({1, 2, 10}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 1);
        REQUIRE(output[0].size() == 3);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(2, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][1]), Catch::Matchers::WithinAbs(4, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][2]), Catch::Matchers::WithinAbs(11, 1e-8));
    }

    SECTION("scalar x empty") {
        op.lhs.set_values(333);
        op.rhs.set_values(Voices<double>::empty_like());
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.is_empty_like());
    }

    SECTION("empty x scalar") {
        op.lhs.set_values(Voices<double>::empty_like());
        op.rhs.set_values(333);
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.is_empty_like());
    }

    SECTION("empty x empty") {
        op.lhs.set_values(Voices<double>::empty_like());
        op.rhs.set_values(Voices<double>::empty_like());
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.is_empty_like());
    }

    SECTION("voices x empty") {
        op.lhs.set_values(Voices<double>::transposed({1, 2, 3}));
        op.rhs.set_values(Voices<double>::empty_like());
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.is_empty_like());
    }

    SECTION("empty x voices") {
        op.lhs.set_values(Voices<double>::empty_like());
        op.rhs.set_values(Voices<double>::singular({1, 2, 3}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.is_empty_like());
    }

    SECTION("scalar x voices") {
        op.lhs.set_values(2);
        op.rhs.set_values(Voices<double>::transposed({1, 2, 3}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 3);
        REQUIRE(output[0].size() == 1);
        REQUIRE(output[1].size() == 1);
        REQUIRE(output[2].size() == 1);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(3, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][0]), Catch::Matchers::WithinAbs(4, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[2][0]), Catch::Matchers::WithinAbs(5, 1e-8));
    }

    SECTION("voices x scalar") {
        op.lhs.set_values(Voices<double>::transposed({1, 2, 3}));
        op.rhs.set_values(2);
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 3);
        REQUIRE(output[0].size() == 1);
        REQUIRE(output[1].size() == 1);
        REQUIRE(output[2].size() == 1);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(3, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][0]), Catch::Matchers::WithinAbs(4, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[2][0]), Catch::Matchers::WithinAbs(5, 1e-8));
    }


    SECTION("vector x voices") {
        op.lhs.set_values(Voices<double>::singular({1, 2}));
        op.rhs.set_values(Voices<double>::transposed({3, 4}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 2);
        REQUIRE(output[0].size() == 2);
        REQUIRE(output[1].size() == 2);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(4, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][1]), Catch::Matchers::WithinAbs(5, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][0]), Catch::Matchers::WithinAbs(5, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][1]), Catch::Matchers::WithinAbs(6, 1e-8));
    }

    SECTION("voices x vector") {
        op.lhs.set_values(Voices<double>::transposed({3, 4}));
        op.rhs.set_values(Voices<double>::singular({1, 2}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 2);
        REQUIRE(output[0].size() == 2);
        REQUIRE(output[1].size() == 2);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(4, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[0][1]), Catch::Matchers::WithinAbs(5, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][0]), Catch::Matchers::WithinAbs(5, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][1]), Catch::Matchers::WithinAbs(6, 1e-8));
    }

    SECTION("voices x voices - same size") {
        op.lhs.set_values(Voices<double>::transposed({1, 2}));
        op.rhs.set_values(Voices<double>::transposed({3, 4}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 2);
        REQUIRE(output[0].size() == 1);
        REQUIRE(output[1].size() == 1);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(4, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][0]), Catch::Matchers::WithinAbs(6, 1e-8));
    }


    SECTION("voices x voices - lhs bigger") {
        op.lhs.set_values(Voices<double>::transposed({1, 2, 3}));
        op.rhs.set_values(Voices<double>::transposed({2, 3}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 3);
        REQUIRE(output[0].size() == 1);
        REQUIRE(output[1].size() == 1);
        REQUIRE(output[2].size() == 1);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(3, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][0]), Catch::Matchers::WithinAbs(5, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[2][0]), Catch::Matchers::WithinAbs(5, 1e-8));
    }

    SECTION("voices x voices - rhs bigger") {
        op.lhs.set_values(Voices<double>::transposed({2, 3}));
        op.rhs.set_values(Voices<double>::transposed({1, 2, 3}));
        op.operator_node.update_time(TimePoint());
        auto output = op.operator_node.process();
        REQUIRE(output.size() == 3);
        REQUIRE(output[0].size() == 1);
        REQUIRE(output[1].size() == 1);
        REQUIRE(output[2].size() == 1);
        REQUIRE_THAT(static_cast<double>(output[0][0]), Catch::Matchers::WithinAbs(3, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[1][0]), Catch::Matchers::WithinAbs(5, 1e-8));
        REQUIRE_THAT(static_cast<double>(output[2][0]), Catch::Matchers::WithinAbs(5, 1e-8));
    }



}