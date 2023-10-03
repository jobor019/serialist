
#ifndef SERIALISTLOOPER_VOICES_H
#define SERIALISTLOOPER_VOICES_H

#include "core/voice.h"
#include "vec.h"

template<typename T>
class Flushable {
public:
    virtual ~Flushable() = default;
    Flushable(const Flushable&) = delete;
    Flushable& operator=(const Flushable&) = delete;
    Flushable(Flushable&&) noexcept = default;
    Flushable& operator=(Flushable&&) noexcept = default;

    virtual Voice<T> flush() = 0;
};


template<typename ObjectType, typename DataType>
class MultiVoiced {
public:

    template<typename E = DataType, typename = std::enable_if_t<std::is_base_of_v<Flushable<E>, ObjectType>>>
    Voices<DataType> flush() {
        Voices<DataType> output;
        for (auto& flushable: m_objects) {
            output.append(flushable.flush());
        }
        return output;

    }


    template<typename E = DataType, typename = std::enable_if_t<std::is_base_of_v<Flushable<E>, ObjectType>>>
    Voice<DataType> flush(std::size_t voice_index) {
        return m_objects.at(voice_index).flush();
    }


    template<typename E = DataType>
    std::enable_if_t<std::is_base_of_v<Flushable<E>, ObjectType>, Voices<DataType>>
    resize(std::size_t new_size) {
        if (new_size < m_objects.size()) {
            Voices<DataType> flushed(m_objects.size()); // old object size
            for (std::size_t i = new_size; i < m_objects.size(); ++i) {
                flushed.at(i) = m_objects.at(i).flush();
            }
            return flushed;
        }

        return Voices<DataType>::create_empty_like();

    }


    template<typename E = DataType>
    std::enable_if_t<!std::is_base_of_v<Flushable<E>, ObjectType>, void>
    resize(std::size_t new_size) {
        m_objects.res(new_size);
    }


private:


    Vec<ObjectType> m_objects;
};

#endif //SERIALISTLOOPER_VOICES_H
