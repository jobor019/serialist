
#ifndef SERIALISTLOOPER_VEC_H
#define SERIALISTLOOPER_VEC_H

#include <vector>
#include <optional>
#include <functional>
#include <iostream>
#include <numeric>


template<typename T>
class Vec {
public:

    explicit Vec(std::vector<T> data) : m_vector(data) {
        static_assert(std::is_arithmetic_v<T>, "T must be arithmetic");
    }


    Vec(std::initializer_list<T> data) : m_vector(data) {
        static_assert(std::is_arithmetic_v<T>, "T must be arithmetic");
    }


    template<typename U>
    Vec<U> histogram(std::size_t num_bins
                     , std::optional<U> low_thresh = std::nullopt
                     , std::optional<U> high_thresh = std::nullopt) const {
        std::vector<U> histogram(num_bins, 0);
        auto min_value = low_thresh.has_value()
                         ? *low_thresh
                         : static_cast<U>(*std::min_element(m_vector.begin(), m_vector.end()));
        auto max_value = high_thresh.has_value()
                         ? *high_thresh
                         : static_cast<U>(*std::max_element(m_vector.begin(), m_vector.end()));

        double bin_width;
        if constexpr (std::is_integral_v<U>) {
            bin_width = (max_value - min_value + 1) / static_cast<double>(num_bins);
        } else {
            bin_width = (max_value - min_value) / static_cast<double>(num_bins);
        }

        for (const T& value: m_vector) {
            if (low_thresh.has_value() && value < *low_thresh)
                histogram.front()++;
            else if (high_thresh.has_value() && value >= *high_thresh)
                histogram.back()++;
            else {
                auto bin_index = static_cast<std::size_t>((value - min_value) / bin_width);
                histogram.at(bin_index)++;
            }
        }

        return Vec<U>(histogram);
    }


    static Vec<T> create_empty_like(std::size_t size) {
        return Vec<T>(std::vector<T>(size, 0.0));
    }


    Vec<T> operator+(const Vec<T>& other) const {
        return elementwise_operation(other, std::plus());
    }


    Vec<T> operator-(const Vec<T>& other) const {
        return elementwise_operation(other, std::minus());
    }


    Vec<T> operator*(const Vec<T>& other) const {
        return elementwise_operation(other, std::multiplies());
    }


    Vec<T> operator/(const Vec<T>& other) const {
        return elementwise_operation(other, std::divides());
    }


    template<typename... Args>
    Vec<T>& operator+=(Args... args) {
        add(args...);
        return *this;
    }


    template<typename... Args>
    Vec<T>& operator-=(Args... args) {
        add(args...);
        return *this;
    }


    template<typename... Args>
    Vec<T>& operator*=(Args... args) {
        add(args...);
        return *this;
    }


    template<typename... Args>
    Vec<T>& operator/=(Args... args) {
        add(args...);
        return *this;
    }


    Vec<T>& operator-=(const Vec<T>& other) {
        apply(std::minus<T>(), other);
        return *this;
    }


    Vec<T>& operator*=(const Vec<T>& other) {
        apply(std::multiplies<T>(), other);
        return *this;
    }


    Vec<T>& operator/=(const Vec<T>& other) {
        apply(std::divides<T>(), other);
        return *this;
    }


    template<typename U>
    T& operator[](const U& index) {
        if constexpr (std::is_signed_v<U>) {
            return m_vector.at(sign_index(index));
        } else {
            return m_vector.at(index);
        }
    }


    template<typename U>
    T operator[](const U& index) const {
        if constexpr (std::is_signed_v<U>) {
            return m_vector.at(sign_index(index));
        } else {
            return m_vector.at(index);
        }
    }


    Vec<T> operator[](const Vec<T>& indices) const {
        std::vector<int> result;
        result.reserve(indices.size());

        for (auto index: indices) {
            result.push_back(m_vector.at(sign_index(index)));
        }
        return Vec<T>(result);
    }


