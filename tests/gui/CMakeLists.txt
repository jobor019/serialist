
add_executable(vt_tests EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/juce_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/value_tree_tests.cpp
        deserialization_tests.cpp
)

target_link_libraries(vt_tests
        PRIVATE
        serialist::core
        serialist::gui
        Catch2::Catch2WithMain
        ${SERIALIST_MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${SERIALIST_FLAGS_JUCE}
)