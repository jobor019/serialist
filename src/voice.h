
#ifndef SERIALISTLOOPER_VOICE_H
#define SERIALISTLOOPER_VOICE_H

#include <vector>

class VoiceUtils {
public:
    VoiceUtils() = delete;


    template<typename OutputType, typename InputType, std::enable_if_t<!std::is_same_v<InputType, OutputType>, int> = 0>
    static std::vector<OutputType> adapted_to(const std::vector<InputType>& v, std::size_t target_num_voices) {
        std::vector<OutputType> voices;
        voices.reserve(target_num_voices);

        for (std::size_t i = 0; i < target_num_voices; ++i) {
            voices.push_back(static_cast<OutputType>(v.at(i % v.size())));
        }

        return voices;
    }


    template<typename OutputType>
    static std::vector<OutputType>
    adapted_to(const std::vector<OutputType>& v, std::size_t target_num_voices) {
        if (v.size() == target_num_voices) {
            return v;
        } else {
            std::vector<OutputType> voices;
            voices.reserve(target_num_voices);

            for (std::size_t i = 0; i < target_num_voices; ++i) {
                voices.push_back(v.at(i % v.size()));
            }

            return voices;
        }
    }
};


// ==============================================================================================

template<typename T>
class Voice {
public:

    explicit Voice(std::vector<T> v) : m_voice(std::move(v)) {}


    explicit Voice(const T& v) : Voice(std::vector<T>(1, v)) {}


    static Voice create_empty() { return Voice({}); }


    std::size_t size() const { return m_voice.size(); }


    bool empty() const { return m_voice.empty(); }


    const T& at(std::size_t pos) const { return m_voice.at(pos); }


    void append(const T& v) { m_voice.push_back(v); }


    const std::vector<T>& vector() const { return m_voice; }


    template<typename U = T>
    U value_or(const U& fallback) const {
        if (m_voice.empty())
            return fallback;
        return static_cast<U>(m_voice.at(0));
    }


    template<typename U = T>
    std::vector<U> adapted_to(std::size_t target_num_voices) const {
        return VoiceUtils::adapted_to<U, T>(m_voice, target_num_voices);
    }


    template<typename U = T>
    std::optional<U> value() const {
        if (m_voice.empty())
            return std::nullopt;
        return m_voice.front();
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


    explicit Voices(std::size_t num_voices) : Voices(empty_like(num_voices)) {}


    explicit Voices(const T& v, std::size_t num_voices = 1) : Voices(std::vector<T>(num_voices, v)) {}


    explicit Voices(const std::vector<T>& voices) {
        assert(!voices.empty());
        for (auto& e: voices) {
            m_voices.emplace_back(Voice<T>(e));
        }
    }


    static Voices<T> create_empty_like() { return Voices<T>(1); }


    void append(const Voice<T>& voice) {
        m_voices.push_back(voice);
    }


    /**
     * @throws std::out_of_range if `voice_index >= size()`
     */
    void append_to(std::size_t voice_index, const T& value) {
        m_voices.at(voice_index).append(value);
    }



//    Voices<T> from_other(const Voices<T>& other, std::size_t target_num_voices) {
//        return adapt_to_voice_count(other, target_num_voices);
//    }


    void clear(std::size_t num_voices = 1) { m_voices = empty_like(num_voices); }


    std::size_t size() const { return m_voices.size(); }


    /**
     * @throw std::out_of_range if `voice_index >= size()`
     */
    const Voice<T>& at(std::size_t voice_index) const {
        return m_voices.at(voice_index);
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
    const Voice<T>& front() const { return m_voices.front(); }


    /**
    * @return The first value in the each Voice or std::nulllopt if voice is empty
    */
    template<typename U = T>
    std::vector<std::optional<U>> fronts() const {
        std::vector<std::optional<U>> output;
        output.reserve(m_voices.size());

        std::transform(m_voices.begin(), m_voices.end(), std::back_inserter(output)
                       , [](const Voice<T>& voice) { return voice.value(); });

        return output;
    }


    template<typename U = T>
    std::vector<U> fronts_or(const U& fallback) const {
        std::vector<U> output;
        output.reserve(m_voices.size());

        std::transform(m_voices.begin(), m_voices.end(), std::back_inserter(output)
                       , [&fallback](const Voice<T>& voice) { return voice.value_or(fallback); });

        return output;
    }


    /**
     * @return The first value in the first Voice or `fallback_value` if the first Voice is empty
     */
    template<typename U = T>
    U front_or(const U& fallback) const {
        return front().value_or(fallback);
    }


    /**
     * @return duplicate of values_or?
     */
//    template<typename U = T>
//    std::vector<U> fronts_or(const U& fallback) const {
//        std::vector<U> output;
//        output.reserve(m_voices.size());
//
//        std::transform(m_voices.begin(), m_voices.end(), std::back_inserter(output)
//                       , [&fallback](const Voice<T>& voice) { return voice.value_or(fallback); });
//
//        return output;
//    }


    /**
     * @return The first value in each Voice or `fallback_value` if voice is empty
     */
    template<typename U = T>
    std::vector<U> values_or(const U& fallback_value) const {
        return v_or(m_voices, fallback_value);
    }


    /**
     * @return The first value in each Voice or `fallback_value` if voice is empty
     */
    template<typename U = T, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
    std::vector<U> values_or(const U& fallback_value
                             , const std::optional<U>& low_thresh
                             , const std::optional<U>& high_thresh) const {
        auto values = v_or(m_voices, fallback_value);
        for (auto& v: values) {
            if (low_thresh)
                v = std::max(*low_thresh, v);
            if (high_thresh) {
                v = std::min(*high_thresh, v);
            }
        }
        return values;
    }


    Voices<T> adapted_to(std::size_t target_num_voices) const {
        return Voices<T>(VoiceUtils::adapted_to<Voice<T>>(m_voices, target_num_voices));
    }


    std::vector<T> flatten() const {
        std::vector<T> flattened;

        for (auto& voice: m_voices) {
            flattened.insert(flattened.end(), voice.begin(), voice.end());
        }
        return flattened;
    }


    const std::vector<Voice<T>>& vector() const { return m_voices; }


private:
    static std::vector<Voice<T>> empty_like(std::size_t num_voices) {
        assert(num_voices > 0);
        return std::vector<Voice<T>>(num_voices, Voice<T>({}));
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


    std::vector<Voice<T>> m_voices;


};

#endif //SERIALISTLOOPER_VOICE_H
