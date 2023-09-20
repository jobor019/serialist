#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/sequence.h"
#include "core/interpolator.h"
#include "core/variable.h"

class BasicSequence {
public:
    ParameterHandler handler;

    Sequence<double> sequence{"", handler};
};

TEST_CASE("Trivial") {
    auto seq = BasicSequence();
    auto data = std::vector<double>{60.0, 61.0, 62.0, 63.0};
    seq.sequence.get_parameter_obj().reset_values(data);

    TimePoint t;
    auto interp = InterpolationStrategy<double>(InterpolationStrategy<double>::Type::clip, 0.0);

    std::cout << seq.sequence.process(t, 0.0, interp).at(0) << "\n";
    std::cout << seq.sequence.process(t, 0.25, interp).at(0) << "\n";
    std::cout << seq.sequence.process(t, 0.5, interp).at(0) << "\n";
    std::cout << seq.sequence.process(t, 0.75, interp).at(0) << "\n";

}
