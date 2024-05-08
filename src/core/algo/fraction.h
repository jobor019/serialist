
#ifndef SERIALISTLOOPER_FRACTION_H
#define SERIALISTLOOPER_FRACTION_H

#include <cmath>
#include <chrono>

class Fraction {
public:
    Fraction(int num, int denom) : n(num), d(denom) {}


    explicit operator double() const {
        return n / static_cast<double>(d);
    }


    int n;
    int d;
};

#endif //SERIALISTLOOPER_FRACTION_H
