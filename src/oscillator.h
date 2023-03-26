

#ifndef SERIALIST_LOOPER_OSCILLATOR_H
#define SERIALIST_LOOPER_OSCILLATOR_H

#include <cmath>
#include <stdexcept>
#include <random>

class Oscillator {
public:
    /**
     * @throws: std::range_error if invalid parameter ranges are provided
     */
    Oscillator() = default;

    virtual double process(double x) = 0;
};


class Sinusoid : public Oscillator {
public:
    double process(double x) override {
        return std::sin(x);
    }
};


// ==============================================================================================

class Square : public Oscillator {
public:
    explicit Square(double duty = 0.5) : duty(duty) {}


    double process(double x) override {
        return static_cast<double>(x <= duty);
    }


    [[nodiscard]]
    double get_duty() const { return duty; }

    void set_duty(double value) { Square::duty = value; }

private:
    double duty;
};


// ==============================================================================================

class Triangle : public Oscillator {
public:
    explicit Triangle(double duty = 0.5, double curve = 1.0) : duty(duty), curve(curve) {
        if (curve <= 0) {
            throw std::range_error("curve parameter must be greater than 0");
        }
    }

    double process(double x) override {
        if (duty < 1e-8) {                  // duty = 0 => negative phase only (avoid div0)
            return std::pow(1 - x, curve);
        } else if (x <= duty) {             // positive phase
            return std::pow(x / duty, curve);
        } else {                            // negative phase
            return std::pow(1 - (x - duty) / (1 - duty), curve);
        }
    }

    [[nodiscard]]
    double get_duty() const { return duty; }

    void set_duty(double value) { Triangle::duty = value; }

    [[nodiscard]]
    double get_curve() const { return curve; }

    void set_curve(double value) { Triangle::curve = value; }

private:
    double duty;
    double curve;
};


// ==============================================================================================

class WhiteNoise : public Oscillator {
public:
    double process(double x) override {
        throw std::runtime_error("this is not implemented yet");
    }


private:
    std::mt19937 rng; // TODO: Initialize (and seed) in ctor
};


// ==============================================================================================

class BrownNoise : public Oscillator {
public:
    double process(double x) override {
        throw std::runtime_error("this is not implemented yet");
    }
};


class BalancedRandom : public Oscillator {
    // TODO Implement something that behaves nicer than true random
    //  (avoiding duplicates, biasing based on what's been generated)

    double process(double x) override {
        throw std::runtime_error("this is not implemented yet");
    }
};


#endif //SERIALIST_LOOPER_OSCILLATOR_H
