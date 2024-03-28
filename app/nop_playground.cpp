#include <string>
#include "core/collections/scheduler.h"
#include "core/generatives/auto_pulsator.h"

int main() {
//    auto transport = Transport();

    auto time = 0.0;



    double duration = 1.0;
    double legato = 0.5;

    auto pulsator = AutoPulsator(duration, legato);
    pulsator.start(time);

    auto pulse = pulsator.process(time);

    for (std::size_t i = 0; i < 1000; ++i) {
        time += 0.1;
        pulse = pulsator.process(time);
        std::cout << "TIME=" << time << std::endl;
        for (const auto& p: pulse) {
        if (p.is(Trigger::Type::pulse_off)) {
            std::cout << "pulse_OFF" << "(id=" << p.get_id() << ")" << std::endl;
        } else {
            std::cout << "pulse_ON" << "(id=" << p.get_id() << ")" << std::endl;
        }
    }
        std::cout << "----------------" << std::endl;

    }


    return 0;
}