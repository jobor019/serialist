add_library(magicEnum INTERFACE
        magic_enum/include/magic_enum.hpp)

target_include_directories(magicEnum
        INTERFACE
        magic_enum/include)

