

#ifndef SERIALIST_LOOPER_TRANSPORT_H
#define SERIALIST_LOOPER_TRANSPORT_H

#include <chrono>
#include <cmath>

class Fraction {
public:
    Fraction(int num, int denom) : n(num), d(denom) {}


    explicit operator double() const {
        return n / static_cast<double>(d);
    }


    int n;
    int d;
};

class Meter {
public:
    explicit Meter(int numerator = 4, int denominator = 4) : m_fraction(numerator, denominator) {}


    double duration() const { return static_cast<double>(m_fraction); }


    int get_numerator() const { return m_fraction.n; }


    int get_denominator() const { return m_fraction.d; }

    double subdivision_duration() const {
        return 1.0 / static_cast<double>(m_fraction.d);
    }


private:
    Fraction m_fraction;
};


// ==============================================================================================

class TimePoint {
public:
    explicit TimePoint(double tick = 0.0, double tempo = 120.0, double beat = 0.0, Meter meter = Meter())
            : m_tick(tick), m_tempo(tempo), m_beat(beat), m_meter(meter) {}


    void increment(int64_t delta_nanos) {
        increment(static_cast<double>(delta_nanos) * 1e-9 * m_tempo / 60.0);
    }

    void increment(double tick_increment) {
        m_tick += tick_increment;
        m_beat = fmod(m_beat + tick_increment, m_meter.duration());
    }


    double next_tick_of(const Fraction& quantization_level = {1, 4}) const {
        auto q = static_cast<double>(quantization_level);
        auto diff = fmod(m_beat, q);

        if (diff < 1e-4)
            return m_tick - diff; // schedule up to 0.0001 ticks in the past

        return m_tick - diff + q;   // schedule on next quantization level
    }

    bool operator<(const TimePoint& other) const {
        return m_tick < other.m_tick;
    }

    bool operator<=(const TimePoint& other) const {
        return m_tick <= other.m_tick;
    }

    bool operator>(const TimePoint& other) const {
        return m_tick > other.m_tick;
    }

    bool operator>=(const TimePoint& other) const {
        return m_tick >= other.m_tick;
    }


    [[nodiscard]] double get_tick() const { return m_tick; }


    [[nodiscard]] double get_tempo() const { return m_tempo; }


    [[nodiscard]] double get_beat() const { return m_beat; }


    [[nodiscard]] const Meter& get_meter() const { return m_meter; }


private:
    double m_tick;
    double m_tempo;
    double m_beat;
    Meter m_meter;
};


// ==============================================================================================

class Transport {
public:
    Transport() : Transport(TimePoint(), false) {}


    explicit Transport(bool active) : Transport(TimePoint(), active) {}


    explicit Transport(TimePoint&& initial_time, bool active)
            : m_time_point(initial_time)
              , m_active(active)
              , m_previous_update_time(std::chrono::system_clock::now()) {}


    void start() {
        m_active = true;
        m_previous_update_time = std::chrono::system_clock::now();
    }


    void stop() {
        m_active = false;
    }


    const TimePoint& update_time() {
        if (m_active) {
            auto current_time = std::chrono::system_clock::now();
            auto time_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    current_time - m_previous_update_time).count();
            m_time_point.increment(time_delta);
            m_previous_update_time = current_time;
        }

        return m_time_point;
    }


private:
    TimePoint m_time_point;

    bool m_active;
    std::chrono::time_point<std::chrono::system_clock> m_previous_update_time;


};

#endif //SERIALIST_LOOPER_TRANSPORT_H
