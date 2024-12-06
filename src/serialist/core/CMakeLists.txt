SET(CORE_NAME core)

add_library(${CORE_NAME}
        INTERFACE # TODO: Temporary - shouldn't be a header-only library in the end
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/equal_duration_sampling.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/filters.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/meter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/phase_accumulator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/pulse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/time_gate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/time_point.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/time_point_generators.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/transport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/temporal/trigger.h

        ${CMAKE_CURRENT_SOURCE_DIR}/algo/pitch/notes.h

        ${CMAKE_CURRENT_SOURCE_DIR}/algo/classifiers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/exponential.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/facet.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/histogram.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/partial_note.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/random.h
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/weighted_random.h

        ${CMAKE_CURRENT_SOURCE_DIR}/collections/circular_buffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/held.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/multi_voiced.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/queue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/range.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/scheduler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/vec.h
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/voices.h

        ${CMAKE_CURRENT_SOURCE_DIR}/connectable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/exceptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generation_graph.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generative.h

        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/stereotypes/base_stereotypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/allocator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/auto_pulsator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/interpolator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/make_note.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/operator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/osc_sender.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/oscillator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/pulsator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/random_pulsator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/selector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/sequence.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/transform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/unit_pulse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/variable.h

        ${CMAKE_CURRENT_SOURCE_DIR}/param/nop_parameter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/socket_base.h
        ${CMAKE_CURRENT_SOURCE_DIR}/param/socket_handler.h

        ${CMAKE_CURRENT_SOURCE_DIR}/utility/enums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/math.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/optionals.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/stateful.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utility/traits.h

        # TODO: Automatically added - move to correct location above and append ${CMAKE_CURRENT_SOURCE_DIR}
        param/deserialization.h
        policies/policies.h
        param/string_serialization.h
        policies/parameter_policy.h
        policies/socket_policy.h
        utility/mapping.h
        algo/temporal/phase_pulse.h
        generatives/phase_pulsator.h
        algo/temporal/phase.h
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
