#pragma once

#include "esp_bt_defs.h"

namespace bluetooth {

class Address {
 private:
  esp_bd_addr_t addr;

 public:
  Address();

  static Address from(const esp_bd_addr_t& addr);

  friend class AddressComparator;
};

class AddressComparator {
 public:
  bool operator()(const Address& lhs, const Address& rhs) const;
};

}  // namespace bluetooth
