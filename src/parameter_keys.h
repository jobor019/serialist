
#ifndef SERIALISTLOOPER_PARAMETER_KEYS_H
#define SERIALISTLOOPER_PARAMETER_KEYS_H

#include <string>

class ParameterKeys {
public:
    ParameterKeys() = delete;

    static const inline std::string ROOT_TREE = "ROOT";
    static const inline std::string GENERATIVES_TREE = "GENERATIVES";
    static const inline std::string GENERATIVE_CLASS = "CLASS";
    static const inline std::string GENERATIVE_SOCKETS = "SOCKETS";
    static const inline std::string MODULES_TREE = "MODULES";
    static const inline std::string MODULE_NAME = "NAME";
    static const inline std::string MODULE_POSITION = "POSITION";
    static const inline std::string MODULE_LAYOUT = "LAYOUT";
    static const inline std::string MODULE_INTERNALS = "INTERNALS";
};

#endif //SERIALISTLOOPER_PARAMETER_KEYS_H
