
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/collections/voices.h"
#include "core/algo/facet.h"
#include "core/param/parameter_policy.h"
#include "core/generatives/sequence.h"
#include "core/generatives/OLD_oscillator.h"
//#include "core/parameter_policy.h"
//#include "core/sequence.h"

TEST_CASE("VTParameter") {}


TEST_CASE("VTSequenceParameter") {
    Voice<Facet> facets{{Facet(1), Facet(2), Facet(3)}};
//    std::vector<int> v2 = v.vector_as<int>();
//
//    for (int i : v2) {
//        std::cout << i << "\n";
//    }
//
//    auto vv = Voices<Facet>::transposed(v);
////    Voices<int> vv(v2);



    juce::UndoManager um;
    ParameterHandler handler(um);


    VTSequenceParameter<Facet, float> sequence("test", handler, {});
    Sequence<Facet, float> seq2("test2", handler);

    std::vector<std::vector<float>> values { {1, 2, 3}, {4, 5, 6} };

    seq2.set_values(values);

    std::cout << seq2.get_parameter_handler().get_value_tree().toXmlString() << "\n";

    std::vector<float> v3 = {1, 2, 3};

    seq2.set_transposed(v3);

//    seq2.set_values({v3});

    std::cout << seq2.get_parameter_handler().get_value_tree().toXmlString() << "\n";

    Sequence<Facet, bool> seq3("test3", handler, {true, false, true});
    seq3.set_transposed({true, false, true});
//    std::cout << seq3.get_parameter_handler().get_value_tree().toXmlString() << "\n";

    Sequence<Facet, OscillatorNode::Type> seq4("test4", handler, OscillatorNode::Type::sin);

}