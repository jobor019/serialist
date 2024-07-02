
#ifndef SERIALIST_PARAMETER_KEYS_H
#define SERIALIST_PARAMETER_KEYS_H

#include <string>
#include "collections/vec.h"

// Note: namespace structure mirrors core/param/parameter_keys.h
namespace serialist::param {

namespace types {

const inline std::string modules = "MODULES";
const inline std::string module_t = "MODULE";
const inline std::string submodule = "SUBMODULE";

const inline std::string widget = "WIDGET";

const inline std::string config_coordinates = "CONFIG_COORDINATES";
const inline std::string coordinate = "COORDINATE";

const inline std::string app_version = "APP_VERSION";


} // namespace serialist::param::types


// ==============================================================================================

namespace serialist::properties {

const inline std::string associated_generative = "generative";
const inline std::string factory_class = "class";

const inline std::string layout = "layout";

const inline std::string app_version_major = "major";
const inline std::string app_version_minor = "minor";
const inline std::string app_version_rev = "rev";

} // namespace serialist::param::properties



} // namespace serialist::param

#endif //SERIALIST_PARAMETER_KEYS_H
