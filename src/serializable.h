

#ifndef SERIALISTLOOPER_SERIALIZABLE_H
#define SERIALISTLOOPER_SERIALIZABLE_H

#include <string>
#include <memory>




class PolymorphicSerializable {
public:
    PolymorphicSerializable() = default;
    virtual ~PolymorphicSerializable() = default;
    PolymorphicSerializable(const PolymorphicSerializable&) = default;
    PolymorphicSerializable& operator=(const PolymorphicSerializable&) = default;
    PolymorphicSerializable(PolymorphicSerializable&&)  noexcept = default;
    PolymorphicSerializable& operator=(PolymorphicSerializable&&)  noexcept = default;

    virtual std::string to_string() = 0;

    static std::unique_ptr<PolymorphicSerializable> from_string(const std::string& s);


};

#endif //SERIALISTLOOPER_SERIALIZABLE_H
