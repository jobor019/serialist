#include <string>
#include "core/generatives/auto_pulsator.h"
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

    TimePoint time;

    auto pulsator = AutoPulsator();
    pulsator.start(time, std::nullopt);

    auto pulse = pulsator.poll(time);
    print_pulses(pulse);

    for (std::size_t i = 0; i < 1000; ++i) {
        time.increment(0.1);
        pulse = pulsator.poll(time);
        std::cout << std::setprecision(16) << "tick=" << time.get_tick() << std::endl;
        print_pulses(pulse);
        std::cout << "----------------" << std::endl;

    }


    return 0;
}