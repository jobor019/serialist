
#ifndef SERIALISTLOOPER_VOICES_H
#define SERIALISTLOOPER_VOICES_H

#include <vector>
#include <optional>
#include <iostream>
#include "core/utility/enums.h"
#include "core/utility/traits.h"
#include "core/collections/vec.h"


template<typename T>
using Voice = Vec<T>;


// ==============================================================================================

template<typename T>
class Voices {
public:
    const static inline std::size_t AUTO_VOICES = 0;


    // =========================== CONSTRUCTORS ==========================

    Voices(std::initializer_list<Voice<T>> v) : Voices(Vec<Voice<T>>(std::move(v))) {}


    explicit Voices(Vec<Voice<T>> v) : m_voices(std::move(v)) {
        // A `Voice` may be empty but a collection of `Voice`s should have at least one entry,
        //   even if the entry doesn't contain any data
        assert(!m_voices.empty());
    }


    static Voices<T> zeros(std::size_t num_voices) {
        return Voices<T>(Voices<T>::create_empty_like(num_voices));
    }


    static Voices<T> empty_like() {
        return Voices<T>(Voices<T>::create_empty_like(1));
    }


    static Voices<T> singular(const T& value, std::size_t num_voices = 1) {
        return one_hot(value, 0, num_voices);
    }


    static Voices<T> one_hot(const T& value, std::size_t index, std::size_t num_voices) {
        auto voices = Voices<T>::zeros(num_voices);
        voices[index].append(value);
        return std::move(voices);
    }


    static Voices<T> one_hot(const Vec<T>& values, std::size_t index, std::size_t num_voices) {
        auto voices = Voices<T>::zeros(num_voices);
        voices[index] = values;
        return voices;
    }


    static Voices<T> repeated(const T& value, std::size_t num_voices) {
        auto v = Vec<Voice<T>>::repeated(num_voices, {value});
        return Voices<T>(std::move(v));
    }


    static Voices<T> transposed(const Voice<T>& voice) {
        auto output = Vec<Voice<T>>::allocated(voice.size());
        for (auto& v: voice.vector()) {
            output.append(Voice<T>::singular(v));
        }
        return Voices<T>(std::move(output));
    }


    // =========================== OPERATORS ==========================

    bool operator==(const Voices& other) const {
        return m_voices == other.m_voices;
    }


    bool operator!=(const Voices& other) const {
        return m_voices != other.m_voices;
    }


    template<typename E = T, typename = std::enable_if_t<std::is_floating_point_v<E>>>
    bool approx_equals(const Voices<T>& other, double epsilon = 1e-6) const {
        if (other.size() != m_voices.size()) {
            return false;
        }

        for (std::size_t i = 0; i < m_voices.size(); ++i) {
            if (!m_voices[i].approx_equals(other.vec()[i], epsilon)) {
                return false;
            }
        }
        return true;
    }


    decltype(auto) operator[](std::size_t index) {
        return m_voices[index];
    }


    decltype(auto) operator[](std::size_t index) const {
        return m_voices[index];
    }


    decltype(auto) begin() { return m_voices.begin(); }


    decltype(auto) end() { return m_voices.end(); }


    decltype(auto) begin() const { return m_voices.begin(); }


    decltype(auto) end() const { return m_voices.end(); }


    // =========================== CLONING / COPYING ==========================

    Voices<T> cloned() const {
        return Voices<T>(m_voices);
    }


    template<typename U = T>
    Voices<U> as_type() const {
        std::vector<Voice<U>> output;
        output.reserve(m_voices.size());

        for (const auto& voice: m_voices.vector()) {
            output.push_back(voice.template as_type<U>());
        }

        auto vec = Vec<Voice<U>>(std::move(output));
        return Voices<U>(std::move(vec));
    }


    template<typename U = T>
    Voices<U> as_type(std::function<U(const T&)> f) const {
        std::vector<Voice<U>> output;
        output.reserve(m_voices.size());

        for (const auto& voice: m_voices.vector()) {
            output.push_back(voice.template as_type<U>(f));
        }

        auto vec = Vec<Voice<U>>(std::move(output));
        return Voices<U>(std::move(vec));
    }


    // =========================== MUTATORS ==========================

    Voices<T>& clear(std::size_t num_voices = 1) {
        m_voices = create_empty_like(num_voices);
        return *this;
    }


    Voices<T>& merge(const Voices<T>& other) {
        if (size() != other.size()) {
            throw std::runtime_error("Voices size mismatch");
        }

        for (std::size_t i = 0; i < m_voices.size(); ++i) {
            m_voices[i].concatenate(other.vec()[i]);
        }

        return *this;

    }


