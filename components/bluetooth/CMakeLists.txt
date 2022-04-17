cmake_minimum_required(VERSION 3.5)

file(GLOB srcs *.cpp)

set(include_dirs "include")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES nvs_flash bt
    )

    