//
//#ifndef SERIALISTLOOPER_DOMAIN_TIME_H
//#define SERIALISTLOOPER_DOMAIN_TIME_H
//
//#include "time_point.h"
//
//class DomainDuration;
//
//class DomainTimePoint {
//public:
//    DomainTimePoint(double value, DomainType type) : m_value(value), m_type(type) {}
//
//    static DomainTimePoint from_time_point(const TimePoint& t, DomainType type) { return {t.get(type), type}; }
//
//    static DomainTimePoint zero() { return {0.0, DomainType::ticks}; }
//
//    DomainTimePoint as_type(DomainType target_type, const TimePoint& last_transport_tp) const;
//
//    DomainDuration distance_from(double time_value, DomainType type) const;
//
//
//};
//
//#endif //SERIALISTLOOPER_DOMAIN_TIME_H
