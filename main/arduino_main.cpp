/****************************************************************************
http://retro.moe/unijoysticle2

Copyright 2021 Ricardo Quesada

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#include "sdkconfig.h"
#ifndef CONFIG_BLUEPAD32_PLATFORM_ARDUINO
#error "Must only be compiled when using Bluepad32 Arduino platform"
#endif  // !CONFIG_BLUEPAD32_PLATFORM_ARDUINO

#include <Arduino.h>
#include <Bluepad32.h>
#include "servos.hpp"

GamepadPtr myGamepad = nullptr;

void onConnectedGamepad(GamepadPtr gp) {
    if (myGamepad != nullptr) {
        Serial.println("CALLBACK: Current gamepad is already connected. Ignored.");
        return;
    }

    Serial.print("CALLBACK: Gamepad is connected");

    GamepadProperties properties = gp->getProperties();
    Serial.print("Gamepad model: ");
    Serial.print(gp->getModelName());
    Serial.print(", VID/PID: ");
    Serial.print(properties.vendor_id, HEX);
    Serial.print(":");
    Serial.print(properties.product_id, HEX);
    Serial.println();
    myGamepad = gp;
}

void onDisconnectedGamepad(GamepadPtr gp) {
    if (myGamepad == gp) {
        Serial.println("CALLBACK: Current gamepad disconnected.");
        myGamepad = nullptr;
    } else {
        Serial.println("CALLBACK: Gamepad disconnected, but is not current gamepad");
    }
}

void setup() {
    Serial.begin(115200);

    Serial.println("===== PWM SETUP =====");
    pinMode(LED_BUILTIN, OUTPUT);
    servos_init();
    Serial.println("===== PWM SETUP END =====");

    String fv = BP32.firmwareVersion();
    Serial.print("Firmware: ");
    Serial.println(fv);

    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    BP32.forgetBluetoothKeys();
}

// Arduino loop function. Runs in CPU 1
void loop() {
    BP32.update();

    if (myGamepad && myGamepad->isConnected()) {
        if (myGamepad->dpad()) {
            if (myGamepad->dpad() & DPAD_UP) {
                servos_cmd(W_FORWARD, 1);
            } else if (myGamepad->dpad() & DPAD_DOWN) {
                servos_cmd(W_BACKWARD, 1);
            } else if (myGamepad->dpad() & DPAD_LEFT) {
                servos_cmd(W_LEFT, 1);
            } else if (myGamepad->dpad() & DPAD_RIGHT) {
                servos_cmd(W_RIGHT, 1);
            }
        } else if (myGamepad->y()) {
            servos_cmd(W_SHAKE, 1);
        } else if (myGamepad->x()) {
            servos_cmd(W_WAVE, 1);
        } else if (myGamepad->a()) {
            if (is_stand()) {
                servos_cmd(W_STAND_SIT, 0);
            } else {
                servos_cmd(W_STAND_SIT, 1);
            }
        } else if (myGamepad->b()) {
            servos_cmd(W_DANCE, 1);
        }
    }

    delay(140);
}
