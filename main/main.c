#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "ps4.h"

#define TAG "main"

void handlePs4Event(ps4_t ps4, ps4_event_t event) {
  // // analog
  // ESP_LOGI(TAG, "Left Stick X: %d", ps4.analog.stick.lx);
  // ESP_LOGI(TAG, "Left Stick Y: %d", ps4.analog.stick.ly);
  // ESP_LOGI(TAG, "Right Stick X: %d", ps4.analog.stick.rx);
  // ESP_LOGI(TAG, "Right Stick Y: %d", ps4.analog.stick.ry);
  // ESP_LOGI(TAG, "L2: %d", ps4.analog.button.l2);
  // ESP_LOGI(TAG, "R2: %d", ps4.analog.button.r2);

  // // digital
  // ESP_LOGI(TAG, "D-Pad Up: %d", ps4.button.up);
  // ESP_LOGI(TAG, "D-Pad Down: %d", ps4.button.down);
  // ESP_LOGI(TAG, "D-Pad Left: %d", ps4.button.left);
  // ESP_LOGI(TAG, "D-Pad Right: %d", ps4.button.right);
  // ESP_LOGI(TAG, "D-Pad Up-Right: %d", ps4.button.upright);
  // ESP_LOGI(TAG, "D-Pad Down-Right: %d", ps4.button.downright);
  // ESP_LOGI(TAG, "D-Pad Up-Left: %d", ps4.button.upleft);
  // ESP_LOGI(TAG, "D-Pad Down-Left: %d", ps4.button.downleft);
  // ESP_LOGI(TAG, "Square: %d", ps4.button.square);
  // ESP_LOGI(TAG, "Cross: %d", ps4.button.cross);
  // ESP_LOGI(TAG, "Circle: %d", ps4.button.circle);
  // ESP_LOGI(TAG, "Triangle: %d", ps4.button.triangle);
  // ESP_LOGI(TAG, "L1: %d", ps4.button.l1);
  // ESP_LOGI(TAG, "R1: %d", ps4.button.r1);
  // ESP_LOGI(TAG, "L2: %d", ps4.button.l2);
  // ESP_LOGI(TAG, "R2: %d", ps4.button.r2);
  // ESP_LOGI(TAG, "L3: %d", ps4.button.l3);
  // ESP_LOGI(TAG, "R3: %d", ps4.button.r3);
  // ESP_LOGI(TAG, "Share: %d", ps4.button.share);
  // ESP_LOGI(TAG, "Options: %d", ps4.button.options);
  // ESP_LOGI(TAG, "PS: %d", ps4.button.ps);
  // ESP_LOGI(TAG, "TouchPad: %d", ps4.button.touchpad);

  // // Status
  // ESP_LOGI(TAG, "Battery: %d", ps4.status.battery);
  // ESP_LOGI(TAG, "Charging: %d", ps4.status.charging);
  // ESP_LOGI(TAG, "Audio: %d", ps4.status.audio);
  // ESP_LOGI(TAG, "Mic: %d", ps4.status.mic);

  // // Accelerometer
  // ESP_LOGI(TAG, "Accelerometer X: %d", ps4.sensor.accelerometer.x);
  // ESP_LOGI(TAG, "Accelerometer Y: %d", ps4.sensor.accelerometer.y);
  // ESP_LOGI(TAG, "Accelerometer Z: %d", ps4.sensor.accelerometer.z);

  // // Gyroscope
  // ESP_LOGI(TAG, "Gyroscope Z: %d", ps4.sensor.gyroscope.z);
}

void handlePs4Connection(uint8_t isConnected) {
  if (isConnected) {
    // ToDo: figure out how to know when the channel is free again
    // so this delay can be removed
    vTaskDelay(250 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "Connecteed!");
  } else {
    ESP_LOGI(TAG, "Disconnecteed!");
  }
}

void app_main(void) {
  nvs_flash_init();
  ps4SetEventCallback(&handlePs4Event);
  ps4SetConnectionCallback(&handlePs4Connection);

  uint8_t fakeMac[6] = {0x18, 0x3e, 0xef, 0xbb, 0x9c, 0x82};
  ps4SetBluetoothMacAddress(&fakeMac);
  ps4Init();

  uint8_t mac[6] = {0};
  ESP_ERROR_CHECK(esp_read_mac(&mac, ESP_MAC_BT));
  ESP_LOGI(TAG, "Bluetooth MAC - %x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2],
           mac[3], mac[4], mac[5]);
}
