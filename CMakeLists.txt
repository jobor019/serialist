cmake_minimum_required(VERSION 3.15)

SET(CMAKE_CXX_STANDARD 17)

project(SerialistLooper VERSION 0.0.1)

list(APPEND CMAKE_PREFIX_PATH "/Applications/JUCE/install")
find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
find_package(Catch2 3 REQUIRED)
# add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE


add_subdirectory(lib)
add_subdirectory(GuiUtils/src)

set(SOURCES
        ${CMAKE_SOURCE_DIR}/src/collection.h
        ${CMAKE_SOURCE_DIR}/src/events.h
        ${CMAKE_SOURCE_DIR}/src/exceptions.h
        ${CMAKE_SOURCE_DIR}/src/facet.h
        ${CMAKE_SOURCE_DIR}/src/generative.h
        ${CMAKE_SOURCE_DIR}/src/generator.h
        ${CMAKE_SOURCE_DIR}/src/interpolation_adapter.h
        ${CMAKE_SOURCE_DIR}/src/interpolator.h
        ${CMAKE_SOURCE_DIR}/src/modular_generator.h
        ${CMAKE_SOURCE_DIR}/src/nop_parameter.h
        ${CMAKE_SOURCE_DIR}/src/oscillator.h
        ${CMAKE_SOURCE_DIR}/src/parameter_policy.h
        ${CMAKE_SOURCE_DIR}/src/phasor.h
        ${CMAKE_SOURCE_DIR}/src/scheduler.h
        ${CMAKE_SOURCE_DIR}/src/transport.h
        ${CMAKE_SOURCE_DIR}/src/sequence.h
        ${CMAKE_SOURCE_DIR}/src/serializable.h
        ${CMAKE_SOURCE_DIR}/src/selector.h
        ${CMAKE_SOURCE_DIR}/src/socket_policy.h
        ${CMAKE_SOURCE_DIR}/src/transform.h
        ${CMAKE_SOURCE_DIR}/src/vt_socket.h
        ${CMAKE_SOURCE_DIR}/src/source.h
        ${CMAKE_SOURCE_DIR}/src/utils.h
        ${CMAKE_SOURCE_DIR}/src/variable.h
        ${CMAKE_SOURCE_DIR}/src/voice.h
        ${CMAKE_SOURCE_DIR}/src/vt_parameter.h
        ${CMAKE_SOURCE_DIR}/src/gui/modules/generator_module.h
        ${CMAKE_SOURCE_DIR}/src/gui/modules/interpolation_module.h
        ${CMAKE_SOURCE_DIR}/src/gui/modules/note_source_module.h
        ${CMAKE_SOURCE_DIR}/src/gui/modules/oscillator_module.h
        ${CMAKE_SOURCE_DIR}/src/gui/modules/text_sequence_module.h
        ${CMAKE_SOURCE_DIR}/src/gui/views/note_view.h
        ${CMAKE_SOURCE_DIR}/src/gui/views/oscillator_view.h
        ${CMAKE_SOURCE_DIR}/src/gui/widgets/combobox_widget.h
        ${CMAKE_SOURCE_DIR}/src/gui/widgets/header_widget.h
        ${CMAKE_SOURCE_DIR}/src/gui/widgets/slider_widget.h
        ${CMAKE_SOURCE_DIR}/src/gui/widgets/toggle_button_widget.h
        ${CMAKE_SOURCE_DIR}/src/gui/configuration_layer_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/connectable.h
        ${CMAKE_SOURCE_DIR}/src/gui/connectable_dnd_controller.h
        ${CMAKE_SOURCE_DIR}/src/gui/connector_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/generative_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/interaction_visualizations.h
        ${CMAKE_SOURCE_DIR}/src/gui/look_and_feel.h
        ${CMAKE_SOURCE_DIR}/src/gui/main_component.h
        ${CMAKE_SOURCE_DIR}/src/gui/module_factory.h
        ${CMAKE_SOURCE_DIR}/src/io/keyboard_shortcuts.h
        ${CMAKE_SOURCE_DIR}/src/io/midi_config.h
        ${CMAKE_SOURCE_DIR}/src/io/renderers.h
        )

set(INCLUDES
        ${CMAKE_SOURCE_DIR}/src/gui
        ${CMAKE_SOURCE_DIR}/src/gui/modules
        ${CMAKE_SOURCE_DIR}/src/gui/views
        ${CMAKE_SOURCE_DIR}/src/gui/widgets
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/io
        ${CMAKE_SOURCE_DIR}/GuiUtils/src)

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

target_include_directories(serialistLooper PRIVATE
        ${INCLUDES})

target_compile_definitions(serialistLooper
        PRIVATE
        USE_JUCE=1 # Parameter class
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
## TARGET: PLAYGROUND
#####################################################################################

juce_add_gui_app(playground
        PRODUCT_NAME "Playground")


target_sources(playground
        PRIVATE
        ${CMAKE_SOURCE_DIR}/app/playground.cpp
        ${SOURCES})

target_include_directories(playground PRIVATE
        ${INCLUDES}
        )

target_compile_definitions(playground
        PRIVATE
        USE_JUCE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_VERSION>"
        )

target_link_libraries(playground
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        guiUtils
        magicEnum
        PUBLIC
        ${FLAGS_JUCE})


#####################################################################################
## TARGET: NOP TESTS
#####################################################################################

add_executable(tests
        ${CMAKE_SOURCE_DIR}/test/backend/oscillator_tests.cpp
#        ${CMAKE_SOURCE_DIR}/test/interpolator_tests.cpp
#        ${CMAKE_SOURCE_DIR}/test/selector_tests.cpp
#        ${CMAKE_SOURCE_DIR}/test/looper_tests.cpp
#        ${CMAKE_SOURCE_DIR}/test/generator_tests.cpp
        ${CMAKE_SOURCE_DIR}/test/backend/facet_tests.cpp
        )

target_link_libraries(tests
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${FLAGS_JUCE}
        magicEnum
        )

target_include_directories(tests PRIVATE
        ${INCLUDES}
        )

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)


#####################################################################################
## TARGET: VT_TESTS TESTS
#####################################################################################

add_executable(vtTests
        ${CMAKE_SOURCE_DIR}/test/juce/juce_tests.cpp
        )

target_link_libraries(vtTests
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${FLAGS_JUCE}
        )

target_compile_definitions(vtTests
        PRIVATE
        USE_JUCE=1
        )

target_include_directories(vtTests PRIVATE ${INCLUDES})

target_link_libraries(vtTests
        PRIVATE
        Catch2::Catch2WithMain
        magicEnum)


