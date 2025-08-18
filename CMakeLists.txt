cmake_minimum_required(VERSION 3.15)

SET(CMAKE_CXX_STANDARD 17)

project(serialist VERSION 0.0.1)

enable_testing()


# Default module dependencies for all JUCE-dependent targets
set(SERIALIST_MODULE_DEPENDENCIES_JUCE
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_devices
        juce::juce_osc)

# Default flags for all JUCE-dependent targets
set(SERIALIST_FLAGS_JUCE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Export utility flags to parent scope when building as a dependency
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(SERIALIST_MODULE_DEPENDENCIES_JUCE "${SERIALIST_MODULE_DEPENDENCIES_JUCE}" PARENT_SCOPE)
    set(SERIALIST_FLAGS_JUCE "${SERIALIST_FLAGS_JUCE}" PARENT_SCOPE)
endif()



add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/serialist)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
