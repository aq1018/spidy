#pragma once

#include <map>

#include "Address.hpp"
#include "Device.hpp"
#include "esp_gap_bt_api.h"

namespace bluetooth {

class GAPService {
 public:
  static GAPService* getInstance();

 private:
  static GAPService* instance;

  std::map<Address, Device, AddressComparator> devices;

  GAPService();

  static void handleEvents(esp_bt_gap_cb_event_t event,
                           esp_bt_gap_cb_param_t* param);

  // Device discovery result event
  void handleDiscovery(esp_bt_gap_cb_param_t* param);

  // Discovery state changed event
  void handleDiscoveryStateChange(esp_bt_gap_cb_param_t* param);

  // Get remote services event
  void handleRemoteService(esp_bt_gap_cb_param_t* param);

  // Get remote service record event
  void handleRemoteServiceRecord(esp_bt_gap_cb_param_t* param);

  // Authentication complete event
  void handleAuthComplete(esp_bt_gap_cb_param_t* param);

  // Legacy Pairing Pin code request
  void handlePairingPinRequest(esp_bt_gap_cb_param_t* param);

  // Security Simple Pairing User Confirmation request.
  void handleSSPConfirmationRequest(esp_bt_gap_cb_param_t* param);

  // Security Simple Pairing Passkey Notification
  void handleSSPKeyNotification(esp_bt_gap_cb_param_t* param);

  // Security Simple Pairing Passkey request
  void handleSSPKeyRequest(esp_bt_gap_cb_param_t* param);

  // Read RSSI event
  void handleReadRSSIDelta(esp_bt_gap_cb_param_t* param);

  // Config EIR data event
  void handleConfigEIRData(esp_bt_gap_cb_param_t* param);

  // Set AFH channels event
  void handleSetAFHChannels(esp_bt_gap_cb_param_t* param);

  // Read Remote Name event
  void handleReadRemoteName(esp_bt_gap_cb_param_t* param);

  // Mode change
  void handleModeChange(esp_bt_gap_cb_param_t* param);

  // remove bond device complete event
  void handleRemoveBondDeviceComplete(esp_bt_gap_cb_param_t* param);

  // QOS complete event
  void handleQoSComplete(esp_bt_gap_cb_param_t* param);

  // unknown event
  void handleUnknownEvent(esp_bt_gap_cb_param_t* param);
};

}  // namespace bluetooth
