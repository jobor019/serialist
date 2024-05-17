#include <string>
#include "core/generatives/auto_pulsator.h"
#include "core/generatives/variable_state_pulsator.h"
#include <iomanip>

void print_pulses(const Voice<Trigger>& pulse) {
    for (const auto& p: pulse) {
        if (p.is(Trigger::Type::pulse_off)) {
            std::cout << "pulse_OFF" << "(id=" << p.get_id() << ")" << std::endl;
        } else {
            std::cout << "pulse_ON" << "(id=" << p.get_id() << ")" << std::endl;
        }
    }
}

int main() {
//    auto transport = Transport();

    VariableStatePulsatorWrapper w;


    return 0;
}