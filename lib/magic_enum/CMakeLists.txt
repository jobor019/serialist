add_library(magicEnum INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/magic_enum/include/magic_enum.hpp)

target_include_directories(magicEnum
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/magic_enum/include)

