

#ifndef SERIALIST_LOOPER_MAP_H
#define SERIALIST_LOOPER_MAP_H

#include <vector>


// TODO: Implement getters, multielement MapElements, etc.

template<typename T>
class MapElement {
public:
    MapElement(std::initializer_list<T> values) : values{values} {}

    const T& temp_first() const {
        return values[0];
    }

private:
    std::vector<T> values;
};


// ==============================================================================================

template<typename T>
class Mapping {
public:
    Mapping(std::initializer_list<MapElement<T> > values) : mapping{values} {}

    const MapElement<T>& at(int index) const {
        return mapping.at(index);
    }

    int size() {
        return mapping.size();
    }

    // TODO: Convenience functions for insert, replace, swap, etc. Currently just a dummy
    //  copy of std::vector, but will be relevant later


private:
    std::vector<MapElement<T> > mapping;
};

#endif //SERIALIST_LOOPER_MAP_H
