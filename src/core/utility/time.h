//
//#ifndef SERIALISTLOOPER_TIME_H
//#define SERIALISTLOOPER_TIME_H
//
//#include "core/algo/time/meter.h"
//
//namespace utils {
//
//inline double ticks2bars(double ticks, const Meter& m) {
//        return ticks / m.duration();
//    }
//
//    inline double bars2ticks(double bars, const Meter& m) {
//        return bars * m.duration();
//    }
//
//    inline double ticks2beats(double ticks, const Meter& m) {
//        return ticks / m.subdivision_duration();
//    }
//
//    inline double beats2ticks(double beats, const Meter& m) {
//        return beats * m.subdivision_duration();
//    }
//
//    inline double beats2bars(double beats, const Meter& m) {
//        return beats / m.get_numerator();
//    }
//    inline double bars2beats(double bars, const Meter& m) {
//        return bars * m.get_numerator();
//    }
//
//    inline std::pair<double, double> ticks2bars_beats(double ticks, const Meter& m) {
//        auto beats = ticks2beats(ticks, m);
//        return utils::divmod(beats, m.duration());
//    }
//
//    inline double bars_beats2ticks(double bars, double beats, const Meter& m) {
//        return bars2ticks(bars, m) + beats2ticks(beats, m
//        );
//    }
//
//} // utils
//
//#endif //SERIALISTLOOPER_TIME_H
