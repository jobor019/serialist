add_library(testutils EXCLUDE_FROM_ALL
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/node_runner.h
        ${CMAKE_CURRENT_SOURCE_DIR}/run_results.h
        ${CMAKE_CURRENT_SOURCE_DIR}/matchers/v11.h
        ${CMAKE_CURRENT_SOURCE_DIR}/matchers/matchers_common.h
        generators.h
)

target_include_directories(testutils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(testutils INTERFACE Catch2::Catch2WithMain)

add_library(${PROJECT_NAME}::testutils ALIAS testutils)


add_executable(testutils_tests EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/v11_tests.cpp
)

target_link_libraries(testutils_tests
        PRIVATE
        testutils
        serialist::core
        Catch2::Catch2WithMain
)