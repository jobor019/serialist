#ifndef TESTUTILS_C11_H
#define TESTUTILS_C11_H
#include "condition.h"


namespace serialist::test::c11 {
template<typename T>
std::unique_ptr<CompareValue<T> > ge(const T& target) {
    return std::make_unique<CompareValue<T> >(std::bind(std::greater_equal<T>(), std::placeholders::_1, target));
}


template<typename U>
std::unique_ptr<CompareValue<Facet> > gef(const U& expected) {
    static_assert(utils::is_static_castable_v<U, Facet>);
    return std::make_unique<CompareValue<Facet> >(std::bind(std::greater_equal<Facet>()
                                                            , std::placeholders::_1
                                                            , static_cast<Facet>(expected)));
}
}
#endif //TESTUTILS_C11_H
