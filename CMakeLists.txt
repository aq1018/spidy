# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

list(APPEND EXTRA_COMPONENT_DIRS 
    "vendor/arduino" 
    "vendor/bluepad32/src/components/bluepad32"
    "vendor/bluepad32/src/components/bluepad32_arduino"
)


# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
set(ENV{BLUEPAD32_ARDUINO} TRUE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(spidey)
