
#ifndef SERIALISTLOOPER_VOICE_H
#define SERIALISTLOOPER_VOICE_H

#include <vector>
#include <optional>
#include <iostream>
#include "utils.h"
#include "core/algo/vec.h"


class VoiceUtils {
public:



    template<typename T>
    static std::vector<std::vector<T>> transpose(const std::vector<T>& v) {
        std::vector<std::vector<T>> output;
        output.reserve(v.size());

        for (const T& t: v) {
            output.push_back({t});
        }
        return output;
    }
};




// ==============================================================================================

template<typename T>
using Voice = Vec<T>;


// ==============================================================================================

template<typename T>
class Voices {
public:
    const static inline std::size_t AUTO_VOICES = 0;


    explicit Voices(Vec<Voice<T>> v) : m_voices(std::move(v)) {
        // A `Voice` may be empty but a collection of `Voice`s should have at least one entry,
        //   even if the entry doesn't contain any data
        assert(!m_voices.empty());
    }


    explicit Voices(std::size_t num_voices) : Voices(empty_like(num_voices)) {}


    static Voices<T> create_empty_like() { return Voices<T>(empty_like(1)); }


    static Voices<T> new_transposed(const Voice<T>& voice) {
        return Voices{voice.vector()};
    }


    bool operator==(const Voices& other) const {
        return m_voices == other.m_voices;
    }


    void clear(std::size_t num_voices = 1) { m_voices = empty_like(num_voices); }


    std::size_t size() const { return m_voices.size(); }


    void merge(const Voices<T>& other) {
        if (size() != other.size()) {
            throw std::runtime_error("Voices size mismatch");
        }

        for (std::size_t i = 0; i < m_voices.size(); ++i) {
            m_voices[i].concatenate(other.vec());
        }
    }

    Voices<T> merge_uneven(const Voices<T>& other, bool overwrite_dimensions) const {

        if (overwrite_dimensions && other.size() > m_voices.size()) {
            m_voices.resize(other.size());
        }
        auto size = m_voices.size();
    }

    /**
    * @return true if every Voice is empty
    */
    bool is_empty_like() const {
        for (auto& voice: m_voices) {
            if (!voice.empty()) {
                return false;
            }
        }
        return true;
    }


    /**
    * @return The first value in the the first Voice or std::nulllopt if the first voice is empty
    */
    std::optional<T> front() const {
        if (m_voices.empty())
            return std::nullopt;

        return m_voices.front();
    }


    /**
    * @return The first value in the first Voice or `fallback_value` if the first Voice is empty
    */
    template<typename U = T>
    U front_or(const U& fallback) const {
        if (m_voices.empty())
            return fallback;

        return m_voices.front_or();
    }


    /**
    * @return The first value in the each Voice or std::nulllopt if voice is empty
    */
    template<typename U = T>
    Vec<std::optional<U>> fronts() const {
        Vec<std::optional<U>> output;
        output.reserve(m_voices.size());

        std::transform(m_voices.begin(), m_voices.end(), std::back_inserter(output)
                       , [](const Voice<T>& voice) { return voice.front(); });

        return output;
    }


    template<typename U = T>
    Vec<U> fronts_or(const U& fallback) const {
        Vec<U> output;
        output.reserve(m_voices.size());

        std::transform(m_voices.begin(), m_voices.end(), std::back_inserter(output)
                       , [&fallback](const Voice<T>& voice) { return voice.front_or(fallback); });

        return output;
    }



    Voices<T> adapted_to(std::size_t target_num_voices) const {
        if (m_voices.size() == target_num_voices || target_num_voices == AUTO_VOICES) {
            return *this;
        } else {
            Voices<T>(m_voices.cloned().resize_fold(target_num_voices));
        }
    }

    void resize(std::size_t new_size) {
        assert(new_size > 0);
        m_voices.resize_fold(new_size);
    }

    template<typename U = T>
    Voices<U> as_type() const {
        Vec<Voice<U>> output;
        output.reserve(m_voices.size());
        for (auto& voice: m_voices) {
            output.push_back(voice.as_type());
        }
        return output;
    }


    template<typename E = T, typename = std::enable_if_t<utils::is_printable_v<E>>>
    void print() const {
        std::cout << "{ ";
        for (auto& voice: m_voices)
            voice.print();

        std::cout << " }" << std::endl;
    }



    const Vec<Voice<T>>& vec() const { return m_voices; }

    Vec<Voice<T>>& vec_mut() { return m_voices; }




private:
    static Vec<Voice<T>> empty_like(std::size_t num_voices) {
        assert(num_voices > 0);
        return Vec<Voice<T>>::repeated(Voice<T>({}), num_voices);
    }


    Vec<Voice<T>> m_voices;


};

#endif //SERIALISTLOOPER_VOICE_H
