

#ifndef SERIALIST_LOOPER_PHASOR_H
#define SERIALIST_LOOPER_PHASOR_H

#include <cmath>
#include <iostream>
#include <iomanip>


class Phasor {
public:

    enum class Mode {
        stepped, scheduled
    };


    explicit Phasor(double step_size = 0.1
                    , double max = 1.0
                    , double phase = 0.0
                    , Mode mode = Mode::stepped
                    , double current_time = 0.0)
            : step_size(step_size)
              , max(max)
              , current_value(std::fmod(phase, 1.0) * max)
              , mode(mode)
              , previous_update_time(current_time) {}


    double process(double time) {
        double increment;
        if (mode == Mode::stepped) {
            increment = step_size;
        } else {
            increment = step_size * (time - previous_update_time);

        }
        previous_update_time = time;

        double val = current_value + increment;

        // modulo (supports negative numerator but not negative denominator)
        current_value = std::fmod(std::fmod(val, max) + max, max);

        // Handle fmod rounding errors
        if (std::abs(current_value - max) < 1e-8) {
            current_value = 0;
        }

        return current_value;
    }


    void set_phase(double value) {
        current_value = std::fmod(value, 1.0) * max;
    }


    void set_step_size(double value) {
        Phasor::step_size = value;
    }


    void set_max(double value) {
        Phasor::max = value;
    }


    void set_mode(Mode new_mode) {
        mode = new_mode;
    }


    [[nodiscard]]
    double get_step_size() const { return step_size; }

    [[nodiscard]]
    double get_max() const { return max; }

    [[nodiscard]]
    Mode get_mode() const { return mode; }


private:
    double step_size;
    double max;
    double current_value;
    Mode mode;

    double previous_update_time;
};

#endif //SERIALIST_LOOPER_PHASOR_H
