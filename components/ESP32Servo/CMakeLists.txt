set(srcs "analogWrite.cpp" "ESP32PWM.cpp" "ESP32Servo.cpp" "ESP32Tone.cpp")
set(requires "arduino")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES "${requires}")
