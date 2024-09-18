#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/policies/policies.h"
#include "core/generatives/sequence.h"
#include "core/algo/pitch/notes.h"
#include "core/algo/facet.h"



using namespace serialist;

template<typename StoredType, typename OutputType = StoredType>
class BasicSequence {
public:
    ParameterHandler handler;

    Sequence<StoredType, OutputType> sequence{"", handler};
};

TEST_CASE("Sequence initialization") {
    auto seq = BasicSequence<double>();

    auto data = Voices<double>::transposed({60.0, 61.0, 62.0, 63.0});
    seq.sequence.set_values(data);

    seq.sequence.process().print();

    auto seq2 = BasicSequence<Facet>();

    auto data2 = Voices<Facet>::transposed(Vec<int>::range(12).as_type<Facet>());
    seq2.sequence.set_values(data2);

    auto seq3 = BasicSequence<Facet, NoteNumber>();

    auto data3 = Voices<NoteNumber>::transposed(Vec<NoteNumber>::range(12));
    seq3.sequence.set_values(data3);
}