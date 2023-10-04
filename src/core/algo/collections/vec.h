
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

    // =========================== CONSTRUCTORS ==========================

    Vec() = default;


    explicit Vec(std::vector<T> data) : m_vector(std::move(data)) {
        // TODO: For now. Ideally, Vec's copy ctor should be deleted to avoid accidental copies
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
    }


    Vec(std::initializer_list<T> data) : m_vector(std::move(data)) {
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible"); // TODO: For now
    }


    explicit Vec(const T& value) : m_vector({std::move(value)}) {}


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    static Vec<T> range(T begin, T end, T step = static_cast<T>(1)) {
        // TODO: ensure begin <= end, etc.
        std::vector<T> output;
        T v = begin;

        while (v < end) {
            output.push_back(v);
            v += step;
            if (v == end)
                output.push_back(end);
        }
        return Vec<T>{output};
    }


    static Vec<T>
    repeated(std::size_t repetitions, const T& value) {
        std::vector<T> output(repetitions, value);
        return Vec<T>{output};
    }


    static Vec<T> repeated(std::size_t repetitions, const Vec<T>& values) {
        std::vector<T> output;
        output.reserve(values.size() * repetitions);

        for (std::size_t i = 0; i < repetitions; ++i) {
            output.insert(output.end(), values.m_vector.begin(), values.m_vector.end());
        }
        return Vec<T>(std::move(output));
    }

    // =========================== OPERATORS ==========================

    bool operator==(const Vec<T>& other) const {
        return m_vector == other.m_vector;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T> operator+(const Vec<T>& other) const {
        return elementwise_operation(other, std::plus());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T> operator-(const Vec<T>& other) const {
        return elementwise_operation(other, std::minus());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T> operator*(const Vec<T>& other) const {
        return elementwise_operation(other, std::multiplies());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T> operator/(const Vec<T>& other) const {
        return elementwise_operation(other, std::divides());
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& operator+=(Args... args) {
        add(args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& operator-=(Args... args) {
        subtract(args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& operator*=(Args... args) {
        multiply(args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& operator/=(Args... args) {
        divide(args...);
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
        return Vec<T>(std::move(result));
    }

    // =========================== CLONING / COPYING ==========================

    Vec<T> cloned() const {
        return Vec<T>(m_vector);
    }


    Vec<T> slice(long start = 0, long end = -1) const {
        std::size_t start_idx = sign_index(start);
        std::size_t end_idx = sign_index(end);

        std::vector<T> output;
        output.reserve(end_idx - start_idx);
        for (std::size_t i = start_idx; i < end_idx; ++i) {
            output.push_back(m_vector[i]);
        }

        return Vec<T>(std::move(output));
    }


    Vec<T> drain() {
        std::vector<T> v;
        std::swap(m_vector, v);
        return Vec<T>(std::move(v));
    }


    template<typename U>
    Vec<U> as_type() const {
        std::vector<U> result;
        result.reserve(m_vector.size());
        for (const T& element: m_vector) {
            result.push_back(static_cast<U>(element));
        }
        return Vec<U>(std::move(result));
    }


    // =========================== MUTATORS ==========================


    Vec<T>& append(T value) {
        m_vector.push_back(std::move(value));
        return *this;
    }


    Vec<T>& extend(const Vec<T>& other) {
        m_vector.insert(m_vector.end(), other.m_vector.begin(), other.m_vector.end());
        return *this;
    }


    Vec<T>& concatenate(const Vec<T>& other) {
        m_vector.insert(m_vector.end(), other.m_vector.begin(), other.m_vector.end());
        return *this;
    }


    Vec<T>& remove(const T& value) {
        if (auto it = std::find(m_vector.begin(), m_vector.end(), value); it != m_vector.end()) {
            m_vector.erase(it);
        }
        return *this;
    }


    template<typename SizeType, typename = std::enable_if_t<std::is_integral_v<SizeType>>>
    Vec<T>& erase(SizeType index) {
        long iterator_index;
        if constexpr (std::is_signed_v<SizeType>) {
            iterator_index = static_cast<long>(sign_index(index));
        } else {
            iterator_index = index;
        }
        m_vector.erase(m_vector.begin() + iterator_index);

        return *this;
    }


    template<typename SizeType, typename = std::enable_if_t<std::is_integral_v<SizeType>>>
    Vec<T>& erase(SizeType begin, SizeType end) {
        // TODO: check begin <= end, etc.
        long begin_index;
        long end_index;
        if constexpr (std::is_signed_v<SizeType>) {
            begin_index = static_cast<long>(sign_index(begin));
            end_index = static_cast<long>(sign_index(end));
        } else {
            begin_index = begin;
            end_index = end;
        }
        m_vector.erase(m_vector.begin() + begin_index, m_vector.begin() + end_index);

        return *this;

    }


//    Vec<T>& reserve(std::size_t n) {
//        m_vector.reserve(n);
//        return *this;
//    }


    Vec<T>& resize_append(std::size_t new_size, const T& append_value) {
        if (new_size == 0)
            m_vector.clear();
        else
            resize_internal(new_size, {append_value});

        return *this;
    }


    Vec<T>& resize_extend(std::size_t new_size) {
        assert(!m_vector.empty());

        if (new_size == 0)
            m_vector.clear();
        else
            resize_internal(new_size, {m_vector.back()});

        return *this;
    }


    Vec<T>& resize_fold(std::size_t new_size) {
        if (new_size == 0)
            m_vector.clear();
        else
            resize_internal(new_size, std::nullopt);
        return *this;
    }

    // // =========================== FUNCTIONAL ==========================

    Vec<T>& map(std::function<T(T)> f) {
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector[i] = f(m_vector[i]);
        }
        return *this;
    }


    /**
     *  Removes all elements for which `f` returns false
     */
    Vec<T>& filter(std::function<bool(T)> f) {
        m_vector.erase(std::remove_if(m_vector.begin(), m_vector.end(), [f](const T& element) {
            return !f(element);
        }), m_vector.end());
        return *this;
    }


    Vec<T>& apply(std::function<T(T, T)> f, T value) {
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector[i] = f(m_vector[i], value);
        }
        return *this;
    }


    Vec<T>& apply(std::function<T(T, T)> f, const Vec<T>& values) {
        if (values.size() != m_vector.size()) {
            throw std::logic_error("Cannot apply function to vectors of different sizes");
        }

        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector[i] = f(m_vector[i], values[i]);
        }
        return *this;
    }


    template<typename SizeType>
    Vec<T>& apply(std::function<T(T, T)> f, const Vec<T>& values, const Vec<SizeType>& indices) {
        if (values.size() != indices.size()) {
            throw std::logic_error("values and indices must have the same size");
        }

        Vec<std::size_t> signed_indices;
        if constexpr (std::is_signed_v<SizeType>) {
            signed_indices = sign_indices(indices);
        } else {
            signed_indices = indices.as_type();
        }

        auto next_value_index = 0;
        for (const auto& idx: signed_indices.vector()) {
            m_vector[idx] = f(m_vector[idx], values[next_value_index]);
            next_value_index++;
        }
        return *this;
    }


    Vec<T>& apply(std::function<T(T, T)> f, T value, const Vec<bool>& binary_mask) {
        if (m_vector.size() != binary_mask.size()) {
            throw std::logic_error("binary_mask must have the same size as the internal vector");
        }

        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            if (binary_mask[i]) {
                m_vector[i] = f(m_vector[i], value);
            }
        }
        return *this;
    }


    Vec<T>& apply(std::function<T(T, T)> f, const Vec<T>& values, const Vec<bool>& binary_mask) {
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
        return *this;
    }


    T foldl(std::function<T(T, T)> f, const T& initial) {
        T value = initial;
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            value = f(value, m_vector[i]);
        }
        return value;
    }


    /**
     * Removes all elements for which `f` returns false from the original Vec and returns them as a separate vector
     */
    Vec<T> filter_drain(std::function<bool(T)> f) {
        auto drain_iterator = std::stable_partition(m_vector.begin(), m_vector.end(), f);

        std::vector<T> drained;

        for (auto it = drain_iterator; it != m_vector.end(); ++it) {
            drained.push_back(std::move(*it));
        }

        m_vector.erase(drain_iterator, m_vector.end());
        return Vec<T>(std::move(drained));
    }



    // =========================== MISC ==========================

    void print() const {
        std::cout << "[";
        for (const T& element: m_vector) {
            std::cout << element << ", ";
        }
        std::cout << "]" << std::endl;
    }


    std::size_t size() const {
        return m_vector.size();
    }


    bool contains(const T& value) const {
        return std::find(m_vector.begin(), m_vector.end(), value) != m_vector.end();
    }


    bool contains(const Vec<T>& values) const {
        for (const T& value: values) {
            if (!contains(value)) {
                return false;
            }
        }
        return true;
    }


    bool empty() const {
        return m_vector.empty();
    }


    const std::vector<T>& vector() const {
        return m_vector;
    }


    std::vector<T>& vector_mut() {
        return m_vector;
    }


    std::optional<T> front() const {
        if (m_vector.empty()) {
            return std::nullopt;
        }

        return std::make_optional(m_vector.at(0));
    }


    template<typename U = T>
    U front_or(const U& fallback) const {
        if (m_vector.empty())
            return fallback;
        return static_cast<U>(m_vector.at(0));
    }

    // ======================== ARITHMETIC OPERATIONS ========================



    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& add(Args... args) {
        apply_base(std::plus<T>(), args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& subtract(Args... args) {
        apply_base(std::minus<T>(), args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& multiply(Args... args) {
        apply_base(std::multiplies<T>(), args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& divide(Args... args) {
        apply_base(std::divides<T>(), args...);
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& pow(const T& exponent) {
        apply_base([](const T& a, const T& b) { return std::pow(a, b); }, exponent);
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& normalize() {
        if (auto max_value = max(); max_value != 0.0) {
            auto scale_factor = 1 / max_value;

            for (auto& e: m_vector) {
                e *= scale_factor;
            }
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& min_of(const T& value) {
        for (auto& e: m_vector) {
            e = std::min(e, value);
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& max_of(const T& value) {
        for (auto& e: m_vector) {
            e = std::max(e, value);
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& clip(std::optional<T> low_thresh, std::optional<T> high_thresh) {
        if (low_thresh) {
            max_of(*low_thresh);
        }
        if (high_thresh) {
            min_of(*high_thresh);
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Vec<T>& sort(bool ascending) {
        if (ascending)
            std::sort(m_vector.begin(), m_vector.end());
        else
            std::sort(m_vector.begin(), m_vector.end(), std::greater<>());

        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    T sum() const {
        T result = 0;
        for (const T& element: m_vector) {
            result += element;
        }
        return result;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    T cumsum() const {
        return std::accumulate(m_vector.begin(), m_vector.end(), T(0));
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    T mean() const {
        if (m_vector.empty()) {
            throw std::logic_error("Cannot compute mean of an empty vector");
        }
        return sum() / static_cast<T>(m_vector.size());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    T max() const {
        return *std::max_element(m_vector.begin(), m_vector.end());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    T min() const {
        return *std::min_element(m_vector.begin(), m_vector.end());
    }


    // TODO: Update
//    template<typename U = std::size_t>
//    Vec<U> histogram(std::optional<std::size_t> num_bins
//                     , std::optional<U> low_thresh = std::nullopt
//                     , std::optional<U> high_thresh = std::nullopt) const {
//        static_assert(std::is_arithmetic_v<U>);
//        std::vector<U> histogram(num_bins, 0);
//        auto min_value = low_thresh.has_value()
//                         ? *low_thresh
//                         : static_cast<U>(*std::min_element(m_vector.begin(), m_vector.end()));
//        auto max_value = high_thresh.has_value()
//                         ? *high_thresh
//                         : static_cast<U>(*std::max_element(m_vector.begin(), m_vector.end()));
//
//        double bin_width;
//        if constexpr (std::is_integral_v<U>) {
//            bin_width = (max_value - min_value + 1) / static_cast<double>(num_bins);
//        } else {
//            bin_width = (max_value - min_value) / static_cast<double>(num_bins);
//        }
//
//        for (const T& value: m_vector) {
//            if (low_thresh.has_value() && value < *low_thresh)
//                histogram.front()++;
//            else if (high_thresh.has_value() && value >= *high_thresh)
//                histogram.back()++;
//            else {
//                auto bin_index = static_cast<std::size_t>((value - min_value) / bin_width);
//                histogram.at(bin_index)++;
//            }
//        }
//
//        return Vec<U>(histogram);
//    }


private:

    std::size_t sign_index(long index) const {
        if (index < 0) {
            return static_cast<std::size_t>(static_cast<long>(m_vector.size()) + index);
        }
        return static_cast<std::size_t>(index);
    }


    template<typename SizeType, typename = std::enable_if_t<std::is_integral_v<SizeType> && std::is_signed_v<SizeType>>>
    Vec<std::size_t> sign_indices(const Vec<SizeType>& indices) const {
        std::vector<std::size_t> result;
        result.reserve(indices.size());
        for (auto index: indices.vector()) {
            result.push_back(sign_index(index));
        }
        return Vec<std::size_t>(std::move(result));
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
        return Vec<T>(std::move(output));
    }


    template<typename... Args>
    void apply_base(std::function<T(T, T)> f, Args... args) {
        apply(f, args...);
    }


    void resize_internal(std::size_t new_size, const std::optional<T>& append_value) {
        if (new_size == 0) {
            m_vector.clear();
            return;
        }

        auto diff = static_cast<long>(new_size - m_vector.size());
        if (diff < 0) {
            // remove N last objects
            m_vector.erase(vector().end() + diff, m_vector.end());
        } else if (diff > 0) {
            if (append_value.has_value()) {
                // append
                m_vector.resize(new_size, *append_value);
            } else {
                // fold
                std::size_t original_size = m_vector.size();
                for (std::size_t i = 0; i < static_cast<std::size_t>(diff); ++i) {
                    m_vector.push_back(m_vector.at(i % original_size));
                }
            }
        }
    }


    std::vector<T> m_vector;

};


#endif //SERIALISTLOOPER_VEC_H