    void apply(std::function<T(T, T)> f, T value) {
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector[i] = f(m_vector[i], value);
        }
    }


    void apply(std::function<T(T, T)> f, const Vec<T>& values) {
        if (values.size() != m_vector.size()) {
            throw std::logic_error("Cannot apply function to vectors of different sizes");
        }

        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector[i] = f(m_vector[i], values[i]);
        }
    }


    void apply(std::function<T(T, T)> f, const Vec<T>& values, const Vec<long>& indices) {
        if (values.size() != indices.size()) {
            throw std::logic_error("values and indices must have the same size");
        }

        auto signed_indices = sign_indices(indices);
        auto next_value_index = 0;
        for (const auto& idx: signed_indices.vector()) {
            m_vector[idx] = f(m_vector[idx], values[next_value_index]);
            next_value_index++;
        }
    }


    void apply(std::function<T(T, T)> f, T value, const Vec<bool>& binary_mask) {
        if (m_vector.size() != binary_mask.size()) {
            throw std::logic_error("binary_mask must have the same size as the internal vector");
        }

        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            if (binary_mask[i]) {
                m_vector[i] = f(m_vector[i], value);
            }
        }
    }


    void apply(std::function<T(T, T)> f, const Vec<T>& values, const Vec<bool>& binary_mask) {
        if (m_vector.size() != binary_mask.size()) {
            throw std::logic_error("binary_mask must have the same size as the internal vector");
        }

        T next_index = 0;
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            if (binary_mask[i]) {
                m_vector[i] = f(m_vector[i], values[next_index]);
                next_index++;
            }
        }
    }


    template<typename... Args>
    void add(Args... args) {
        apply_base(std::plus<T>(), args...);
    }


    template<typename... Args>
    void subtract(Args... args) {
        apply_base(std::minus<T>(), args...);
    }


    template<typename... Args>
    void multiply(Args... args) {
        apply_base(std::multiplies<T>(), args...);
    }


    template<typename... Args>
    void divide(Args... args) {
        apply_base(std::divides<T>(), args...);
    }


    Vec<T> slice(long start = 0, long end = -1) const {
        std::size_t start_idx = sign_index(start);
        std::size_t end_idx = sign_index(end);

        std::vector<T> output;
        output.reserve(end_idx - start_idx);
        for (std::size_t i = start_idx; i < end_idx; ++i) {
            output.push_back(m_vector[i]);
        }

        return Vec<T>(output);
    }


    void print() const {
        std::cout << "[";
        for (const T& element: m_vector) {
            std::cout << element << ", ";
        }
        std::cout << "]" << std::endl;
    }


    T sum() const {
        T result = 0;
        for (const T& element: m_vector) {
            result += element;
        }
        return result;
    }


    T cumsum() const {
        return std::accumulate(m_vector.begin(), m_vector.end(), T(0));
    }


    T mean() const {
        if (m_vector.empty()) {
            throw std::logic_error("Cannot compute mean of an empty vector");
        }
        return sum() / static_cast<T>(m_vector.size());
    }


    Vec<T> pow(const T& exponent) const {
        std::vector<T> result;
        result.reserve(m_vector.size());
        for (const T& element: m_vector) {
            result.push_back(static_cast<T>(std::pow(element, exponent)));
        }
        return Vec<T>(result);
    }


    template<typename U>
    Vec<U> as_type() const {
        std::vector<U> result;
        result.reserve(m_vector.size());
        for (const T& element: m_vector) {
            result.push_back(static_cast<U>(element));
        }
        return Vec<U>(result);
    }


    void normalize() {
        if (auto max_value = max(); max_value != 0.0) {
            auto scale_factor = 1 / max_value;

            for (auto& e: m_vector) {
                e *= scale_factor;
            }
        }
    }


    Vec<T> sort(bool ascending) const {
        std::vector<T> result = m_vector;
        if (ascending) {
            std::sort(result.begin(), result.end());
        } else {
            std::sort(result.begin(), result.end(), std::greater<T>());
        }
        return Vec<T>(result);
    }


    T max() const {
        return *std::max_element(m_vector.begin(), m_vector.end());
    }


    T min() const {
        return *std::min_element(m_vector.begin(), m_vector.end());
    }


    std::size_t size() const {
        return m_vector.size();
    }


    const std::vector<T>& vector() const {
        return m_vector;
    }


private:

    std::size_t sign_index(long index) const {
        if (index < 0) {
            return static_cast<std::size_t>(static_cast<long>(m_vector.size()) + index);
        }
        return static_cast<std::size_t>(index);
    }


    Vec<std::size_t> sign_indices(Vec<long> indices) const {
        std::vector<std::size_t> result;
        result.reserve(indices.size());
        for (auto index : indices.vector()) {
            result.push_back(sign_index(index));
        }
        return Vec<std::size_t>{result};
    }


    Vec<T> elementwise_operation(const Vec<T>& other, std::function<T(T, T)> op) const {
        if (m_vector.size() != other.m_vector.size()) {
            throw std::out_of_range("vectors must have the same size for element-wise operation");
        }

        std::vector<T> output;
        output.reserve(m_vector.size());
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            output.push_back(op(m_vector.at(i), other.m_vector.at(i)));
        }
        return Vec<T>(output);
    }


    template<typename... Args>
    void apply_base(std::function<T(T, T)> f, Args... args) {
        apply(f, args...);
    }


    std::vector<T> m_vector;

};


#endif //SERIALISTLOOPER_VEC_H
