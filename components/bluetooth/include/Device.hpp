#pragma once

#include <map>
#include <string>

#include "Address.hpp"

namespace bluetooth {

class COD {
 public:
  COD();
  static COD from(uint32_t cod);

  bool isValid;
  uint32_t svcMajor;
  uint32_t devMajor;
  uint32_t devMinor;
};

class Device {
 private:
  std::string name;
  uint8_t signal;
  COD cod;

 public:
  Device();

  void setName(const std::string& name);
  void setSignal(uint8_t signal);
  void setCOD(const COD& cod);

  const std::string& getName();
  const COD& getCOD();
  uint8_t getSignal();
};

}  // namespace bluetooth
