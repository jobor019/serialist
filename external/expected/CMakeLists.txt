# Simplistic CMakeLists.txt for expected to avoid certain includes in the source CMakeLists.txt

add_library(expected INTERFACE)
target_include_directories(expected INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/expected/include)

add_library(tl::expected ALIAS expected)