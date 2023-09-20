
#ifndef SERIALISTLOOPER_CONNECTABLE_H
#define SERIALISTLOOPER_CONNECTABLE_H

#include "generative.h"

class Connectable {
public:
    Connectable() = default;
    virtual ~Connectable() = default;
    Connectable(const Connectable&) = delete;
    Connectable& operator=(const Connectable&) = delete;
    Connectable(Connectable&&)  noexcept = default;
    Connectable& operator=(Connectable&&)  noexcept = default;

    virtual bool is_connected() const = 0;

    virtual Generative* get_connected() const = 0;

    virtual bool is_connectable(Generative& generative) const = 0;

    virtual bool try_connect(Generative& generative) = 0;

    virtual void disconnect_if(Generative& connected_to) = 0;


//    template<typename... Args>
//    static std::vector<Connectable*> collect_connectable(Args* ... args) {
//        return utils::collect_if<Connectable>(args...);
//    }
//
//
//    template<typename... Args>
//    static std::vector<Generative*> collect_connected(Args* ... args) {
//        return utils::collect_if<Generative>(args...);
//    }

};

#endif //SERIALISTLOOPER_CONNECTABLE_H
