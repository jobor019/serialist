add_library(testutils EXCLUDE_FROM_ALL
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/node_runner.h
        ${CMAKE_CURRENT_SOURCE_DIR}/runner_results.h
        matchers/v11.h
        matchers/matchers_common.h
)

target_include_directories(testutils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(testutils INTERFACE Catch2::Catch2WithMain)

add_library(${PROJECT_NAME}::testutils ALIAS testutils)