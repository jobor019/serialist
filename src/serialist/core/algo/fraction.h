
#ifndef SERIALISTLOOPER_FRACTION_H
#define SERIALISTLOOPER_FRACTION_H

#include <cmath>
#include <chrono>

namespace serialist {

class Fraction {
public:
    Fraction(long num, long denom) : n(num), d(denom) {}


    explicit operator double() const {
        return n / static_cast<double>(d);
    }


    long n;
    long d;
};


} // namespace serialist


#endif //SERIALISTLOOPER_FRACTION_H
