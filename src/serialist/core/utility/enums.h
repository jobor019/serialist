

#ifndef SERIALIST_LOOPER_UTILS_H
#define SERIALIST_LOOPER_UTILS_H

#include "magic_enum/magic_enum.hpp"

namespace serialist::utils {

template<typename EnumType>
bool enum_is_consecutive_and_zero_indexed() {
    constexpr auto enum_entries = magic_enum::enum_entries<EnumType>();
    if (enum_entries.empty()) {
        return true;
    }

    int prev = static_cast<int>(enum_entries.at(0).first);

    if (prev != 0) {
        return false;
    }

    for (auto it = std::begin(enum_entries) + 1; it != std::end(enum_entries); ++it) {
        int current = static_cast<int>(it->first);
        if (current != prev + 1) {
            return false;
        }
        prev = current;
    }
    return true;
}


} // namespace serialist::utils

#endif //SERIALIST_LOOPER_UTILS_H
