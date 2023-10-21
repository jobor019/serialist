


#include "core/collections/voices.h"
#include "core/algo/time/trigger.h"
#include "core/generatives/random_pulsator.h"


int main() {
    auto v = Voices<int>{{1, 2, 3}};
    auto u = v;


    auto w = Voices<Trigger>({{Trigger::pulse_on, Trigger::pulse_off}, {Trigger::pulse_on, Trigger::pulse_off}});
    auto x = w;

    ParameterHandler handler;
    RandomPulsatorNode node("", handler, nullptr, nullptr, nullptr, nullptr, nullptr);

    node.process();

    return 0;

}