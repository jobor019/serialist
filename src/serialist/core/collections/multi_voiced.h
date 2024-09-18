
#ifndef SERIALISTLOOPER_MULTI_VOICED_H
#define SERIALISTLOOPER_MULTI_VOICED_H

#include "core/collections/voices.h"
#include "core/collections/vec.h"

namespace serialist {

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


    virtual Voice<T> flush(std::function<bool(const T&)> f) {
        (void) f;
        return flush();
    }
};


// ==============================================================================================

template<typename ObjectType, typename DataType>
class MultiVoiced {
public:

    explicit MultiVoiced(std::size_t num_voices = 1)
            : m_objects(Vec<ObjectType>::repeated(num_voices, ObjectType())) {
        static_assert(std::is_default_constructible_v<ObjectType>, "ObjectType must be default constructible");
        if (num_voices == 0) {
            throw std::invalid_argument("num_voices cannot be 0");
        }
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
    Voices<DataType> flush(std::function<bool(const DataType&)> f) {
        auto output = Voices<DataType>::zeros(m_objects.size());

        for (std::size_t i = 0; i < m_objects.size(); ++i) {
            auto e = m_objects[i].flush(f);
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
        if (new_size == 0) {
            throw std::invalid_argument("num voices cannot be 0");
        }
        auto flushed = Voices<DataType>::zeros(m_objects.size()); // old object size

        if (new_size < m_objects.size()) {
            for (std::size_t i = new_size; i < m_objects.size(); ++i) {
                flushed[i] = m_objects[i].flush();
            }
        }
        m_objects.resize_default(new_size);
        return flushed;
    }


    template<typename E = DataType>
    std::enable_if_t<!std::is_base_of_v<Flushable<E>, ObjectType>, void>
    resize(std::size_t new_size) {
        if (new_size == 0) {
            throw std::invalid_argument("num voices cannot be 0");
        }
        m_objects.resize_default(new_size);
    }


    template<typename Setter, typename ArgType, typename = std::enable_if_t<std::is_member_function_pointer_v<Setter>>>
    void set(Setter func, Vec<ArgType>&& values) {
        if (values.size() != m_objects.size()) {
            throw std::invalid_argument("values.size() != m_vector.size()");
        }

        for (std::size_t i = 0; i < values.size(); ++i) {
            (m_objects[i].*func)(std::move(values[i]));
        }
    }

    template<typename Setter, typename ArgType, typename = std::enable_if_t<std::is_member_function_pointer_v<Setter>>>
    void set(Setter func, const ArgType& v) {
        for (auto& obj : m_objects) {
            (obj.*func)(v);
        }
    }


    template<typename U>
    decltype(auto) operator[](const U& index) {
        return m_objects[index];
    }


    template<typename U>
    decltype(auto) operator[](const U& index) const {
        return m_objects[index];
    }


    decltype(auto) begin() { return m_objects.begin(); }


    decltype(auto) begin() const { return m_objects.begin(); }


    decltype(auto) end() { return m_objects.end(); }


    decltype(auto) end() const { return m_objects.end(); }


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


} // namespace serialist

#endif //SERIALISTLOOPER_MULTI_VOICED_H
