
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/collections/queue.h"

using namespace serialist;

// ============= QUEUE TESTS =============

TEST_CASE("Queue Basic Operations", "[queue]") {
    Queue<int> q(3);
    REQUIRE(q.empty());
    REQUIRE(q.size() == 0);

    q.push(1);
    REQUIRE_FALSE(q.empty());
    REQUIRE(q.size() == 1);
    REQUIRE(q.back() == 1);

    q.push(2);
    q.push(3);
    REQUIRE(q.size() == 3);
    REQUIRE(q.back() == 3);
}

TEST_CASE("Queue FIFO Behavior", "[queue]") {
    Queue<int> q(3);

    q.push(1);
    q.push(2);
    q.push(3);

    auto val1 = q.pop();
    REQUIRE(val1.has_value());
    REQUIRE(val1.value() == 1);  // First in, first out

    auto val2 = q.pop();
    REQUIRE(val2.has_value());
    REQUIRE(val2.value() == 2);

    REQUIRE(q.size() == 1);
    REQUIRE(q.back() == 3);
}

TEST_CASE("Queue Bounded Behavior", "[queue]") {
    Queue<int> q(2);  // Small queue for testing overflow

    q.push(1);
    q.push(2);
    REQUIRE(q.size() == 2);

    q.push(3);  // Should evict oldest (1)
    REQUIRE(q.size() == 2);

    auto val = q.pop();
    REQUIRE(val.has_value());
    REQUIRE(val.value() == 2);  // 1 was evicted, 2 is now oldest
}

TEST_CASE("Queue Empty Operations", "[queue]") {
    Queue<int> q(3);

    auto val = q.pop();
    REQUIRE_FALSE(val.has_value());

    auto all_vals = q.pop_all();
    REQUIRE(all_vals.size() == 0);
}

TEST_CASE("Queue pop_all", "[queue]") {
    Queue<int> q(5);

    q.push(1);
    q.push(2);
    q.push(3);

    auto all_vals = q.pop_all();
    REQUIRE(all_vals.size() == 3);
    REQUIRE(all_vals[0] == 1);  // FIFO order
    REQUIRE(all_vals[1] == 2);
    REQUIRE(all_vals[2] == 3);

    REQUIRE(q.empty());
}

TEST_CASE("Queue get_snapshot", "[queue]") {
    Queue<int> q(5);

    q.push(1);
    q.push(2);
    q.push(3);

    auto snapshot = q.get_snapshot();
    REQUIRE(snapshot.size() == 3);
    REQUIRE(snapshot[0] == 1);
    REQUIRE(snapshot[1] == 2);
    REQUIRE(snapshot[2] == 3);

    // Original queue should be unchanged
    REQUIRE(q.size() == 3);
    REQUIRE_FALSE(q.empty());
}
