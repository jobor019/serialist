
#ifndef SERIALISTLOOPER_PARAMETER_KEYS_H
#define SERIALISTLOOPER_PARAMETER_KEYS_H

#include <string>
#include "collections/vec.h"


class [[deprecated("Use serialist::param::types and serialist::param::properties instead")]] ParameterTypes {
public:
    ParameterTypes() = delete;

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


// ==============================================================================================
// ==============================================================================================


namespace serialist {

namespace param::types {

const inline std::string root_tree = "ROOT";

const inline std::string generatives_tree = "GENERATIVES";
const inline std::string generative = "GENERATIVE";

const inline std::string sockets_tree = "SOCKETS";
const inline std::string socket = "SOCKET";

} // namespace serialist::param::types


// ==============================================================================================

namespace param::properties {

// Baseline
static const inline std::string template_class = "class";
static const inline std::string member_name = "name";
static const inline std::string identifier = "id";


// Socket
static const inline std::string connected_id = "connected_id";


// Commonly reoccurring properties
static const inline std::string trigger = "trigger";
static const inline std::string num_voices = "num_voices";
static const inline std::string enabled = "enabled";
static const inline std::string value = "v";

} // namespace serialist::param::properties


// ==============================================================================================

class Specification {
public:

    explicit Specification(std::string type) : m_type(std::move(type)) {}

    // TODO: Find solution for non-string static properties (version number, ...)

    // NOTE: Module-specific Constructors exist in their specific class (e.g. Generative::specification(...))

    Specification& with_static_property(std::string property_name, std::string property_value) {
        m_static_properties.append(std::make_pair(std::move(property_name), std::move(property_value)));
        return *this;
    }

    Specification& with_name_in_parent(std::string member_name) {
        return with_static_property(param::properties::member_name, std::move(member_name));
    }

    Specification& with_template_class(std::string template_class) {
        return with_static_property(param::properties::template_class, std::move(template_class));
    }

    Specification& with_identifier(std::string identifier) {
        return with_static_property(param::properties::identifier, std::move(identifier));
    }

    const std::string& type() const { return m_type; }

    const Vec<std::pair<std::string, std::string>>& static_properties() const { return m_static_properties; }

private:

    std::string m_type;
    Vec<std::pair<std::string, std::string>> m_static_properties;
};


} // namespace serialist


#endif //SERIALISTLOOPER_PARAMETER_KEYS_H
