
#ifndef SERIALISTLOOPER_CONNECTABLE_H
#define SERIALISTLOOPER_CONNECTABLE_H

class Connectable {
public:
    Connectable() = default;
    virtual ~Connectable() = default;
    Connectable(const Connectable&) = delete;
    Connectable& operator=(const Connectable&) = delete;
    Connectable(Connectable&&)  noexcept = default;
    Connectable& operator=(Connectable&&)  noexcept = default;

    virtual bool connectable_to(juce::Component& component) = 0;

    virtual bool connect(Connectable& connectable) = 0;

};




#endif //SERIALISTLOOPER_CONNECTABLE_H
