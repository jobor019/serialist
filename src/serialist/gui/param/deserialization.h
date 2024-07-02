
#ifndef SERIALIST_DESERIALIZATION_H
#define SERIALIST_DESERIALIZATION_H

#include <juce_data_structures/juce_data_structures.h>
#include "core/exceptions.h"
#include "core/collections/vec.h"
#include "core/param/parameter_keys.h"


namespace serialist {

class VTDeserializationData {
public:
    /**
     * @throws ParameterTypeMissingError if vt is invalid
     */
    explicit VTDeserializationData(juce::ValueTree vt) : m_vt(std::move(vt)) {
        if (!m_vt.isValid()) {
            throw ParameterTypeMissingError("Invalid ValueTree");
        }
    }


    /**
     * @throws ParameterMissingError if vt doesn't have a child with that type
     */
    VTDeserializationData get_child_by_type(const std::string& type) const {
        auto child_tree = m_vt.getChildWithName({type});
        if (!child_tree.isValid()) {
            throw ParameterMissingError("Couldn't find child with type: " + type);
        }

        return VTDeserializationData(child_tree);
    }


    // Useful for getting all SOCKETS, etc. Note: not recursive, will only look at depth 1.
    Vec <VTDeserializationData> get_all_children_by_type(const std::string& type) const noexcept {
        Vec <VTDeserializationData> result;
        for (int i = 0; i < m_vt.getNumChildren(); ++i) {
            auto child = m_vt.getChild(i);
            if (child.hasType({type})) {
                result.append(VTDeserializationData(child));
            }
        }
        return result;
    }


    /**
     * @throws ParameterMissingError if vt doesn't have a child matching the provided conditions
     */
    VTDeserializationData get_child_by_property(std::string property_name
                                                , std::string property_value
                                                , std::optional <std::string> child_type = std::nullopt) const {
        juce::Identifier id{std::move(property_name)};

        if (child_type) {
            for (int i = 0; i < m_vt.getNumChildren(); ++i) {
                auto child = m_vt.getChild(i);
                if (child.hasType({child_type.value()}) && child.getProperty(id) == property_value) {
                    return VTDeserializationData(child);
                }
            }

            throw ParameterMissingError("Couldn't find child with type " + child_type.value() +
                                        " and property '" + property_name + "' = " + property_value);
        }

        auto child = m_vt.getChildWithProperty({property_name}, {property_value});
        if (!child.isValid()) {
            throw ParameterMissingError("Couldn't find child with property '" +
                                        property_name + "' = " + property_value);
        }

        return VTDeserializationData(child);
    }


    /**
     * @throws ParameterMissingError if vt doesn't have a child with that name
     */
    VTDeserializationData get_child_with_name(std::string name, std::optional <std::string> type = std::nullopt) const {
        return get_child_by_property(param::properties::member_name, std::move(name), std::move(type));
    }


    /**
     * @throws ParameterMissingError if vt doesn't have a child with that identifier
     */
    VTDeserializationData get_child_with_identifier(std::string id
                                                    , std::optional <std::string> type = std::nullopt) const {
        return get_child_by_property(param::properties::identifier, std::move(id), std::move(type));
    }


    bool has_property(const std::string& property_name) const noexcept { return m_vt.hasProperty({property_name}); }


    /**
     * @throws ParameterMissingError if vt doesn't have a property with that name
     */
    template<typename T>
    T get_property(const std::string& property_name) const {
        auto id = juce::Identifier(property_name);

        if (!m_vt.hasProperty(id)) {
            throw ParameterMissingError("Couldn't find property " + property_name);
        }

        return juce::VariantConverter<T>::fromVar(m_vt.getProperty(id));
    }


    std::string get_type() const noexcept {
        // Note: value tree type (XML element name) should always be defined
        return m_vt.getType().toString().toStdString();
    }


    bool has_name_in_parent() const noexcept { return m_vt.hasProperty({param::properties::member_name}); }


    bool has_identifier() const noexcept { return m_vt.hasProperty({param::properties::identifier}); }


    bool has_template_class() const noexcept { return m_vt.hasProperty({param::properties::template_class}); }


    std::string get_name_in_parent() const {
        if (!has_name_in_parent()) {
            throw ParameterMissingError("Couldn't find name in parent");
        }
        return m_vt.getProperty({param::properties::member_name}).toString().toStdString();
    }


private:
    juce::ValueTree m_vt;
};

} // namespace serialist


#endif //SERIALIST_DESERIALIZATION_H
