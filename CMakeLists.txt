cmake_minimum_required(VERSION 3.15)

SET(CMAKE_CXX_STANDARD 17)

project(SerialistLooper VERSION 0.0.1)

list(APPEND CMAKE_PREFIX_PATH "/Applications/JUCE/install")
find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
find_package(Catch2 3 REQUIRED)
# add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE


set(SOURCES
        ${CMAKE_SOURCE_DIR}/src/events.h
        ${CMAKE_SOURCE_DIR}/src/exceptions.h
        ${CMAKE_SOURCE_DIR}/src/generation_graph.h
        ${CMAKE_SOURCE_DIR}/src/generation_layer.h
        ${CMAKE_SOURCE_DIR}/src/generative.h
        ${CMAKE_SOURCE_DIR}/src/generator.h
        ${CMAKE_SOURCE_DIR}/src/interpolator.h
        ${CMAKE_SOURCE_DIR}/src/mapping.h
        ${CMAKE_SOURCE_DIR}/src/oscillator.h
        ${CMAKE_SOURCE_DIR}/src/nop_parameter.h
        ${CMAKE_SOURCE_DIR}/src/parameter_policy.h
        ${CMAKE_SOURCE_DIR}/src/phasor.h
        ${CMAKE_SOURCE_DIR}/src/renderers.h
        ${CMAKE_SOURCE_DIR}/src/scheduler.h
        ${CMAKE_SOURCE_DIR}/src/transport.h
        ${CMAKE_SOURCE_DIR}/src/selector.h
        ${CMAKE_SOURCE_DIR}/src/utils.h
        ${CMAKE_SOURCE_DIR}/src/variable.h
        ${CMAKE_SOURCE_DIR}/src/vt_parameter.h
        ${CMAKE_SOURCE_DIR}/src/gui/looper_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/main_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/mapping_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/generator_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/oscillator_component.h
        )

set(MODULE_DEPENDENCIES_JUCE
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_devices)

set(FLAGS_JUCE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

#####################################################################################
## TARGET: SERIALIST LOOPER
#####################################################################################


juce_add_gui_app(serialistLooper
        # VERSION ...                       # Set this if the app version is different to the project version
        # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
        # ICON_SMALL ...
        # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
        # COMPANY_NAME ...                  # Specify the name of the app's author
        PRODUCT_NAME "Serialist Looper")

target_sources(serialistLooper
        PRIVATE
        ${CMAKE_SOURCE_DIR}/app/serialist_looper.cpp
        ${SOURCES})

target_compile_definitions(serialistLooper
        PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_VERSION>"
        )

target_link_libraries(serialistLooper
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${FLAGS_JUCE})

#####################################################################################
## TARGET: MIDI GENERATOR V1
#####################################################################################



juce_add_gui_app(midiGeneratorV1
        PRODUCT_NAME "MIDI Generator V1")


target_sources(midiGeneratorV1
        PRIVATE
        ${CMAKE_SOURCE_DIR}/app/midi_generator_v1.cpp
        ${SOURCES})

target_compile_definitions(midiGeneratorV1
        PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_VERSION>"
        )

target_link_libraries(midiGeneratorV1
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${FLAGS_JUCE})


#####################################################################################
## TARGET: TESTS
#####################################################################################

add_executable(tests
        ${CMAKE_SOURCE_DIR}/test/core_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/mapping_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/interpolator_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/selector_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/looper_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/generator_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/juce_tests.cpp
        )

target_link_libraries(tests
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${FLAGS_JUCE}
        )

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

