
#ifndef SERIALISTLOOPER_RELATIVE_TIME_H
#define SERIALISTLOOPER_RELATIVE_TIME_H

#include <optional>
#include "core/algo/time/time_point.h"
#include "core/utility/optionals.h"


class RelativeDuration /* : public Duration */ {
public:
    static RelativeDuration from_bars(double bars) { return {std::make_optional(bars), std::nullopt}; }

    static RelativeDuration from_beats(double beats) { return {std::nullopt, std::make_optional(beats)}; }

    static RelativeDuration from_bars_beats(double bars, double beats) {
        return {std::make_optional(bars), std::make_optional(beats)};
    }

    friend RelativeDuration operator%(const TimePoint& t, const RelativeDuration& d) {
        if (d.m_bars && !d.m_beats) {
            return {utils::modulo(t.get_bar(), d.m_bars.value()), std::nullopt};

        } else if (!d.m_bars && d.m_beats) {
            // Note: absolute beat when beat alone is used
            return {std::nullopt, utils::modulo(t.get_absolute_beat(), d.m_beats.value())};

        } else {
            // Note: relative beat when used in combination with bar
            return {utils::modulo(t.get_bar(), d.m_bars.value())
                    , utils::modulo(t.get_relative_beat(), d.m_beats.value())};
        }
    }

    RelativeDuration operator+(const RelativeDuration& other) const {
        auto bars = utils::optional_op<double>(m_bars, other.m_bars, [](double a, double b) { return a + b; });
        auto beats = utils::optional_op<double>(m_beats, other.m_beats, [](double a, double b) { return a + b; });
        return {bars, beats};
    }

    bool operator==(const RelativeDuration& other) const {
        return utils::equals(m_bars, other.m_bars) && utils::equals(m_beats, other.m_beats);
    }

    bool operator!=(const RelativeDuration& other) const {
        return !(*this == other);
    }

    bool operator>(const RelativeDuration& other) const {
        return (m_bars > other.m_bars) || (m_bars == other.m_bars && m_beats > other.m_beats);
    }

    bool operator>=(const RelativeDuration& other) const {
        return (m_bars >= other.m_bars) || (m_bars == other.m_bars && m_beats >= other.m_beats);
    }

    bool operator<(const RelativeDuration& other) const {
        return !(*this >= other);
    }

    bool operator<=(const RelativeDuration& other) const {
        return !(*this > other);
    }


    std::optional<double> get_bars() const { return m_bars; }

    std::optional<double> get_beats() const { return m_beats; }


private:
    RelativeDuration(std::optional<double> bars, std::optional<double> beats) : m_bars(bars), m_beats(beats) {
        assert(bars || beats);
    }

    std::optional<double> m_bars;
    std::optional<double> m_beats;
};


// ==============================================================================================



class RelativeTimePoint {
public:
    static RelativeTimePoint next_from(const TimePoint& t, const RelativeDuration& d) {
        return {t.get_bar() + d.get_bars().value_or(0.0)
                , t.get_relative_beat() + d.get_beats().value_or(0.0)};
    }

    static RelativeTimePoint from_time_point(const TimePoint& t) {
        return {t.get_bar(), t.get_relative_beat()};
    }

    bool operator>(const TimePoint& t) const {
        return m_bars > t.get_bar() || (m_bars == t.get_bar() && m_beats > t.get_relative_beat());
    }

    bool operator>=(const TimePoint& t) const {
        return m_bars >= t.get_bar() || (m_bars == t.get_bar() && m_beats >= t.get_relative_beat());
    }

    bool operator<(const TimePoint& t) const {
        return !(*this >= t);
    }

    bool operator<=(const TimePoint& t) const {
        return !(*this > t);
    }

    bool elapsed(const TimePoint& t) const {
        return *this >= t;
    }


private:
    RelativeTimePoint(std::optional<double> bars, std::optional<double> beats) : m_bars(bars), m_beats(beats) {
        assert(m_bars || m_beats);
    }

    std::optional<double> m_bars;
    std::optional<double> m_beats;
};


#endif //SERIALISTLOOPER_RELATIVE_TIME_H
