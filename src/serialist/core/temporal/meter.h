#ifndef SERIALISTLOOPER_METER_H
#define SERIALISTLOOPER_METER_H

#include "domain_type.h"
#include "exceptions.h"
#include "core/utility/math.h"
#include "core/algo/fraction.h"


namespace serialist {
class Meter {
public:
    Meter() : m_fraction(4, 4) {}

    Meter(int numerator, int denominator) : m_fraction(numerator, denominator) {}

    bool operator==(const Meter& other) const { return m_fraction == other.m_fraction; }
    bool operator!=(const Meter& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Meter& obj) {
        os << obj.to_string();
        return os;
    }


    double duration() const { return 4.0 * static_cast<double>(m_fraction); }
    int get_numerator() const { return static_cast<int>(m_fraction.n); }
    int get_denominator() const { return static_cast<int>(m_fraction.d); }
    double subdivision_duration() const { return 4.0 / static_cast<double>(m_fraction.d); }


    double ticks2bars(double ticks) const { return ticks / duration(); }
    double bars2ticks(double bars) const { return bars * duration(); }


    double ticks2beats(double ticks) const { return ticks / subdivision_duration(); }
    double beats2ticks(double beats) const { return beats * subdivision_duration(); }


    double beats2bars(double beats) const { return beats / static_cast<double>(m_fraction.n); }
    double bars2beats(double bars) const { return bars * static_cast<double>(m_fraction.n); }


    std::pair<double, double> ticks2bars_beats(double ticks) const {
        auto beats = ticks2beats(ticks);
        return utils::divmod(beats, duration());
    }


    double bars_beats2ticks(double bars, double beats) const {
        return bars2ticks(bars) + beats2ticks(beats);
    }

    double ticks2domain(double ticks, DomainType domain) const {
        switch (domain) {
            case DomainType::ticks: return ticks;
            case DomainType::bars: return ticks2bars(ticks);
            case DomainType::beats: return ticks2beats(ticks);
            default: throw TimeDomainError("Unsupported domain type");
        }
    }

    double domain2ticks(double ticks, DomainType domain) const {
        switch (domain) {
            case DomainType::ticks: return ticks;
            case DomainType::bars: return bars2ticks(ticks);
            case DomainType::beats: return beats2ticks(ticks);
            default: throw TimeDomainError("Unsupported domain type");
        }
    }

    double domain2domain(double value, DomainType from, DomainType to) const {
        if (from == to) {
            return value;
        }
        return domain2ticks(ticks2domain(value, from), to);
    }


    std::string to_string() const {
        return std::to_string(m_fraction.n) + "/" + std::to_string(m_fraction.d);
    }

private:
    Fraction m_fraction;
};
}

#endif //SERIALISTLOOPER_METER_H
