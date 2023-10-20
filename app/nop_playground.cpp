


#include "core/collections/voices.h"
#include "core/algo/time/trigger.h"


int main() {
    auto v = Voices<int>{{1, 2, 3}};
    auto u = v;


    auto w = Voices<Trigger>({{Trigger::pulse_on, Trigger::pulse_off}, {Trigger::pulse_on, Trigger::pulse_off}});
    auto x = w;

    return 0;

}