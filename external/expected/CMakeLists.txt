# Simplistic CMakeLists.txt for expected to avoid certain includes in the source CMakeLists.txt

add_library(expected INTERFACE expected/include/tl/expected.hpp)
target_include_directories(expected INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/expected/include/tl)

add_library(tl::expected ALIAS expected)