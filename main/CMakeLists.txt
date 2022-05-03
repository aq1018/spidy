set(srcs "main.c" "arduino_main.cpp" "servos.cpp")

set(requires "bluepad32" "ESP32Servo")

# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
if(DEFINED ENV{BLUEPAD32_ARDUINO})
    list(APPEND srcs "arduino_main.cpp")
    list(APPEND requires "bluepad32_arduino" "arduino")
endif()

idf_component_register(SRCS "servos.cpp" "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES "${requires}")
