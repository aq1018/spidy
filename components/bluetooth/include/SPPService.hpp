#pragma once

#include <string>

#include "esp_spp_api.h"

namespace bluetooth {

class SPPService {
 public:
  static SPPService* getInstance();

 private:
  static SPPService* instance;

  SPPService();

  static void handleEvents(esp_spp_cb_event_t event, esp_spp_cb_param_t* param);

  // SPP is initialized
  void handleInit(esp_spp_cb_param_t* param);

  // SPP is shut down
  void handleUninit(esp_spp_cb_param_t* param);

  // SPP service started
  void handleStart(esp_spp_cb_param_t* param);

  // SDP discovery complete
  void handleDiscovery(esp_spp_cb_param_t* param);

  // SPP client connection initialized
  void handleClientInit(esp_spp_cb_param_t* param);

  // SPP client connection open
  void handleClientOpen(esp_spp_cb_param_t* param);

  // SPP client connection closed
  void handleClientClose(esp_spp_cb_param_t* param);

  // ESP_SPP_MODE_CB only
  // SPP connection received data
  void handleData(esp_spp_cb_param_t* param);

  // ESP_SPP_MODE_CB only
  // SPP connection congestion status changed
  void handleCongestion(esp_spp_cb_param_t* param);

  // SPP write operation completed
  void handleWrite(esp_spp_cb_param_t* param);

  // SPP Server connection open
  void handleServerOpen(esp_spp_cb_param_t* param);

  // SPP Server connection stopped
  void handleServerStop(esp_spp_cb_param_t* param);

  // SPP unknown even handler
  void handleUnknownEvent(esp_spp_cb_param_t* param);
};

}  // namespace bluetooth
