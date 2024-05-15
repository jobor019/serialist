
#ifndef SERIALISTLOOPER_OPTIONALS_H
#define SERIALISTLOOPER_OPTIONALS_H

#include <optional>

namespace utils {

template<typename U, typename T, typename = std::enable_if_t<std::is_constructible_v<U, T>>>
inline std::optional<U> optional_cast(std::optional<T> opt) {
    return opt.has_value() ? std::optional<U>(static_cast<U>(opt.value()))
                           : std::nullopt;
}


// ==============================================================================================

template<typename T>
inline std::optional<T> optional_op(const std::optional<T>& a
                                    , const std::optional<T>& b
                                    , const std::function<T(T, T)>& op) {
    if (!a.has_value()) return b;
    if (!b.has_value()) return a;
    return std::optional<T>(op(a.value(), b.value()));
}

} // namespace utils

#endif //SERIALISTLOOPER_OPTIONALS_H
