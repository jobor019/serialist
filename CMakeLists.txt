cmake_minimum_required(VERSION 3.15)

SET(CMAKE_CXX_STANDARD 17)

project(SerialistLooper VERSION 0.0.1)

list(APPEND CMAKE_PREFIX_PATH "/Applications/JUCE/install")
find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
find_package(Catch2 3 REQUIRED)

# add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library(serialistCore INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/exponential.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/connectable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/exceptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/facet.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generation_graph.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generative.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/note_source.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/make_note.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/param/nop_parameter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/operator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/oscillator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/param/parameter_policy.h
        src/core/algo/temporal/phase_accumulator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/auto_pulsator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/pulsator.h
        src/core/collections/scheduler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/transport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/time_point_generators.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/sequence.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/selector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/param/socket_handler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/param/socket_policy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/time_gate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/time_point.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/meter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/transform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/param/vt_socket.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/unit_pulse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/utility/enums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/utility/stateful.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/variable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/collections/circular_buffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/collections/voices.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/param/vt_parameter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/param/socket_base.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/random_pulsator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/weighted_random.h
        src/core/generatives/stereotypes/base_stereotypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/partial_note.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/generatives/allocator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/collections/vec.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/pitch/notes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/random.h
        src/core/collections/multi_voiced.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/collections/held.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/histogram.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/classifiers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/collections/range.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/trigger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/utility/traits.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/pulse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/algo/temporal/equal_duration_sampling.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/utility/math.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/utility/optionals.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/collections/queue.h
        src/core/algo/temporal/filters.h
        src/core/generatives/interpolator.h
        src/core/generatives/osc_sender.h
)

target_link_libraries(serialistCore INTERFACE magicEnum)

target_include_directories(serialistCore
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)



set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules/generator_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules/interpolation_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules/note_source_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules/osc_send_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules/oscillator_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules/pulsator_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules/text_sequence_module.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/views/note_view.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/views/oscillator_view.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/combobox_widget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/header_widget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/slider_widget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/slider_widget_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/toggle_button_widget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/multi_slider.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/multi_slider/clefs.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/multi_slider/multi_slider_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/multi_slider/multi_slider_action_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/multi_slider/multi_slider_element.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/multi_slider/score_slider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/multi_slider/bar_slider.h


        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/configuration_layer_component.h
        src/gui/bases/connectable_module.h
        src/gui/bases/connectable_dnd_controller.h
        src/gui/bases/connector_component.h
        src/io/key_state.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/look_and_feel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/main_component.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/module_factory.h
        src/gui/bases/module_stereotypes.h
        src/gui/bases/widget_stereotypes.h
        src/io/mouse_state.h


        ${CMAKE_CURRENT_SOURCE_DIR}/src/io/keyboard_shortcuts.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io/midi_config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io/osc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io/renderers.h


        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/drag_and_drop_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/generative_component.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/input_condition.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/input_events.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/input_handler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/input_mode.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/input_mode_map.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/interaction_visualizations.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/interaction_visualizer.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/interaction/drag_and_drop/drag_and_drop.h
        )



set(INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/modules
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/views
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io
)

set(MODULE_DEPENDENCIES_JUCE
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_devices
        juce::juce_osc)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/app/serialist_looper.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/app/midi_generator_v1.cpp
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

juce_add_gui_app(playground EXCLUDE_FROM_ALL
        PRODUCT_NAME "Playground")


target_sources(playground
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app/playground.cpp
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
        INTERFACE
        serialistCore
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        magicEnum
        PUBLIC
        ${FLAGS_JUCE})

#####################################################################################
## TARGET: OSC PLAYGROUND
#####################################################################################

juce_add_gui_app(oscPlayground EXCLUDE_FROM_ALL
        PRODUCT_NAME "OSC Playground")


target_sources(oscPlayground
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app/osc_playground.cpp
        ${SOURCES})

target_include_directories(oscPlayground PRIVATE
        ${INCLUDES}
        )

target_compile_definitions(oscPlayground
        PRIVATE
        USE_JUCE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_VERSION>"
        )

target_link_libraries(oscPlayground
        INTERFACE
        serialistCore
        PRIVATE
        ${MODULE_DEPENDENCIES_JUCE}
        magicEnum
        PUBLIC
        ${FLAGS_JUCE})


#####################################################################################
## TARGET: STATE PLAYGROUND
#####################################################################################

juce_add_gui_app(statePlayground EXCLUDE_FROM_ALL
        PRODUCT_NAME "State Playground")


target_sources(statePlayground
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app/state_playground.cpp
        ${SOURCES})

target_include_directories(statePlayground PRIVATE
        ${INCLUDES}
        )

target_compile_definitions(statePlayground
        PRIVATE
        USE_JUCE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_VERSION>"
        )

target_link_libraries(statePlayground
        INTERFACE
        serialistCore
        PRIVATE
        ${MODULE_DEPENDENCIES_JUCE}
        magicEnum
        PUBLIC
        ${FLAGS_JUCE})


#####################################################################################
## TARGET : MULTISLIDER PLAYGROUND
#####################################################################################

juce_add_gui_app(multiSliderPlayground EXCLUDE_FROM_ALL
        PRODUCT_NAME "MultiSlider Playground")


target_sources(multiSliderPlayground
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app/multi_slider_playground.cpp
        ${SOURCES})

target_include_directories(multiSliderPlayground PRIVATE
        ${INCLUDES}
        )

target_compile_definitions(multiSliderPlayground
        PRIVATE
        USE_JUCE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialistLooper,JUCE_VERSION>"
        )

target_link_libraries(multiSliderPlayground
        INTERFACE
        serialistCore
        PRIVATE
        ${MODULE_DEPENDENCIES_JUCE}
        magicEnum
        PUBLIC
        ${FLAGS_JUCE})



#####################################################################################
## TARGET: NOP PLAYGROUND
#####################################################################################

add_executable(nopPlayground EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/app/nop_playground.cpp
)

target_link_libraries(nopPlayground
        INTERFACE
        serialistCore
        PUBLIC
        magicEnum
        )

target_include_directories(nopPlayground
        PRIVATE
        ${INCLUDES}
        )

#####################################################################################
## TARGET: NOP TESTS
#####################################################################################

add_executable(tests EXCLUDE_FROM_ALL
        test/backend/collections/range_tests.cpp
        test/backend/collections/vec_tests.cpp
        test/backend/collections/scheduler_tests.cpp
        test/backend/algo/voices_tests.cpp
        test/backend/algo/random_tests.cpp
        test/backend/algo/histogram_tests.cpp
        test/backend/algo/transport_tests.cpp
        test/backend/algo/classifiers_tests.cpp
        test/backend/generatives/allocator_tests.cpp
        test/backend/generatives/sequence_tests.cpp
        test/backend/generatives/variable_tests.cpp
        test/backend/algo/equal_distribution_tests.cpp
        test/backend/generatives/random_pulsator_tests.cpp
        test/backend/algo/filters_tests.cpp
        test/backend/generatives/oscillator_tests.cpp
        test/backend/generatives/operator_tests.cpp
        test/backend/generatives/interpolator_tests.cpp
        test/backend/algo/facet_tests.cpp
        test/backend/generatives/selector_tests.cpp
        test/backend/generatives/pulsator_tests.cpp
        test/backend/generatives/make_note_tests.cpp
        test/backend/utility/math_tests.cpp
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

add_executable(vtTests EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/test/juce/juce_tests.cpp
        test/juce/value_tree_tests.cpp
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


