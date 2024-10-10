#ifndef SERIALISTLOOPER_VEC_H
#define SERIALISTLOOPER_VEC_H

#include <vector>
#include <optional>
#include <functional>
#include <iostream>
#include <numeric>
#include "core/utility/math.h"
#include "core/utility/traits.h"

namespace serialist {

template<typename T>
class Vec {
public:
    // =========================== CONSTRUCTORS ==========================

    using value_type = T;
    using size_type = std::size_t;
    using allocator_type = std::allocator<T>;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    Vec() = default;


    explicit Vec(std::vector<T> data) : m_vector(std::move(data)) {
        // TODO: For now. Ideally, Vec's copy ctor should be deleted to avoid accidental copies
        //        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
    }


    template<typename E = T, typename std::enable_if_t<std::is_copy_constructible_v<E>, int>  = 0>
    Vec(std::initializer_list<T> data) : m_vector(std::move(data)) {
        //        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible"); // TODO: For now
    }


    template<typename... Args
             , typename std::enable_if_t<((!std::is_copy_constructible_v<Args> && std::is_constructible_v<T, Args>)
                                             && ...), int>  = 0>
    explicit Vec(Args&&... args) {
        (m_vector.emplace_back(std::forward<Args>(args)), ...);
    }


    static Vec<T> singular(const T& value) {
        return Vec<T>({value});
    }


