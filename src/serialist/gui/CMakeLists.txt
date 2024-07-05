
SET(GUI_NAME gui)

add_library(${GUI_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/bases/connectable_dnd_controller.h
        ${CMAKE_CURRENT_SOURCE_DIR}/bases/connectable_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/bases/connector_component.h
        ${CMAKE_CURRENT_SOURCE_DIR}/bases/module_stereotypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/bases/widget_stereotypes.h

        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/drag_and_drop/drag_and_drop.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/drag_and_drop_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/generative_component.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/input_condition.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/input_events.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/input_handler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/input_mode.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/input_mode_map.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/interaction_visualizations.h
        ${CMAKE_CURRENT_SOURCE_DIR}/interaction/interaction_visualizer.h

        ${CMAKE_CURRENT_SOURCE_DIR}/io/key_state.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/keyboard_shortcuts.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/midi_config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/mouse_state.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/osc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/renderers.h

        ${CMAKE_CURRENT_SOURCE_DIR}/modules/generator_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/interpolation_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/note_source_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/osc_send_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/oscillator_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/pulsator_module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/text_sequence_module.h

        ${CMAKE_CURRENT_SOURCE_DIR}/param/vt_parameter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/vt_socket.h

        ${CMAKE_CURRENT_SOURCE_DIR}/views/note_view.h
        ${CMAKE_CURRENT_SOURCE_DIR}/views/oscillator_view.h

        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/combobox_widget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/header_widget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/multi_slider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/multi_slider/bar_slider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/multi_slider/clefs.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/multi_slider/multi_slider_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/multi_slider/multi_slider_action_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/multi_slider/multi_slider_element.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/multi_slider/score_slider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/slider_widget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/slider_widget_LEGACY.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/toggle_button_widget.h

        ${CMAKE_CURRENT_SOURCE_DIR}/configuration_layer_component.h
        ${CMAKE_CURRENT_SOURCE_DIR}/look_and_feel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/main_component.h
        ${CMAKE_CURRENT_SOURCE_DIR}/module_factory.h

        # TODO: Automatically added - move to correct location above and append ${CMAKE_CURRENT_SOURCE_DIR}
        param/vt_serializer.h
        param/parameter_keys.h
        policies/policies.h
        param/deserialization.h
        policies/parameter_policy.h
        policies/socket_policy.h
        policies/deserialization_policy.h
)

add_library(${PROJECT_NAME}::${GUI_NAME} ALIAS ${GUI_NAME})

target_link_libraries(${GUI_NAME}
        INTERFACE
        serialist::core
)

target_compile_definitions(${GUI_NAME}
        INTERFACE
        SERIALIST_OVERRIDE_POLICIES=1
)


# TODO: This is not an ideal solution for includes
target_include_directories(${GUI_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../.. # for `#include <serialist/gui/.../file.h>`
        ${CMAKE_CURRENT_SOURCE_DIR}/..    # for `#include <gui/.../file.h>`
        ${CMAKE_CURRENT_SOURCE_DIR}       # for `#include ".../file.h"`
)