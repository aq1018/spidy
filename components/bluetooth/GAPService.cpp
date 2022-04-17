#include "GAPService.hpp"

#include "esp_bt_device.h"
#include "esp_err.h"
#include "esp_log.h"

#define TAG "GAPService"

using namespace bluetooth;

GAPService* GAPService::instance = nullptr;

GAPService* GAPService::getInstance() {
  if (!instance) instance = new GAPService;
  return instance;
}

uint8_t getSignalFromDiscoveryProp(const esp_bt_gap_dev_prop_t& prop) {
  return ((uint8_t*)prop.val)[0];
}

std::string getDeviceNameFromDiscoveryPropEIR(
    const esp_bt_gap_dev_prop_t& prop) {
  uint8_t* name = nullptr;
  uint8_t len = 0;

  if (!prop.val) {
    return std::string();
  }

  name = esp_bt_gap_resolve_eir_data((uint8_t*)prop.val,
                                     ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &len);

  if (!name) {
    name = esp_bt_gap_resolve_eir_data((uint8_t*)prop.val,
                                       ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &len);
  }

  if (!name) {
    return std::string();
  }

  return std::string(name, name + len);
}

std::string getDeviceNameFromDiscoveryPropBDName(
    const esp_bt_gap_dev_prop_t& prop) {
  return std::string((uint8_t*)prop.val, (uint8_t*)prop.val + prop.len);
}

COD getCODfromDiscoveryProp(const esp_bt_gap_dev_prop_t& prop) {
  return COD::from(((uint32_t*)prop.val)[0]);
}

GAPService::GAPService() {
  ESP_ERROR_CHECK(esp_bt_gap_register_callback(handleEvents));
}

void GAPService::handleDiscovery(esp_bt_gap_cb_param_t* param) {
  Device device;

  ESP_LOGI(TAG, "[ESP_BT_GAP_DISC_RES_EVT]");

  for (int i = 0; i < param->disc_res.num_prop; i++) {
    esp_bt_gap_dev_prop_t& prop = param->disc_res.prop[i];
    switch (prop.type) {
      case ESP_BT_GAP_DEV_PROP_BDNAME:
        device.setName(getDeviceNameFromDiscoveryPropBDName(prop));
        break;

      case ESP_BT_GAP_DEV_PROP_COD:
        device.setCOD(getCODfromDiscoveryProp(prop));
        break;

      case ESP_BT_GAP_DEV_PROP_RSSI:
        device.setSignal(getSignalFromDiscoveryProp(prop));
        break;

      case ESP_BT_GAP_DEV_PROP_EIR:
        device.setName(getDeviceNameFromDiscoveryPropEIR(prop));
        break;
    }
  }

  this->devices[Address::from(param->disc_res.bda)] = device;
}

void GAPService::handleDiscoveryStateChange(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_DISC_STATE_CHANGED_EVT] %d",
           param->disc_st_chg.state);
}

void GAPService::handleRemoteService(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_RMT_SRVCS_EVT]");
}

void GAPService::handleRemoteServiceRecord(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_RMT_SRVC_REC_EVT]");
}

void GAPService::handleAuthComplete(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_AUTH_CMPL_EVT]");
}

void GAPService::handlePairingPinRequest(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_PIN_REQ_EVT]");
}

void GAPService::handleSSPConfirmationRequest(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_CFM_REQ_EVT]");
}

void GAPService::handleSSPKeyNotification(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_KEY_NOTIF_EVT]");
}

void GAPService::handleSSPKeyRequest(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_KEY_REQ_EVT]");
}

void GAPService::handleReadRSSIDelta(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_READ_RSSI_DELTA_EVT]");
}

void GAPService::handleConfigEIRData(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_CONFIG_EIR_DATA_EVT] stat: %d, eir_type_num: %d",
           param->config_eir_data.stat, param->config_eir_data.eir_type_num);
  for (uint8_t i = 0; i < param->config_eir_data.eir_type_num; i++) {
    esp_bt_eir_type_t eir_type = param->config_eir_data.eir_type[i];

    ESP_LOGI(TAG, "eir_type[%d]=%d", i, eir_type);
  }
}

void GAPService::handleSetAFHChannels(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_SET_AFH_CHANNELS_EVT]");
}

void GAPService::handleReadRemoteName(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_READ_REMOTE_NAME_EVT]");
}

void GAPService::handleModeChange(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_MODE_CHG_EVT]");
}

void GAPService::handleRemoveBondDeviceComplete(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_REMOVE_BOND_DEV_COMPLETE_EVT]");
}

void GAPService::handleQoSComplete(esp_bt_gap_cb_param_t* param) {
  ESP_LOGI(TAG, "[ESP_BT_GAP_QOS_CMPL_EVT]");
}

void GAPService::handleUnknownEvent(esp_bt_gap_cb_param_t* param) {
  ESP_LOGW(TAG, "Unknown BT GAP event!");
}

void GAPService::handleEvents(esp_bt_gap_cb_event_t event,
                              esp_bt_gap_cb_param_t* param) {
  auto instance = getInstance();
  switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT:
      instance->handleDiscovery(param);
      break;

    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
      instance->handleDiscoveryStateChange(param);
      break;

    case ESP_BT_GAP_RMT_SRVCS_EVT:
      instance->handleRemoteService(param);
      break;

    case ESP_BT_GAP_RMT_SRVC_REC_EVT:
      instance->handleRemoteServiceRecord(param);
      break;

    case ESP_BT_GAP_AUTH_CMPL_EVT:
      instance->handleAuthComplete(param);
      break;

    case ESP_BT_GAP_PIN_REQ_EVT:
      instance->handlePairingPinRequest(param);
      break;

    case ESP_BT_GAP_CFM_REQ_EVT:
      instance->handleSSPConfirmationRequest(param);
      break;

    case ESP_BT_GAP_KEY_NOTIF_EVT:
      instance->handleSSPKeyNotification(param);
      break;

    case ESP_BT_GAP_KEY_REQ_EVT:
      instance->handleSSPKeyRequest(param);
      break;

    case ESP_BT_GAP_READ_RSSI_DELTA_EVT:
      instance->handleReadRSSIDelta(param);
      break;

    case ESP_BT_GAP_CONFIG_EIR_DATA_EVT:
      instance->handleConfigEIRData(param);
      break;

    case ESP_BT_GAP_SET_AFH_CHANNELS_EVT:
      instance->handleSetAFHChannels(param);
      break;

    case ESP_BT_GAP_READ_REMOTE_NAME_EVT:
      instance->handleReadRemoteName(param);
      break;

    case ESP_BT_GAP_MODE_CHG_EVT:
      instance->handleModeChange(param);
      break;

    case ESP_BT_GAP_REMOVE_BOND_DEV_COMPLETE_EVT:
      instance->handleRemoveBondDeviceComplete(param);
      break;

    case ESP_BT_GAP_QOS_CMPL_EVT:
      instance->handleQoSComplete(param);
      break;

    default:
      instance->handleUnknownEvent(param);
      break;
  }
}
