
#ifndef SERIALISTLOOPER_HELD_H
#define SERIALISTLOOPER_HELD_H

#include <type_traits>
#include "core/collections/vec.h"
#include "multi_voiced.h"

namespace serialist {

template<typename T, bool AllowDuplicates = false, bool InsertSorted = false>
class Held : public Flushable<T> {
public:
    explicit Held()  {
        static_assert(std::is_same_v<decltype(std::declval<T>() == std::declval<T>()), bool>
                      , "T must implement the == operator");
    }


    bool bind(const T& v) {
        if constexpr (AllowDuplicates) {
            insert(v);
            return true;
        } else {
            if (!m_held.contains(v)) {
                insert(v);
                return true;
            }
            return false;
        }
    }


    void release(const T& v) {
        m_held.remove(v);
    }


    Voice<T> flush() override {
        return m_held.drain();
    }


    /** Flushed all elements for which `f` returns false */
    Voice<T> flush(std::function<bool(const T&)> f) override {
        return m_held.filter_drain(f);
    }


    std::optional<std::reference_wrapper<T>> find(const std::function<bool(const T&)>& f) {
        return m_held.find(f);
    }


    std::optional<std::reference_wrapper<const T>> find(const std::function<bool(const T&)>& f) const {
        return m_held.find(f);
    }


    const Vec<T>& get_held() const {
        return m_held;
    }

    Vec<T>& get_held_mut() {
        return m_held;
    }


private:
    void insert(const T& v) {
        if constexpr (InsertSorted) {
            m_held.insert_sorted(v);
        } else {
            m_held.append(v);
        }
    }


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


    Voices<T> flush() {
        return m_voiced_held.flush();
    }


    Voices<T> flush(std::function<bool(const T&)> f) {
        return m_voiced_held.flush(f);
    }


    Vec<T> flush(std::size_t voice_index) {
        return m_voiced_held.flush(voice_index);
    }


    Vec<T> flush(std::size_t voice_index, std::function<bool(const T&)> f) {
        return m_voiced_held.flush(voice_index, f);
    }


    Voices<T> resize(std::size_t num_voices) {
        return m_voiced_held.resize(num_voices);
    }


    std::size_t size() const {
        return m_voiced_held.size();
    }

    const Vec<T>& get_vec(std::size_t voice_index) const {
        return m_voiced_held.get_objects()[voice_index].get_held();
    }

    Vec<T>& get_vec_mut(std::size_t voice_index) {
        return m_voiced_held.get_objects()[voice_index].get_held_mut();
    }

    const MultiVoiced<Held<T>, T>& get_internal_object() const { return m_voiced_held; }
    MultiVoiced<Held<T>, T>& get_internal_object() { return m_voiced_held; }


private:
    MultiVoiced<Held<T>, T> m_voiced_held;

};

} // namespace serialist

#endif //SERIALISTLOOPER_HELD_H