    static Vec<T> allocated(std::size_t size) {
        Vec<T> v;
        v.vector_mut().reserve(size);
        return std::move(v);
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    static Vec<T> range(T begin, T end, T step = static_cast<T>(1)) {
        // TODO: ensure begin <= end, etc.
        std::vector<T> output;
        T v = begin;

        while (v < end) {
            output.push_back(v);
            v += step;
        }
        return Vec<T>{output};
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    static Vec<T> range(T end) {
        return Vec<T>::range(0, end);
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    static Vec<T> linspace(T start, T end, std::size_t num, bool include_endpoint = true) {
        if (num == 0) {
            return {};
        } else if (num == 1) {
            return Vec<T>::singular(start);
        }

        double step;
        if (include_endpoint) {
            step = static_cast<double>(end - start) / static_cast<double>(num - 1);
        } else {
            step = static_cast<double>(end - start) / static_cast<double>(num);
        }

        auto result = Vec<T>::allocated(num);
        for (std::size_t i = 0; i < num; ++i) {
            result.append(static_cast<T>(start + static_cast<T>(i) * step));
        }
        return result;
    }


    static Vec<T> repeated(std::size_t repetitions, const T& value) {
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


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    static Vec<T> ones(std::size_t size) {
        return Vec<T>::repeated(size, static_cast<T>(1));
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    static Vec<T> zeros(std::size_t size) {
        return Vec<T>::repeated(size, static_cast<T>(0));
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    static Vec<T> one_hot(const T& value, std::size_t index, std::size_t size) {
        auto v = Vec<T>::zeros(size);
        v[index] = value;
        return std::move(v);
    }


    // TODO: Quite a bit of code duplication in these two functions
    /**
     * Takes a lambda function with any number of arguments and any number of Vecs of any type corresponding to the
     * types of the arguments of the lambda, and computes the function for each entry in the vector
     * after broadcasting the size of the Vecs to the largest size.
     *
     * Example:
     * @code
     *   auto lambda = [](int a, double b, const std::string& c) {
     *       return std::to_string(a) + c + std::to_string(b);
     *   };
     *
     *   auto a = Vec<int>({1, 2, 3, 4});
     *   auto b = Vec<double>({4.0, 5.0, 6.0});
     *   auto c = Vec<std::string>({"a", "b"});
     *
     *   auto result = Vec<std::string>::broadcast_apply(lambda, std::move(a), std::move(b), std::move(c));
     * @endcode
     */
    template<typename Callable
             , typename... Containers
             , typename = std::enable_if_t<(std::is_same_v<Containers, Vec<typename Containers::value_type> > && ...)> >
    static auto broadcast_apply(Callable&& func
                                , Containers&&... containers
    ) -> Vec<decltype(std::forward<Callable>(func)(*(containers.begin())...))> {
        using ResultType = decltype(std::forward<Callable>(func)(*(containers.begin())...));

        auto max_size = std::max({containers.size()...});

        ((containers.resize_fold(max_size)), ...);

        auto results = Vec<ResultType>::allocated(max_size);

        for (size_t i = 0; i < max_size; ++i) {
            results.append(std::forward<Callable>(func)((*(containers.begin() + i))...));
        }

        return results;
    }


    /**
     * Same as broadcast_apply but without moving / mutating the original Vecs
     * @see broadcast_apply
     */
    template<typename Callable
             , typename... Containers
             , typename = std::enable_if_t<(std::is_same_v<Containers, Vec<typename Containers::value_type> > && ...)> >
    static auto broadcast_apply(Callable&& func, const Containers&... containers) -> Vec<
        decltype(std::forward<Callable>(func)(*(containers.begin())...))> {
        using ResultType = decltype(std::forward<Callable>(func)(*(containers.begin())...));

        auto max_size = std::max({containers.size()...});

        auto resized_containers = std::make_tuple(std::move(containers.cloned().resize_fold(max_size))...);

        auto results = Vec<ResultType>::allocated(max_size);

        for (size_t i = 0; i < max_size; ++i) {
            results.append(std::apply([&func, i](auto&&... args) {
                return std::forward<Callable>(func)(*(args.begin() + i)...);
            }, resized_containers));
        }

        return results;
    }


    // =========================== OPERATORS ==========================

    bool operator==(const Vec<T>& other) const {
        if constexpr (std::is_floating_point_v<T>) {
            return approx_equals(other, 1e-8);
        }
        return m_vector == other.m_vector;
    }


    bool operator!=(const Vec<T>& other) const {
        return m_vector != other.m_vector;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator+(const T& operand2) const {
        return elementwise_operation(operand2, std::plus());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator+(const Vec<T>& other) const {
        return elementwise_operation(other, std::plus());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator-(const Vec<T>& other) const {
        return elementwise_operation(other, std::minus());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator-(const T& operand2) const {
        return elementwise_operation(operand2, std::minus());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator*(const Vec<T>& other) const {
        return elementwise_operation(other, std::multiplies());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator*(const T& operand2) const {
        return elementwise_operation(operand2, std::multiplies());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator/(const Vec<T>& other) const {
        return elementwise_operation(other, std::divides());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T> operator/(const T& operand2) const {
        return elementwise_operation(operand2, std::divides());
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& operator+=(Args... args) {
        add(args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& operator-=(Args... args) {
        subtract(args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& operator*=(Args... args) {
        multiply(args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& operator/=(Args... args) {
        divide(args...);
        return *this;
    }


    template<typename U>
    decltype(auto) operator[](const U& index) {
        if constexpr (std::is_signed_v<U>) {
            return m_vector.at(sign_index(index));
        } else {
            return m_vector.at(index);
        }
    }


    template<typename U>
    decltype(auto) operator[](const U& index) const {
        if constexpr (std::is_signed_v<U>) {
            return m_vector.at(sign_index(index));
        } else {
            return m_vector.at(index);
        }
    }


    template<typename U>
    Vec<T> operator[](const Vec<U>& indices) const {
        std::vector<T> result;
        result.reserve(indices.size());

        for (auto index: indices) {
            result.push_back(m_vector.at(sign_index(index)));
        }
        return Vec<T>(std::move(result));
    }


    // TODO: Should probably use `iterator` and `const_iterator` instead of decltype(auto) for consistency

    decltype(auto) begin() { return m_vector.begin(); }


    decltype(auto) begin() const { return m_vector.begin(); }


    decltype(auto) cbegin() const noexcept { return m_vector.cbegin(); }


    decltype(auto) end() { return m_vector.end(); }


    decltype(auto) end() const { return m_vector.end(); }


    decltype(auto) cend() const noexcept { return m_vector.cend(); }


    decltype(auto) back() { return m_vector.back(); }


    decltype(auto) back() const { return m_vector.back(); }


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
        std::vector<U> output;
        output.reserve(m_vector.size());
        for (const T& element: m_vector) {
            output.push_back(static_cast<U>(element));
        }
        return Vec<U>(std::move(output));
    }


    /**
     * note: requires explicit template argument to be called, cannot be inferred from `f`
     */
    template<typename U>
    Vec<U> as_type(std::function<U(const T&)> f) const {
        std::vector<U> output;
        output.reserve(m_vector.size());
        for (const T& element: m_vector) {
            output.push_back(f(element));
        }
        return Vec<U>(std::move(output));
    }


    template<typename E = T, typename = std::enable_if_t<std::is_integral_v<E> > >
    Vec<bool> boolean_mask(std::optional<std::size_t> size = std::nullopt) const {
        Vec<bool> output = Vec<bool>::repeated(size.value_or(max() + 1), false);
        for (const auto& index: m_vector) {
            output[index] = true;
        }
        return output;
    }


    template<typename U, typename E = T, typename = std::enable_if_t<std::is_same_v<E, bool> > >
    Vec<U> index_map() {
        Vec<T> output;
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            if (m_vector[i])
                output.append(i);
        }
        return output;
    }


    // =========================== MUTATORS ==========================


    Vec<T>& append(T value) {
        m_vector.push_back(std::move(value));
        return *this;
    }


    Vec<T>& insert(long index, T value) {
        auto signed_index = sign_index(index);

        if (signed_index >= size()) {
            return append(std::move(value));
        }

        m_vector.insert(m_vector.begin() + static_cast<long>(signed_index), std::move(value));
        return *this;
    }


    // TODO: Insert with padding. Will require quite some work to work with non-copy constructible objects,
    //  e.g. Vec<std::unique_ptr<std::string>
    //    Vec<T>& insert(long index, T value, std::optional<T> pad_value = std::nullopt) {
    //        auto signed_index = sign_index(index);
    //
    //        if (signed_index == size()) {
    //            return append(std::move(value));
    //        } else if (signed_index > size()) {
    //            if (pad_value) {
    //                auto extension = Vec<T>::repeated(signed_index - size(), *pad_value);
    //                extension.append(std::move(value));
    //                m_vector.insert(m_vector.end()
    //                                , std::make_move_iterator(extension.begin())
    //                                , std::make_move_iterator(extension.end()));
    //
    //                return *this;
    //            } else {
    //                throw std::out_of_range("insert: index out of range");
    //            }
    //        }
    //
    //        m_vector.insert(m_vector.begin() + static_cast<long>(signed_index), std::move(value));
    //        return *this;
    //    }

    template<typename E = T, std::enable_if_t<std::is_copy_constructible_v<E>, int>  = 0>
    Vec<T>& extend(const Vec<T>& other) {
        m_vector.insert(m_vector.end(), other.m_vector.begin(), other.m_vector.end());
        return *this;
    }


    Vec<T>& extend(Vec<T>&& other) {
        m_vector.insert(m_vector.end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        return *this;
    }


    Vec<T>& remove(const T& value) {
        if (auto it = std::find(m_vector.begin(), m_vector.end(), value); it != m_vector.end()) {
            m_vector.erase(it);
        }
        return *this;
    }


    Vec<T>& remove(std::function<bool(const T&)> pred) {
        if (auto it = std::find_if(m_vector.begin(), m_vector.end(), pred); it != m_vector.end()) {
            m_vector.erase(it);
        }
        return *this;
    }


    void clear() {
        m_vector.clear();
    }


    std::optional<T> pop_value(const T& value) {
        if (auto it = std::find(m_vector.begin(), m_vector.end(), value); it != m_vector.end()) {
            return pop_internal(it);
        }
        return std::nullopt;
    }


    std::optional<T> pop_value(std::function<bool(const T&)> pred) {
        if (auto it = std::find_if(m_vector.begin(), m_vector.end(), pred); it != m_vector.end()) {
            return pop_internal(it);
        }
        return std::nullopt;
    }


    template<typename SizeType, typename = std::enable_if_t<std::is_integral_v<SizeType> > >
    std::optional<T> pop_index(SizeType index) {
        long iterator_index;
        if constexpr (std::is_signed_v<SizeType>) {
            iterator_index = static_cast<long>(sign_index(index));
        } else {
            iterator_index = static_cast<long>(index);
        }

        if (static_cast<std::size_t>(iterator_index) >= size()) {
            return std::nullopt;
        }

        return pop_internal(m_vector.begin() + iterator_index);
    }


    template<typename SizeType, typename = std::enable_if_t<std::is_integral_v<SizeType> > >
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


    template<typename SizeType, typename = std::enable_if_t<std::is_integral_v<SizeType> > >
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


    /**
     * Resize and append copies of the provided element
     */
    template<typename E = T, typename = std::enable_if_t<std::is_copy_constructible_v<E> > >
    Vec<T>& resize_append(std::size_t new_size, const T& append_value) {
        if (auto diff = size_diff(new_size); diff < 0) {
            shrink_internal(new_size);
        } else {
            m_vector.resize(new_size, append_value);
        }

        return *this;
    }


    /**
     * Resize and append default-constructed elements (useful for seeded elements where identical copies aren't desired)
     */
    template<typename E = T, typename = std::enable_if_t<std::is_default_constructible_v<E> > >
    Vec<T>& resize_default(std::size_t new_size) {
        if (auto diff = size_diff(new_size); diff < 0) {
            shrink_internal(new_size);
        } else {
            for (std::size_t i = 0; i < static_cast<std::size_t>(diff); ++i) {
                m_vector.emplace_back();
            }
        }
        return *this;
    }


    /**
     * Resize and append copies of the last element
     */
    template<typename E = T, typename = std::enable_if_t<std::is_copy_constructible_v<E> > >
    Vec<T>& resize_extend(std::size_t new_size) {
        if (m_vector.empty()) {
            throw std::invalid_argument("cannot resize_extend empty vector");
        }

        if (auto diff = size_diff(new_size); diff < 0) {
            shrink_internal(new_size);
        } else {
            m_vector.resize(new_size, m_vector.back());
        }

        return *this;
    }


    /**
    * Resize and append copies of existing elements in the vector (consecutive folded)
    */
    template<typename E = T, typename = std::enable_if_t<std::is_copy_constructible_v<E> > >
    Vec<T>& resize_fold(std::size_t new_size) {
        if (m_vector.empty()) {
            throw std::invalid_argument("cannot resize_fold empty vector");
        }

        if (auto diff = size_diff(new_size); diff < 0) {
            shrink_internal(new_size);
        } else {
            std::size_t original_size = m_vector.size();
            for (std::size_t i = 0; i < static_cast<std::size_t>(diff); ++i) {
                m_vector.push_back(m_vector.at(i % original_size));
            }
        }

        return *this;
    }



    // =========================== FUNCTIONAL ==========================


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


    Vec<T>& apply(std::function<T(const T&, const T&)> f, const T& value, const Vec<bool>& binary_mask) {
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


    // TODO: Not tested
    //    Vec& rotate(long amount) {
    //        if (amount >= 0) {
    //            auto rotation_position = amount % static_cast<long>(m_vector.size());
    //            std::rotate(m_vector.begin(), m_vector.begin() + rotation_position, m_vector.end());
    //        } else {
    //            auto rotation_position = -amount % static_cast<long>(m_vector.size());
    //            std::rotate(m_vector.begin(), m_vector.begin() + rotation_position, m_vector.end());
    //        }
    //        return *this;
    //    }


    Vec<T>& shift(long amount) {
        if (amount == 0) {
            return *this;
        }

        if (static_cast<std::size_t>(std::abs(amount)) >= m_vector.size()) {
            m_vector = std::vector<T>(m_vector.size(), static_cast<T>(0));
            return *this;
        }

        std::vector<T> target(m_vector.size(), static_cast<T>(0));
        if (amount > 0) {
            auto target_offset = static_cast<std::size_t>(amount);
            for (std::size_t i = 0; i < target.size() - target_offset; ++i) {
                target.at(i + target_offset) = m_vector.at(i);
            }
        } else {
            auto source_offset = static_cast<std::size_t>(std::abs(amount));
            for (std::size_t j = 0; j < target.size() - source_offset; ++j) {
                target.at(j) = m_vector.at(j + source_offset);
            }
        }

        m_vector = target;

        return *this;
    }


    T foldl(std::function<T(T, T)> f, const T& initial) const {
        T value = initial;
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            value = f(value, m_vector[i]);
        }
        return value;
    }


    decltype(auto) count(std::function<bool(T)> f) const {
        return std::count_if(m_vector.begin(), m_vector.end(), f);
    }


    template<typename E = T, typename = std::enable_if_t<!std::is_same_v<E, bool>> >
    bool all(std::function<bool(T)> f) const {
        return std::all_of(m_vector.begin(), m_vector.end(), f);
    }


    template<typename E = T, typename = std::enable_if_t<std::is_same_v<E, bool> > >
    bool all() const {
        return std::all_of(m_vector.begin(), m_vector.end(), [](const T& element) { return element; });
    }


    template<typename E = T, typename = std::enable_if_t<!std::is_same_v<E, bool>> >
    bool any(std::function<bool(T)> f) const {
        return std::any_of(m_vector.begin(), m_vector.end(), f);
    }


    template<typename E = T, typename = std::enable_if_t<std::is_same_v<E, bool> > >
    bool any() const {
        return std::any_of(m_vector.begin(), m_vector.end(), [](const T& element) { return element; });
    }


    /**
     * Removes all elements for which `f` returns false from the original Vec and returns them as a separate vector
     */
    Vec<T> filter_drain(std::function<bool(const T&)> f) {
        auto drain_iterator = std::stable_partition(m_vector.begin(), m_vector.end(), f);

        std::vector<T> drained;

        for (auto it = drain_iterator; it != m_vector.end(); ++it) {
            drained.push_back(std::move(*it));
        }

        m_vector.erase(drain_iterator, m_vector.end());
        return Vec<T>(std::move(drained));
    }


    // =========================== MISC ==========================


    template<typename E = T, typename = std::enable_if_t<utils::is_printable_v<E> > >
    void print() const {
        std::cout << "[";
        for (const T& element: m_vector) {
            std::cout << element << ", ";
        }
        std::cout << "]" << std::endl;
    }


    void print(std::function<std::string(T)> f) const {
        std::cout << "[";
        for (const T& element: m_vector) {
            std::cout << f(element) << ", ";
        }
        std::cout << "]" << std::endl;
    }


    std::size_t size() const {
        return m_vector.size();
    }


    bool contains(const T& value) const {
        return std::find(m_vector.begin(), m_vector.end(), value) != m_vector.end();
    }


    bool contains_all(const Vec<T>& values) const {
        return std::all_of(values.begin(), values.end(), [this](const T& value) {
            return contains(value);
        });
    }


    bool contains_any(const Vec<T>& values) const {
        return std::any_of(values.begin(), values.end(), [this](const T& value) {
            return contains(value);
        });
    }


    bool contains(std::function<bool(const T&)> f) const {
        for (const T& element: m_vector) {
            if (f(element)) {
                return true;
            }
        }
        return false;
    }


    std::optional<std::size_t> index(const T& value) const {
        if (auto it = std::find(m_vector.begin(), m_vector.end(), value); it != m_vector.end()) {
            return {std::distance(m_vector.begin(), it)};
        } else {
            return std::nullopt;
        }
    }


    std::optional<std::size_t> index(std::function<bool(const T&)> f) const {
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            if (f(m_vector[i])) {
                return i;
            }
        }
        return std::nullopt;
    }

    Vec<std::size_t> argwhere(std::function<bool(const T&)> f) const {
        Vec<std::size_t> indices;
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            if (f(m_vector[i])) {
                indices.append(i);
            }
        }
        return indices;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_floating_point_v<E> > >
    bool approx_equals(const Vec<T>& other, double epsilon = 1e-6) const {
        if (other.size() != m_vector.size()) {
            return false;
        }

        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            if (std::fabs(other[i] - m_vector[i]) > epsilon) {
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


    std::optional<T> first() const {
        if (m_vector.empty()) {
            return std::nullopt;
        }

        return std::make_optional(m_vector.at(0));
    }


    template<typename U = T>
    U first_or(const U& fallback) const {
        if (m_vector.empty())
            return fallback;
        return static_cast<U>(m_vector.at(0));
    }


    // ======================== ARITHMETIC OPERATIONS ========================


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& add(Args... args) {
        apply_base(std::plus<T>(), args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& subtract(Args... args) {
        apply_base(std::minus<T>(), args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& multiply(Args... args) {
        apply_base(std::multiplies<T>(), args...);
        return *this;
    }


    template<typename... Args, typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& divide(Args... args) {
        apply_base(std::divides<T>(), args...);
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& pow(const T& exponent) {
        apply_base([](const T& a, const T& b) { return std::pow(a, b); }, exponent);
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_floating_point_v<E> > >
    Vec<T>& normalize_max() {
        if (auto max_value = max(); max_value != static_cast<T>(0.0)) {
            auto scale_factor = 1 / max_value;

            for (auto& e: m_vector) {
                e *= scale_factor;
            }
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_floating_point_v<E> > >
    Vec<T>& normalize_l1() {
        auto sum = static_cast<T>(0.0);
        for (const auto& e: m_vector) {
            sum += std::abs(e);
        }

        if (sum != static_cast<T>(0.0)) {
            auto scale_factor = static_cast<T>(1.0) / sum;

            for (auto& e: m_vector) {
                e *= scale_factor;
            }
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_floating_point_v<E> > >
    Vec<T>& normalize_l2() {
        auto sum_of_squares = static_cast<T>(0.0);
        for (const auto& e: m_vector) {
            sum_of_squares += e * e;
        }

        if (sum_of_squares != static_cast<T>(0.0)) {
            auto scale_factor = std::sqrt(static_cast<T>(1.0) / sum_of_squares);

            for (auto& e: m_vector) {
                e *= scale_factor;
            }
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& min_of(const T& value) {
        for (auto& e: m_vector) {
            e = std::min(e, value);
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& max_of(const T& value) {
        for (auto& e: m_vector) {
            e = std::max(e, value);
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& clip(std::optional<T> low_thresh, std::optional<T> high_thresh) {
        if (low_thresh) {
            max_of(*low_thresh);
        }
        if (high_thresh) {
            min_of(*high_thresh);
        }
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& clip(T low_thresh, T high_thresh) {
        return clip(std::optional<T>(low_thresh), std::optional<T>(high_thresh));
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& clip_remove(std::optional<T> low_thresh, std::optional<T> high_thresh) {
        filter_drain([low_thresh, high_thresh](const T& e) {
            return (!low_thresh || e >= *low_thresh) && (!high_thresh || e <= *high_thresh);
        });
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& sort(bool ascending = true) {
        if (ascending)
            std::sort(m_vector.begin(), m_vector.end());
        else
            std::sort(m_vector.begin(), m_vector.end(), std::greater<>());

        return *this;
    }


    /**
     * @note Assumes that `indices` does not contain any gaps nor invalid indices
     */
    Vec<T>& reorder(const Vec<std::size_t>& indices) {
        if (indices.size() != m_vector.size()) {
            throw std::out_of_range("indices.size() != m_vector.size()");
        }

        std::vector<T> reordered(m_vector.size());
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            reordered[i] = m_vector[indices[i]];
        }

        m_vector = std::move(reordered);
        return *this;
    }


    Vec<std::size_t> argsort(bool ascending = true, bool apply_sort = false) {
        std::vector<std::size_t> indices(m_vector.size());
        std::iota(indices.begin(), indices.end(), 0);

        if (ascending)
            std::sort(indices.begin(), indices.end()
                      , [this](std::size_t i, std::size_t j) { return m_vector[i] < m_vector[j]; });
        else
            std::sort(indices.begin(), indices.end()
                      , [this](std::size_t i, std::size_t j) { return m_vector[i] > m_vector[j]; });

        auto output = Vec<std::size_t>(std::move(indices));

        if (apply_sort) {
            reorder(output);
        }

        return output;
    }


    Vec<T>& reverse() {
        throw std::runtime_error("Not implemented: reverse()");
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    T sum() const {
        return std::accumulate(m_vector.begin(), m_vector.end(), T(0));
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    Vec<T>& cumsum() {
        std::partial_sum(m_vector.begin(), m_vector.end(), m_vector.begin());
        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    T mean() const {
        if (m_vector.empty()) {
            throw std::logic_error("Cannot compute mean of an empty vector");
        }
        return sum() / static_cast<T>(m_vector.size());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    T max() const {
        return *std::max_element(m_vector.begin(), m_vector.end());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    T min() const {
        return *std::min_element(m_vector.begin(), m_vector.end());
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E> > >
    T peak() const {
        std::vector<T> abs_values;
        abs_values.reserve(m_vector.size());

        std::transform(m_vector.begin(), m_vector.end(), std::back_inserter(abs_values), [](const auto& e) {
            return std::abs(e);
        });

        return *std::max_element(abs_values.begin(), abs_values.end());
    }


    // ======================== BOOLEAN MASK OPERATORS =============================

    template<typename E = T, typename = std::enable_if_t<std::is_same_v<E, bool> > >
    Vec<T>& logical_not() {
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector.at(i) = !m_vector.at(i);
        }

        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_same_v<E, bool> > >
    Vec<T>& logical_and(const Vec<T>& other) {
        if (other.size() != m_vector.size()) {
            throw std::out_of_range("vectors must have the same size for element-wise operation");
        }

        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector.at(i) = m_vector.at(i) && other.m_vector.at(i);
        }

        return *this;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_same_v<E, bool> > >
    Vec<T>& logical_or(const Vec<T>& other) {
        if (other.size() != m_vector.size()) {
            throw std::out_of_range("vectors must have the same size for element-wise operation");
        }

        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            m_vector.at(i) = m_vector.at(i) || other.m_vector.at(i);
        }

        return *this;
    }

private:
    std::size_t sign_index(long index) const {
        // Note that if `std::abs(index) > m_vector.size()`,
        //  this will return std::numeric_limits<std::size_t>::max() - index`
        //  (which technically is unproblematic, since the value regardless will throw std::out_of_range)
        return static_cast<std::size_t>(utils::sign_index(index, m_vector.size()));
    }


    template<typename SizeType, typename = std::enable_if_t<
                 std::is_integral_v<SizeType> && std::is_signed_v<SizeType>> >
    Vec<std::size_t> sign_indices(const Vec<SizeType>& indices) const {
        std::vector<std::size_t> result;
        result.reserve(indices.size());
        for (auto index: indices.vector()) {
            result.push_back(sign_index(index));
        }
        return Vec<std::size_t>(std::move(result));
    }


    Vec<T> elementwise_operation(const T& operand2, std::function<T(T, T)> op) const {
        std::vector<T> output;
        output.reserve(m_vector.size());
        for (std::size_t i = 0; i < m_vector.size(); ++i) {
            output.push_back(op(m_vector.at(i), operand2));
        }
        return Vec<T>(std::move(output));
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


    enum class ResizeType {
        append, extend, fold, default_ctor
    };


    long size_diff(std::size_t new_size) const {
        return static_cast<long>(new_size - m_vector.size());
    }


    void shrink_internal(std::size_t new_size) {
        assert(new_size <= m_vector.size());

        if (new_size == 0) {
            m_vector.clear();
            return;
        }

        auto diff = size_diff(new_size);
        m_vector.erase(m_vector.end() + diff, m_vector.end());
    }


    std::optional<T> pop_internal(typename std::vector<T>::iterator it) {
        auto output = std::make_optional<T>(std::move(*it));
        m_vector.erase(it);
        return std::move(output);
    }


    std::vector<T> m_vector;
};


} // namespace serialist

#endif //SERIALISTLOOPER_VEC_H
