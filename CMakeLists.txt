cmake_minimum_required(VERSION 3.15)

project(serialist_looper VERSION 0.0.1)

list(APPEND CMAKE_PREFIX_PATH "/Applications/JUCE/install")
find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
find_package(Catch2 3 REQUIRED)
# add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE


juce_add_gui_app(serialistLooper
        # VERSION ...                       # Set this if the app version is different to the project version
        # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
        # ICON_SMALL ...
        # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
        # COMPANY_NAME ...                  # Specify the name of the app's author
        PRODUCT_NAME "Serialist Looper")


target_sources(serialistLooper
        PRIVATE
        ${CMAKE_SOURCE_DIR}/app/Main.cpp
        ${CMAKE_SOURCE_DIR}/src/main_component.h
        ${CMAKE_SOURCE_DIR}/src/events.h
        ${CMAKE_SOURCE_DIR}/src/generation_graph.h
        ${CMAKE_SOURCE_DIR}/src/generation_layer.h
        ${CMAKE_SOURCE_DIR}/src/generator.h
        ${CMAKE_SOURCE_DIR}/src/graph_node.h
        ${CMAKE_SOURCE_DIR}/src/looper.h
        ${CMAKE_SOURCE_DIR}/src/map.h
        ${CMAKE_SOURCE_DIR}/src/oscillator.h
        ${CMAKE_SOURCE_DIR}/src/renderers.h
        ${CMAKE_SOURCE_DIR}/src/phasor.h
        ${CMAKE_SOURCE_DIR}/src/scheduler.h
        ${CMAKE_SOURCE_DIR}/src/transport.h
        ${CMAKE_SOURCE_DIR}/src/exceptions.h
        )

target_compile_definitions(serialistLooper
        PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_VERSION>"
        )

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

# juce_add_binary_data(GuiAppData SOURCES ...)

target_link_libraries(serialistLooper
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_devices
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        )

add_executable(tests
        ${CMAKE_SOURCE_DIR}/test/core_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/juce_tests.cpp
        )

target_link_libraries(tests
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_devices
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        )


target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

