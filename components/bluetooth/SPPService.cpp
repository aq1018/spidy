#include "SPPService.hpp"

#include "esp_bt_device.h"
#include "esp_err.h"
#include "esp_gap_bt_api.h"
#include "esp_log.h"
#include "esp_spp_api.h"

#define TAG "SPPService"

using namespace bluetooth;

SPPService* SPPService::instance = nullptr;

SPPService* SPPService::getInstance() {
  if (!instance) {
    instance = new SPPService;
    ESP_ERROR_CHECK(esp_spp_init(ESP_SPP_MODE_CB));
  }

  return instance;
}

SPPService::SPPService() {
  ESP_ERROR_CHECK(esp_spp_register_callback(&handleEvents));
}

void SPPService::handleInit(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_INIT_EVT] %d", param->init.status);

  // if (param->init.status == ESP_SPP_SUCCESS) {
  //   // TODO specify handler from outside of class
  //   ESP_ERROR_CHECK(esp_bt_dev_set_device_name(spp_config->device_name));
  //   ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(spp_config->connection_mode,
  //                                            spp_config->discovery_mode));

  //   ESP_ERROR_CHECK(esp_bt_gap_start_discovery(
  //       spp_config->inq_mode, spp_config->inq_len, spp_config->inq_num_rsp));
  // }
}

void SPPService::handleUninit(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_UNINIT_EVT] %d", param->uninit.status);
}

void SPPService::handleStart(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_START_EVT] %d", param->start.status);
}

void SPPService::handleDiscovery(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_DISCOVERY_COMP_EVT] %d", param->disc_comp.status);
}

void SPPService::handleClientInit(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_CL_INIT_EVT] %d", param->cl_init.status);
}

void SPPService::handleClientOpen(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_OPEN_EVT] %d", param->open.status);
}

void SPPService::handleClientClose(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_CLOSE_EVT] %d", param->close.status);
}

void SPPService::handleData(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_DATA_IND_EVT] %d", param->data_ind.status);
}

void SPPService::handleCongestion(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_CONG_EVT] %d", param->cong.status);
}

void SPPService::handleWrite(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_WRITE_EVT] %d", param->write.status);
}

void SPPService::handleServerOpen(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_SRV_OPEN_EVT] %d", param->srv_open.status);
}

void SPPService::handleServerStop(esp_spp_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_SPP_SRV_STOP_EVT] %d", param->srv_stop.status);
}

void SPPService::handleUnknownEvent(esp_spp_cb_param_t* param) {
  ESP_LOGW(TAG, "[ESP_SPP Event Unknown]");
}

void SPPService::handleEvents(esp_spp_cb_event_t event,
                              esp_spp_cb_param_t* param) {
  auto instance = getInstance();
  switch (event) {
    case ESP_SPP_INIT_EVT:
      instance->handleInit(param);
      break;

    case ESP_SPP_START_EVT:
      instance->handleStart(param);
      break;

    case ESP_SPP_UNINIT_EVT:
      instance->handleUninit(param);
      break;

    case ESP_SPP_DISCOVERY_COMP_EVT:
      instance->handleDiscovery(param);
      break;

    case ESP_SPP_CL_INIT_EVT:
      instance->handleClientInit(param);
      break;

    case ESP_SPP_OPEN_EVT:
      instance->handleClientOpen(param);
      break;

    case ESP_SPP_CLOSE_EVT:
      instance->handleClientClose(param);
      break;

    case ESP_SPP_DATA_IND_EVT:
      instance->handleData(param);
      break;

    case ESP_SPP_CONG_EVT:
      instance->handleCongestion(param);
      break;

    case ESP_SPP_WRITE_EVT:
      instance->handleWrite(param);
      break;

    case ESP_SPP_SRV_OPEN_EVT:
      instance->handleServerOpen(param);
      break;

    case ESP_SPP_SRV_STOP_EVT:
      instance->handleServerStop(param);
      break;

    default:
      instance->handleUnknownEvent(param);
      break;
  }
}
