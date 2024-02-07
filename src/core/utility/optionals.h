
#ifndef SERIALISTLOOPER_OPTIONALS_H
#define SERIALISTLOOPER_OPTIONALS_H

#include <optional>

namespace utils {

template<typename U, typename T, typename = std::enable_if_t<std::is_constructible_v<U, T>>>
std::optional<U> optional_cast(std::optional<T> opt) {
    return opt.has_value() ? std::optional<U>(static_cast<U>(opt.value()))
                           : std::nullopt;
}

} // namespace utils

#endif //SERIALISTLOOPER_OPTIONALS_H
