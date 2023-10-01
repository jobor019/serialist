
#ifndef SERIALISTLOOPER_HELD_NOTES_H
#define SERIALISTLOOPER_HELD_NOTES_H

#include <vector>

template<typename T>
class Held {
public:
    struct HeldObject {
        T value;
        std::size_t voice;


        bool operator==(const HeldObject& other) const {
            return value == other.value && voice == other.voice;
        }
    };


    bool bind(T value, std::size_t voice) {
        if (auto obj = HeldObject{value, voice}; std::find(m_held.begin(), m_held.end(), obj) == m_held.end()) {
            m_held.push_back(obj);
            return true;
        }
        return false;
    }


    bool release(T value, std::size_t voice) {
        if (auto it = std::find(m_held.begin(), m_held.end(), HeldObject{value, voice}); it != m_held.end()) {
            m_held.erase(it);
            return true;
        }
        return false;
    }


    std::vector<HeldObject> flush() {
        std::vector<HeldObject> v;
        std::swap(m_held, v);
        return v;
    }


    const std::vector<HeldObject>& get_held() const {
        return m_held;
    }


private:
    std::vector<HeldObject> m_held;

};

#endif //SERIALISTLOOPER_HELD_NOTES_H
