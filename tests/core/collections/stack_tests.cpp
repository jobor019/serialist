

#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/collections/stack.h"

using namespace serialist;

TEST_CASE("Stack Basic Operations", "[stack]") {
    Stack<int> s(3);
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);

    s.push(1);
    REQUIRE_FALSE(s.empty());
    REQUIRE(s.size() == 1);
    REQUIRE(s.top() == 1);

    s.push(2);
    s.push(3);
    REQUIRE(s.size() == 3);
    REQUIRE(s.top() == 3);
}

TEST_CASE("Stack LIFO Behavior", "[stack]") {
    Stack<int> s(3);

    s.push(1);
    s.push(2);
    s.push(3);

    auto val1 = s.pop();
    REQUIRE(val1.has_value());
    REQUIRE(val1.value() == 3);  // Last in, first out

    auto val2 = s.pop();
    REQUIRE(val2.has_value());
    REQUIRE(val2.value() == 2);

    REQUIRE(s.size() == 1);
    REQUIRE(s.top() == 1);
}

TEST_CASE("Stack Bounded Behavior", "[stack]") {
    Stack<int> s(2);  // Small stack for testing overflow

    s.push(1);
    s.push(2);
    REQUIRE(s.size() == 2);
    REQUIRE(s.bottom() == 1);
    REQUIRE(s.top() == 2);

    s.push(3);  // Should evict bottom element (1)
    REQUIRE(s.size() == 2);
    REQUIRE(s.bottom() == 2);  // 2 is now at bottom
    REQUIRE(s.top() == 3);     // 3 is at top
}

TEST_CASE("Stack Empty Operations", "[stack]") {
    Stack<int> s(3);

    auto val = s.pop();
    REQUIRE_FALSE(val.has_value());

    auto all_vals = s.pop_all();
    REQUIRE(all_vals.size() == 0);
}

TEST_CASE("Stack pop_all", "[stack]") {
    Stack<int> s(5);

    s.push(1);
    s.push(2);
    s.push(3);

    auto all_vals = s.pop_all();
    REQUIRE(all_vals.size() == 3);
    REQUIRE(all_vals[0] == 3);  // LIFO order
    REQUIRE(all_vals[1] == 2);
    REQUIRE(all_vals[2] == 1);

    REQUIRE(s.empty());
}

TEST_CASE("Stack get_snapshot", "[stack]") {
    Stack<int> s(5);

    s.push(1);
    s.push(2);
    s.push(3);

    auto snapshot = s.get_snapshot();
    REQUIRE(snapshot.size() == 3);
    REQUIRE(snapshot[0] == 1);  // Bottom to top order
    REQUIRE(snapshot[1] == 2);
    REQUIRE(snapshot[2] == 3);

    // Original stack should be unchanged
    REQUIRE(s.size() == 3);
    REQUIRE_FALSE(s.empty());
}
