SET(CORE_NAME core)

add_library(${CORE_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/filters.h
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/phase_node.h
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/phase_pulse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/pulse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/time_gate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/time_point_generators.h
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/transport.h

        ${CMAKE_CURRENT_SOURCE_DIR}/algo/pitch/notes.h

        ${CMAKE_CURRENT_SOURCE_DIR}/algo/random/equal_duration_sampling.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/random/random.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/random/weighted_random.h

        ${CMAKE_CURRENT_SOURCE_DIR}/algo/classifiers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/exponential.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/histogram.h

        ${CMAKE_CURRENT_SOURCE_DIR}/collections/circular_buffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/held.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/multi_voiced.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/queue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/range.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/scheduler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/vec.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/voices.h

        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/stereotypes/base_stereotypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/allocator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/interpolator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/make_note.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/operator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/phase_node.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/patternizer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/phase_map.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/phase_pulsator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/pulse_filter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/random_node.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/sample_and_hold.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/sequence.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/unit_pulse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/variable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/waveform.h

        ${CMAKE_CURRENT_SOURCE_DIR}/param/deserialization.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/nop_parameter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/nop_socket.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/socket_base.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/socket_handler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/string_serialization.h

        ${CMAKE_CURRENT_SOURCE_DIR}/policies/epsilon.h
        ${CMAKE_CURRENT_SOURCE_DIR}/policies/parameter_policy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/policies/policies.h
        ${CMAKE_CURRENT_SOURCE_DIR}/policies/socket_policy.h

        ${CMAKE_CURRENT_SOURCE_DIR}/types/domain_type.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/facet.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/fraction.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/index.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/meter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/partial_note.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/phase.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/time_point.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types/trigger.h

        ${CMAKE_CURRENT_SOURCE_DIR}/utility/enums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/mapping.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/math.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/optionals.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/stateful.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/traits.h

        ${CMAKE_CURRENT_SOURCE_DIR}/connectable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/exceptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generation_graph.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generative.h
        generatives/lowpass.h
        generatives/router.h
        param/multi_socket.h
        generatives/index_node.h
        generatives/trigger_node.h
        # BELOW: Automatically added - move to correct location above and append ${CMAKE_CURRENT_SOURCE_DIR}

)

add_library(${PROJECT_NAME}::${CORE_NAME} ALIAS ${CORE_NAME})

target_link_libraries(${CORE_NAME}
        INTERFACE
        magic_enum::magic_enum
        tl::expected
)

# TODO: This is not an ideal solution for includes
target_include_directories(${CORE_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../.. # for `#include <serialist/core/.../file.h>`
        ${CMAKE_CURRENT_SOURCE_DIR}/..    # for `#include <core/.../file.h>`
        ${CMAKE_CURRENT_SOURCE_DIR}       # for `#include ".../file.h"`
)
