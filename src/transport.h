

#ifndef SERIALIST_LOOPER_TRANSPORT_H
#define SERIALIST_LOOPER_TRANSPORT_H

#include <chrono>
#include <cmath>

class Meter {
public:
    explicit Meter(int numerator = 4, int denominator = 4)
            : numerator(numerator), denominator(denominator) {}


    [[nodiscard]]
    double duration() const {
        return numerator / static_cast<double>(denominator);
    }


    [[nodiscard]] int get_numerator() const { return numerator; }

    [[nodiscard]] int get_denominator() const { return denominator; }


private:
    int numerator;
    int denominator;
};


// ==============================================================================================

class TimePoint {
public:
    explicit TimePoint(double tick = 0.0, double tempo = 120.0, double beat = 0.0, Meter meter = Meter())
            : tick(tick), tempo(tempo), beat(beat), meter(meter) {}


    void increment(int64_t delta_ms) {
        double tick_increment = static_cast<double>(delta_ms) * 0.001 * tempo / 60.0;
        tick += tick_increment;
        beat = fmod(beat + tick_increment, meter.duration());

    }


    [[nodiscard]] double get_tick() const { return tick; }

    [[nodiscard]] double get_tempo() const { return tempo; }

    [[nodiscard]] double get_beat() const { return beat; }

    [[nodiscard]] const Meter& get_meter() const { return meter; }


private:
    double tick;
    double tempo;
    double beat;
    Meter meter;
};


// ==============================================================================================

class Transport {
public:
    Transport() : Transport(TimePoint(), false) {}


    explicit Transport(bool active) : Transport(TimePoint(), active) {}


    explicit Transport(TimePoint&& initial_time, bool active)
            : time_point(initial_time)
              , active(active)
              , previous_update_time(std::chrono::system_clock::now()) {}


    void start() {
        active = true;
        previous_update_time = std::chrono::system_clock::now();
    }


    void stop() {
        active = false;
    }


    const TimePoint& update_time() {
        if (active) {
            auto current_time = std::chrono::system_clock::now();
            auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - previous_update_time).count();
            time_point.increment(time_delta);
        }

        return time_point;
    }


private:
    TimePoint time_point;

    bool active;
    std::chrono::time_point<std::chrono::system_clock> previous_update_time;


};

#endif //SERIALIST_LOOPER_TRANSPORT_H
