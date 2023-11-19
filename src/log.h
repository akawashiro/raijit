#include <cstdint>
#include <iomanip>

#include <glog/logging.h>

#define LOG_KEY_VALUE(key, value) " " << key << "=" << value
#define LOG_SHOW(key) LOG_KEY_VALUE(#key, key)
#define LOG_64BITS(key) LOG_KEY_VALUE(#key, HexString(key, 16))
#define LOG_32BITS(key) LOG_KEY_VALUE(#key, HexString(key, 8))
#define LOG_16BITS(key) LOG_KEY_VALUE(#key, HexString(key, 4))
#define LOG_8BITS(key) LOG_KEY_VALUE(#key, HexString(key, 2))
#define LOG_BITS(key) LOG_KEY_VALUE(#key, HexString(key))
#define LOG_DWEHPE(type) LOG_KEY_VALUE(#type, ShowDW_EH_PE(type))

template <class T> std::string HexString(T num, int length = -1) {
  if (length == -1) {
    length = sizeof(T) / 2;
  }
  std::stringstream ss;
  ss << "0x" << std::uppercase << std::setfill('0') << std::setw(length)
     << std::hex << (+num & ((1 << (length * 4)) - 1));
  return ss.str();
}

inline std::string HexStringUint8Ptr(uint8_t *num) {
  int length = 16;
  std::stringstream ss;
  ss << "0x" << std::setfill('0') << std::setw(length)
     << std::hex
     << (reinterpret_cast<uint64_t>(num) & ((1 << (length * 4)) - 1));
  return ss.str();
}
