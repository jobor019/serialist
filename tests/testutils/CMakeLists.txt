add_library(testutils EXCLUDE_FROM_ALL
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/node_runner.h
        ${CMAKE_CURRENT_SOURCE_DIR}/results.h
        ${CMAKE_CURRENT_SOURCE_DIR}/matchers/m11.h
        ${CMAKE_CURRENT_SOURCE_DIR}/matchers/matchers_common.h
        generators.h
        exceptions.h
        condition.h
        conditions/c11.h
        events.h
        conditions/c1m.h
        matchers/m1m.h
        conditions/c1s.h
        matchers/m1s.h
        conditions/cms.h
        matchers/mms.h
)

target_include_directories(testutils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(testutils INTERFACE Catch2::Catch2WithMain)

add_library(${PROJECT_NAME}::testutils ALIAS testutils)


add_executable(testutils_tests EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/m11_tests.cpp
        tests/node_runner_tests.cpp
        tests/m1m_tests.cpp
        tests/mms_tests.cpp
)

target_link_libraries(testutils_tests
        PRIVATE
        testutils
        serialist::core
        Catch2::Catch2WithMain
)