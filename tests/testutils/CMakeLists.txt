add_library(testutils INTERFACE EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/generative_runner.h
        runner_results.h
)

target_include_directories(testutils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(testutils INTERFACE Catch2::Catch2WithMain)