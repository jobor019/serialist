add_executable(core_tests EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/classifiers_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/equal_distribution_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/facet_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/filters_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/histogram_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/random_tests.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/algo/voices_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/algo/fraction_tests.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/collections/range_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/scheduler_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/collections/vec_tests.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/allocator_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/interpolator_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/make_note_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/operator_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/oscillator_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/pulsator_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/random_pulsator_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/selector_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/sequence_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/variable_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generatives/phase_pulsator_tests.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/time_point_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/phase_accumulator_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/temporal/phase_tests.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/utility/math_tests.cpp
        generatives/phase_map_tests.cpp
        generatives/pulse_filter_test.cpp
        generatives/sample_and_hold_tests.cpp

)

target_link_libraries(core_tests
        PUBLIC
        serialist::core
        serialist::testutils
        PRIVATE
        Catch2::Catch2WithMain
)