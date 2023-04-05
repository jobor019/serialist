

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

    virtual ~Oscillator() = default;

    virtual double process(double x) = 0;
};


class Cosine : public Oscillator {
public:
    double process(double x) override {
        return 0.5 * std::cos(2 * M_PI * x) + 0.5;
    }
};


// ==============================================================================================

class Square : public Oscillator {
public:
    explicit Square(double duty = 0.5) : m_duty(duty) {}


    double process(double x) override {
        return static_cast<double>(x <= m_duty);
    }


    [[nodiscard]]
    double get_duty() const { return m_duty; }

    void set_duty(double value) { Square::m_duty = value; }

private:
    double m_duty;
};


// ==============================================================================================

class Triangle : public Oscillator {
public:
    explicit Triangle(double duty = 0.5, double curve = 1.0) : m_duty(duty), m_curve(curve) {
        if (curve <= 0) {
            throw std::range_error("m_curve parameter must be greater than 0");
        }
    }

    double process(double x) override {
        if (m_duty < 1e-8) {                  // m_duty = 0 => negative phase only (avoid div0)
            return std::pow(1 - x, m_curve);
        } else if (x <= m_duty) {             // positive phase
            return std::pow(x / m_duty, m_curve);
        } else {                            // negative phase
            return std::pow(1 - (x - m_duty) / (1 - m_duty), m_curve);
        }
    }

    [[nodiscard]]
    double get_duty() const { return m_duty; }

    void set_duty(double value) { Triangle::m_duty = value; }

    [[nodiscard]]
    double get_curve() const { return m_curve; }

    void set_curve(double value) { Triangle::m_curve = value; }

private:
    double m_duty;
    double m_curve;
};


// ==============================================================================================

class WhiteNoise : public Oscillator {
public:
    double process(double x) override {
        (void) x;
        throw std::runtime_error("this is not implemented yet");
    }


private:
    std::mt19937 m_rng; // TODO: Initialize (and seed) in ctor
};


// ==============================================================================================

class BrownNoise : public Oscillator {
public:
    double process(double x) override {
        (void) x;
        throw std::runtime_error("this is not implemented yet");
    }
};


class BalancedRandom : public Oscillator {
    // TODO Implement something that behaves nicer than true random
    //  (avoiding duplicates, biasing based on what's been generated)

    double process(double x) override {
        (void) x;
        throw std::runtime_error("this is not implemented yet");
    }
};


#endif //SERIALIST_LOOPER_OSCILLATOR_H