    /**
     * merges two `Voices<T>` of different sizes.
     */
    Voices<T>& merge_uneven(const Voices<T>& other, bool allow_expand) {
        if (allow_expand && other.size() > m_voices.size()) {
            m_voices.resize_append(other.size(), Voice<T>());
        }

        std::size_t num_voices = std::min(m_voices.size(), other.size());
        for (std::size_t i = 0; i < num_voices; ++i) {
            m_voices[i].concatenate(other.vec()[i]);
        }

        return *this;
    }


    Voices<T>& adapted_to(std::size_t target_num_voices) {
        if (m_voices.size() == target_num_voices || target_num_voices == AUTO_VOICES)
            return *this;

        m_voices.resize_fold(target_num_voices);
        return *this;
    }


    // =========================== ACCESSORS ==========================

    /**
    * @return The first value in the the first Voice or std::nulllopt if the first voice is empty
    */
    std::optional<T> first() const {
        if (m_voices.empty())
            return std::nullopt;

        if (auto voice = m_voices.first(); voice.has_value()) {
            return voice.value().first();
        } else {
            return std::nullopt;
        }
    }


    /**
    * @return The first value in the first Voice or `fallback_value` if the first Voice is empty
    */
    template<typename U = T>
    U first_or(const U& fallback) const {
        if (m_voices.empty())
            return fallback;

        if (auto voice = m_voices.first(); voice.has_value()) {
            return (*voice).first_or(fallback);
        } else {
            return fallback;
        }
    }


    /**
    * @return The first value in the each Voice or std::nulllopt if voice is empty
    */
    template<typename U = T>
    Vec<std::optional<U>> firsts() const {
        std::vector<std::optional<U>> output;
        output.reserve(m_voices.size());

        auto& v = m_voices.vector();
        std::transform(v.begin(), v.end(), std::back_inserter(output)
                       , [](const Voice<T>& voice) { return voice.first(); });

        return Vec<std::optional<U>>(std::move(output));
    }


    template<typename U = T>
    Vec<U> firsts_or(const U& fallback) const {
        std::vector<U> output;
        output.reserve(m_voices.size());

        auto v = m_voices.vector();
        std::transform(v.begin(), v.end(), std::back_inserter(output)
                       , [&fallback](const Voice<T>& voice) { return voice.first_or(fallback); });

        return Vec<U>(std::move(output));
    }


    template<typename U = T>
    std::optional<Vec<U>> first_vec() const {
        if (m_voices.empty())
            return std::nullopt;

        auto first = m_voices.first();

        if constexpr (std::is_same_v<U, T>) {
            return first;
        }

        if (!first.has_value()) {
            return std::nullopt;
        }

        return {(*first).template as_type<U>()};
    }


    template<typename U = T>
    Vec<U> first_vec_or(const Vec<U>& fallback) const {
        if (auto v = first_vec<U>(); v.has_value() && !(*v).empty()) {
            return *v;
        }
        return fallback;
    }

    // =========================== MISC ==========================

    Vec<T> flattened() const {
        Vec<T> output;
        for (const auto& voice: m_voices.vector()) {
            output.extend(voice);
        }
        return std::move(output);
    }

    std::pair<Voices<T>, Voices<T>> partition(std::function<bool(const T&)> f) const {
        Voices<T> matching = Voices<T>::zeros(m_voices.size());
        Voices<T> non_matching = Voices<T>::zeros(m_voices.size());

        for (std::size_t i = 0; i < m_voices.size(); ++i) {
            for (const auto& elem : m_voices[i]) {
                if (f(elem)) {
                    matching[i].append(elem);
                } else {
                    non_matching[i].append(elem);
                }
            }
        }

        return {matching, non_matching};
    }


    template<typename E = T, typename = std::enable_if_t<utils::is_printable_v<E>>>
    void print() const {
        std::cout << "{ ";
        for (const auto& voice: m_voices.vector())
            voice.print();

        std::cout << " }" << std::endl;
    }


    std::size_t size() const {
        return m_voices.size();
    }


    /**
    * @return true if every Voice is empty
    */
    bool is_empty_like() const {
        auto& v = m_voices.vector();
        return !std::any_of(v.begin(), v.end(), [](const Voice<T>& voice) { return !voice.empty(); });
    }


    const Vec<Voice<T>>& vec() const { return m_voices; }


    Vec<Voice<T>>& vec_mut() { return m_voices; }


private:
    static Vec<Voice<T>> create_empty_like(std::size_t num_voices) {
        if (num_voices == 0) {
            throw std::invalid_argument("num_voices must be greater than 0");
        }

        return Vec<Voice<T>>::repeated(num_voices, Voice<T>());
    }


    Vec<Voice<T>> m_voices;


};

#endif //SERIALISTLOOPER_VOICES_H
