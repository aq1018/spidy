#include "Device.hpp"

#include "esp_gap_bt_api.h"

using namespace bluetooth;

COD::COD() : isValid(false) {}

COD COD::from(uint32_t cod) {
  COD rv;
  rv.isValid = esp_bt_gap_is_valid_cod(cod);

  if (rv.isValid) {
    rv.svcMajor = esp_bt_gap_get_cod_srvc(cod);
    rv.devMajor = esp_bt_gap_get_cod_major_dev(cod);
    rv.devMinor = esp_bt_gap_get_cod_minor_dev(cod);
  }

  return rv;
}

Device::Device() {}

void Device::setName(const std::string& name) { this->name = name; }
void Device::setSignal(uint8_t signal) { this->signal = signal; }
void Device::setCOD(const COD& cod) { this->cod = cod; }

const std::string& Device::getName() { return name; }
const COD& Device::getCOD() { return cod; }
uint8_t Device::getSignal() { return signal; }
