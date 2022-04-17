#include <cstring>

#include "Address.hpp"

using namespace bluetooth;

Address Address::from(const esp_bd_addr_t& addr) {
  Address a;
  std::memcpy(&a.addr, &addr, sizeof(esp_bd_addr_t));
  return a;
}

Address::Address() { std::memset(&addr, 0, sizeof(esp_bd_addr_t)); }

bool AddressComparator::operator()(const Address& lhs,
                                   const Address& rhs) const {
  return std::memcmp(&(lhs.addr), &(rhs.addr), sizeof(esp_bd_addr_t)) != 0;
}
