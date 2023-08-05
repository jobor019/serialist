
#ifndef SERIALISTLOOPER_VOICE_H
#define SERIALISTLOOPER_VOICE_H

#include <vector>


template<typename T>
class Voice {
public:

    explicit Voice(std::vector<T> v) : m_voice(std::move(v)) {}


    bool empty() const { return m_voice.empty(); }


    const std::vector<T>& vector() const { return m_voice; }


private:
    std::vector<T> m_voice;
};


// ==============================================================================================

template<typename T>
class Voices {
public:

    explicit Voices(std::vector<Voice<T>> v) : m_voices(std::move(v)) {
        // A `Voice` may be empty but a collection of `Voice`s should have at least one entry, even if it's empty
        assert(!v.empty());
    }


    explicit Voices(std::size_t num_voices) : Voices(std::vector<Voice<T>>(num_voices, {})) {}


    Voices<T> from_other(const Voices<T>& other, std::size_t target_num_voices) {
        return adapt_to_voice_count(other, target_num_voices);
    }


    void clear() { m_voices.clear(); }


    size_t size() const { return m_voices.size(); }


    Voices<T> values_or(const T& fallback_value, int num_voices) {
        return distribute(v_or(m_voices, fallback_value), num_voices);
    }


    const std::vector<Voice<T>>& vector() const { return m_voices; }


private:
    static Voices<T> adapt_to_voice_count(const Voices<T>& v, std::size_t target_num_voices) {
        if (v.size() == target_num_voices) {
            return v;
        } else {
            return distribute(v, target_num_voices);
        }
    }


    static Voices<T> v_or(const Voices<T>& v, const T& fallback) {
        std::vector<Voice<T>> output;
        output.reserve(v.size());

        for (auto& voice: v) {
            if (voice.empty())
                output.emplace_back(Voice<T>({fallback}));
            else {
                output.push_back(voice);
            }
        }

        return output;
    }


    static Voices<T> distribute(const Voices<T>& v, std::size_t target_num_voices) {
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
