
#ifndef SERIALIST_DOMAIN_TYPE_H
#define SERIALIST_DOMAIN_TYPE_H

#include <string>

namespace serialist {
enum class DomainType {
    ticks, beats, bars
};


inline std::string domain_type_to_string(DomainType type) {
    switch (type) {
        case DomainType::ticks: return "ticks";
        case DomainType::beats: return "beats";
        case DomainType::bars: return "bars";
    }
    throw std::runtime_error("Unknown domain type");
}

}

#endif //SERIALIST_DOMAIN_TYPE_H
