

#ifndef SERIALIST_LOOPER_OSCILLATOR_H
#define SERIALIST_LOOPER_OSCILLATOR_H

#include <cmath>
#include <stdexcept>
#include <random>
#include "parameter_policy.h"
#include "generative.h"

class Oscillator : public Node<double>
        ,  public ParameterHandler {
public:
    /**
     * @throws: std::range_error if invalid parameter ranges are provided
     */
    Oscillator(const std::string& identifier, VTParameterHandler& parent): ParameterHandler(identifier, parent) {}

    virtual ~Oscillator() = default;
    Oscillator(const Oscillator&) = delete;
    Oscillator& operator=(const Oscillator&) = delete;
    Oscillator(Oscillator&&)  noexcept = default;
    Oscillator& operator=(Oscillator&&)  noexcept = default;

    virtual double process(double x) = 0;
};


// ==============================================================================================

class Identity : public Oscillator {
public:
    double process(double x) override {
        return x;
    }
};


// ==============================================================================================

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
    BrownNoise() : last_output_(0.0), distribution_(-0.5, 0.5), max_difference_(0.01) {
        generator_ = std::mt19937(rd_());
    }

    double process(double x) override {
        (void) x;
        double white_noise = distribution_(generator_);
        double new_output = last_output_ + (white_noise - last_output_) / 16.0;
        double difference = std::abs(new_output - last_output_);
        if (difference > max_difference_) {
            new_output = last_output_ + max_difference_ * std::copysign(1.0, new_output - last_output_);
        }
        last_output_ = new_output;
        return last_output_ + 0.5;
    }

    void set_max_difference(double max_difference) {
        max_difference_ = max_difference;
    }

private:
    double last_output_;
    std::mt19937 generator_;
    std::uniform_real_distribution<double> distribution_;
    std::random_device rd_;
    double max_difference_;
};


// ==============================================================================================

class RandomWalkOscillator : public Oscillator {
public:
    RandomWalkOscillator() : Oscillator(), rng_(std::random_device{}()) {}

    double process(double x) override {
        (void) x;
        double next = current_ + distribution_(rng_);
        if (next > 1.0) {
            current_ = 2.0 - next;
        } else if (next < 0.0) {
            current_ = -next;
        } else {
            current_ = next;
        }
        return current_;
    }

    void setStepSize(double stepSize) {
        if (stepSize <= 0.0 || stepSize >= 1.0) {
            throw std::range_error("Step size must be between 0 and 1.");
        }
        distribution_ = std::uniform_real_distribution<double>(-stepSize, stepSize);
    }

private:
    std::mt19937 rng_;
    std::uniform_real_distribution<double> distribution_{-0.05, 0.05};  // default step size
    double current_ = 0.5;  // starting value
};


// ==============================================================================================

class BalancedRandom : public Oscillator {
    // TODO Implement something that behaves nicer than true random
    //  (avoiding duplicates, biasing based on what's been generated)

    double process(double x) override {
        (void) x;
        throw std::runtime_error("this is not implemented yet");
    }
};


#endif //SERIALIST_LOOPER_OSCILLATOR_H
