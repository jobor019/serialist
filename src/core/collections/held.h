
#ifndef SERIALISTLOOPER_HELD_H
#define SERIALISTLOOPER_HELD_H

#include <type_traits>
#include "core/collections/vec.h"
#include "core/algo/voice/multi_voiced.h"

template<typename T, bool AllowDuplicates = false>
class Held : public Flushable<T> {
public:
    explicit Held()  {
        static_assert(std::is_same<decltype(std::declval<T>() == std::declval<T>()), bool>::value
                      , "T must implement the == operator");
    }


    bool bind(const T& v) {
        if constexpr (AllowDuplicates) {
            m_held.append(v);
            return true;
        }

        if (!m_held.contains(v)) {
            m_held.append(v);
            return true;
        }
        return false;
    }


    void release(const T& v) {
        m_held.remove(v);
    }


    Voice<T> flush() override {
        return m_held.drain();
    }


    Voice<T> flush(std::function<bool(const T&)> f) override {
        return m_held.filter_drain(f);
    }


    const Vec<T>& get_held() const {
        return m_held;
    }

    Vec<T>& get_held_mut() {
        return m_held;
    }


private:
    Vec<T> m_held;
};


// ==============================================================================================

template<typename T>
class MultiVoiceHeld {
public:
    explicit MultiVoiceHeld(std::size_t num_voices) : m_voiced_held(num_voices) {}


    bool bind(const T& v, std::size_t voice_index) {
        return m_voiced_held.get_objects()[voice_index].bind(v);
    }


    void release(const T& v, std::size_t voice_index) {
        m_voiced_held.get_objects()[voice_index].release(v);
    }


    Voices<T> get_held() const {
        auto held = Voices<T>::zeros(m_voiced_held.size());
        for (std::size_t i = 0; i < m_voiced_held.size(); ++i) {
            held[i] = m_voiced_held.get_objects()[i].get_held();
        }
        return held;
    }


    Voices<T> flush() {
        return m_voiced_held.flush();
    }


    Voices<T> flush(std::function<bool(const T&)> f) {
        return m_voiced_held.flush(f);
    }


    Vec<T> flush(std::size_t voice_index) {
        return m_voiced_held.flush(voice_index);
    }


    Voices<T> resize(std::size_t num_voices) {
        return m_voiced_held.resize(num_voices);
    }


    std::size_t size() const {
        return m_voiced_held.size();
    }


private:
    MultiVoiced<Held<T>, T> m_voiced_held;

};

#endif //SERIALISTLOOPER_HELD_H
