
#ifndef SERIALIST_VARIANT_CONVERTER_H
#define SERIALIST_VARIANT_CONVERTER_H

#include <juce_data_structures/juce_data_structures.h>

// ==============================================================================================
// std::string
// ==============================================================================================


// ==============================================================================================
// Facet
// ==============================================================================================



// ==============================================================================================
// std::optional<T>
// ==============================================================================================

template<typename T>
struct juce::VariantConverter<std::optional<T>> {
    static std::optional<T> fromVar(const juce::var& var) {
        if (var.isVoid()) {
            return std::nullopt;
        } else {
            return juce::VariantConverter<T>::fromVar(var);
        }
    }

    static juce::var toVar(const std::optional<int>& value) {
        if (value) {
            return juce::VariantConverter<int>::toVar(*value);
        } else {
            return {};
        }
    }
};

#endif //SERIALIST_VARIANT_CONVERTER_H
