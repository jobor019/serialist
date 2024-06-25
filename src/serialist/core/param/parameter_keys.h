
#ifndef SERIALISTLOOPER_PARAMETER_KEYS_H
#define SERIALISTLOOPER_PARAMETER_KEYS_H

#include <string>

class ParameterKeys {
public:
    ParameterKeys() = delete;

    static const inline std::string ROOT_TREE = "ROOT";
    static const inline std::string GENERATIVES_TREE = "GENERATIVES";
    static const inline std::string GENERATIVE = "GENERATIVE";

    static const inline std::string GENERATIVE_CLASS = "class";

    static const inline std::string GENERATIVE_SOCKETS_TREE = "SOCKETS";
    static const inline std::string GENERATIVE_SOCKET = "SOCKET";

    static const inline std::string TRIGGER = "trigger";
    static const inline std::string NUM_VOICES = "num_voices";
    static const inline std::string ENABLED = "enabled";

    static const inline std::string MODULES_TREE = "MODULES";
    static const inline std::string MODULE_NAME = "name";
    static const inline std::string MODULE_POSITION = "position";
    static const inline std::string MODULE_LAYOUT = "layout";

    static const inline std::string MODULE_INTERNALS_TREE = "INTERNALS";

    static const inline std::string ID_PROPERTY = "id";
};

#endif //SERIALISTLOOPER_PARAMETER_KEYS_H
