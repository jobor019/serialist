
#ifndef SERIALISTLOOPER_VOICE_H
#define SERIALISTLOOPER_VOICE_H

#include <vector>


template<typename T>
class Voice {
public:

    explicit Voice(std::vector<T> v) : m_voice(std::move(v)) {}


    explicit Voice(const T& v) : Voice(std::vector<T>(1, v)) {}


    Voice create_empty() { return Voice({}); }


    bool empty() const { return m_voice.empty(); }


    const T& at(std::size_t pos) const { return m_voice.at(pos); }


    const std::vector<T>& vector() const { return m_voice; }


    template<typename U = T>
    U value_or(const U& fallback) const {
        if (m_voice.empty()) {
            return fallback;
        } else {
            return static_cast<U>(m_voice.at(0));
        }
    }


private:
    std::vector<T> m_voice;
};


// ==============================================================================================

template<typename T>
class Voices {
public:

    explicit Voices(std::vector<Voice<T>> v) : m_voices(std::move(v)) {
        // A `Voice` may be empty but a collection of `Voice`s should have at least one entry,
        //   even if the entry doesn't contain any data
        assert(!m_voices.empty());
    }

    explicit Voices(std::size_t num_voices) : Voices(std::vector<Voice<T>>(num_voices, Voice<T>({}))) {}


    explicit Voices(const T& v, std::size_t num_voices = 1) : Voices(std::vector<T>(num_voices, v)) {}


    explicit Voices(const std::vector<T>& voices) {
        assert(!voices.empty());
        for (auto& e: voices) {
            m_voices.emplace_back(Voice<T>(e));
        }
    }





//    Voices<T> from_other(const Voices<T>& other, std::size_t target_num_voices) {
//        return adapt_to_voice_count(other, target_num_voices);
//    }


    void clear() { m_voices.clear(); }


    std::size_t size() const { return m_voices.size(); }


    const Voice<T>& front() const { return m_voices.front(); }


    template<typename U = T>
    U front_or(const U& fallback) const {
        return front().value_or(fallback);
    }


    template<typename U = T>
    std::vector<U> values_or(const U& fallback_value) const {
        return v_or(m_voices, fallback_value);
    }


    Voices<T> adapted_to(std::size_t target_num_voices) const {
        return adapt_to_voice_count(m_voices, target_num_voices);
    }


    const std::vector<Voice<T>>& vector() const { return m_voices; }


private:
    static Voices<T> adapt_to_voice_count(const std::vector<Voice<T>>& v, std::size_t target_num_voices) {
        if (v.size() == target_num_voices) {
            return Voices<T>(v);
        } else {
            return distribute(v, target_num_voices);
        }
    }


    template<typename U = T>
    static std::vector<U> v_or(const std::vector<Voice<T>>& v, const U& fallback) {
        std::vector<U> output;
        output.reserve(v.size());

        for (auto& voice: v) {
            if (voice.empty())
                output.emplace_back(fallback);
            else {
                output.push_back(static_cast<U>(voice.at(0)));
            }
        }

        return output;
    }


    static Voices<T> distribute(const std::vector<Voice<T>>& v, std::size_t target_num_voices) {
        std::vector<Voice<T>> voices;
        voices.reserve(target_num_voices);

        for (std::size_t i = 0; i < target_num_voices; ++i) {
            voices.push_back(v.at(i % v.size()));
        }

        return Voices<T>(voices);
    }


    std::vector<Voice<T>> m_voices;


};

#endif //SERIALISTLOOPER_VOICE_H
