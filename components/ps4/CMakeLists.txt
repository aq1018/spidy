cmake_minimum_required(VERSION 3.5)

file(GLOB srcs *.c)

set(include_dirs "include")
list(APPEND include_dirs 
    "${IDF_PATH}/components/bt/common/include"
    "${IDF_PATH}/components/bt/host/bluedroid/common/include"
    "${IDF_PATH}/components/bt/host/bluedroid/stack/include"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES nvs_flash bt
    )

    