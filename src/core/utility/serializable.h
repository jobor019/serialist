

#ifndef SERIALISTLOOPER_SERIALIZABLE_H
#define SERIALISTLOOPER_SERIALIZABLE_H

#include <string>
#include <memory>


template<typename T, typename = void>
struct is_serializable : std::false_type {
};

template<typename T>
struct is_serializable<T, std::void_t<decltype(std::declval<T>().from_string(std::declval<const std::string&>())
        , std::declval<T>().to_string())>> : std::true_type {
};


#endif //SERIALISTLOOPER_SERIALIZABLE_H
