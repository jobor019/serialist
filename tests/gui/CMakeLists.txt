
add_executable(vt-tests EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/juce_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/value_tree_tests.cpp
)

target_link_libraries(vt-tests
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${FLAGS_JUCE}
)

target_compile_definitions(vt-tests
        PRIVATE
        USE_JUCE=1
)

target_include_directories(vt-tests PRIVATE ${INCLUDES})

target_link_libraries(vt-tests
        PRIVATE
        Catch2::Catch2WithMain
        magicEnum)