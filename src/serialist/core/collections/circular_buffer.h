
#ifndef SERIALISTLOOPER_CIRCULAR_BUFFER_H
#define SERIALISTLOOPER_CIRCULAR_BUFFER_H

#include <array>

template<typename T, int N>
    class Buffer {
    public:
        Buffer() = default;

        // TODO: This implementation doesn't seem solid at all: why use buffer.size() instead of size?
        //  Not safe and not fixed. Better separate FixedBuffer (std::array) and ResizeableBuffer (std::vector)
        void push(T x) {
            buffer[write_pos++] = x;
            if (write_pos == buffer.size()) {
                write_pos = 0;
            }
            if (size == buffer.size()) {
                read_pos = (read_pos + 1) % buffer.size();
            } else {
                size++;
            }
        }


        const std::array<T, N>& get_buffer() const { return buffer; }


        size_t get_size() const { return size; }


    private:
        std::array<T, N> buffer;
        std::size_t read_pos = 0;
        std::size_t write_pos = 0;
        std::size_t size = 0;
    };

#endif //SERIALISTLOOPER_CIRCULAR_BUFFER_H
