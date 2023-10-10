
#ifndef SERIALISTLOOPER_MULTI_VOICED_H
#define SERIALISTLOOPER_MULTI_VOICED_H

#include "core/algo/collections/voices.h"
#include "core/algo/collections/vec.h"

template<typename T>
class Flushable {
public:
    Flushable() = default;
    virtual ~Flushable() = default;
    Flushable(const Flushable&) = default;
    Flushable& operator=(const Flushable&) = default;
    Flushable(Flushable&&) noexcept = default;
    Flushable& operator=(Flushable&&) noexcept = default;

    virtual Voice<T> flush() = 0;
};


// ==============================================================================================

template<typename ObjectType, typename DataType>
class MultiVoiced {
public:

    explicit MultiVoiced(std::size_t num_voices) : m_objects(Vec<ObjectType>::repeated(num_voices, ObjectType())) {
        static_assert(std::is_default_constructible_v<ObjectType>, "ObjectType must be default constructible");
        assert(num_voices > 0);
    }


    template<typename E = DataType, typename = std::enable_if_t<std::is_base_of_v<Flushable<E>, ObjectType>>>
    Voices<DataType> flush() {
        auto output = Voices<DataType>::zeros(m_objects.size());

        for (std::size_t i = 0; i < m_objects.size(); ++i) {
            auto e = m_objects[i].flush();
            output[i].extend(e);
        }

        return output;
    }


    template<typename E = DataType, typename = std::enable_if_t<std::is_base_of_v<Flushable<E>, ObjectType>>>
    Voice<DataType> flush(std::size_t voice_index) {
        return m_objects[voice_index].flush();
    }


    template<typename E = DataType>
    std::enable_if_t<std::is_base_of_v<Flushable<E>, ObjectType>, Voices<DataType>>
    resize(std::size_t new_size) {
        assert(new_size > 0);
        auto flushed = Voices<DataType>::zeros(m_objects.size()); // old object size

        if (new_size < m_objects.size()) {
            for (std::size_t i = new_size; i < m_objects.size(); ++i) {
                flushed[i] = m_objects[i].flush();
            }
        }
        m_objects.resize_append(new_size, ObjectType());
        return flushed;
    }


    template<typename E = DataType>
    std::enable_if_t<!std::is_base_of_v<Flushable<E>, ObjectType>, void>
    resize(std::size_t new_size) {
        m_objects.resize_append(new_size, ObjectType());
    }


    const Vec<ObjectType>& get_objects() const {
        return m_objects;
    }


    Vec<ObjectType>& get_objects() {
        return m_objects;
    }


    std::size_t size() const {
        return m_objects.size();
    }


private:
    Vec<ObjectType> m_objects;
};


#endif //SERIALISTLOOPER_MULTI_VOICED_H
