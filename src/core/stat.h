
#ifndef SERIALISTLOOPER_STAT_H
#define SERIALISTLOOPER_STAT_H

#include <iostream>
#include <vector>
#include <functional>
#include <random>

class ContinuousWeightedRandom {
public:
    explicit ContinuousWeightedRandom(std::function<double(double, double, double)> lambda
                                      , double lower_bound = 0.0
                                      , double upper_bound = 1.0
                                      , std::size_t num_values = 100)
            : m_pdf(std::move(lambda))
              , m_lower_bound(lower_bound)
              , m_upper_bound(upper_bound)
              , m_num_values(num_values) {
        recompute();
    }


    double next() {
        auto x = m_distribution(m_rng);

        auto it = std::lower_bound(m_cdf.begin(), m_cdf.end(), x);

        if (it == m_cdf.end()) {
            return m_upper_bound;
        } else {
            auto relative_position = static_cast<double>(it - m_cdf.begin()) / static_cast<double>(m_cdf.size());
            return m_lower_bound + relative_position * (m_upper_bound - m_lower_bound);
        }
    }


    void set_lower_bound(double lower_bound) {
        // TODO: Range check on lower bound
        m_lower_bound = lower_bound;
        recompute();
    }


    void set_upper_bound(double upper_bound) {
        // TODO: Range check on lower bound
        m_upper_bound = upper_bound;
        recompute();
    }


    void set_pdf(const std::function<double(double, double, double)>& pdf) {
        m_pdf = pdf;
        recompute();
    }


    void print_cdf() const {
        std::cout << "[";
        for (const auto& e: m_cdf) {
            std::cout << e << ", ";
        }
        std::cout << "]\n";
    }


    double get_lower_bound() const { return m_lower_bound; }


    double get_upper_bound() const { return m_upper_bound; }


private:
    void recompute() {
        m_cdf.clear();
        m_cdf.reserve(m_num_values);

        double x = m_lower_bound;
        double y = 0.0;
        double step_size = (m_upper_bound - m_lower_bound) / static_cast<double>(m_num_values);


        for (std::size_t i = 0; i < m_num_values; ++i) {
            y += m_pdf(x, m_lower_bound, m_upper_bound);
            m_cdf.emplace_back(y);
            x += step_size;
        }

        auto p_max = m_cdf.at(m_cdf.size() - 1);

        if (p_max <= 0.0) return;

        for (auto& e: m_cdf) {
            e /= p_max;
        }
    }


    std::function<double(double, double, double)> m_pdf;
    double m_lower_bound;
    double m_upper_bound;

    std::size_t m_num_values;

    std::vector<double> m_cdf;

    std::mt19937 m_rng{std::random_device()()};
    std::uniform_real_distribution<> m_distribution{0.0, 1.0};
};

#endif //SERIALISTLOOPER_STAT_H
