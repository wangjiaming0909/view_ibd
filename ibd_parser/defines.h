#pragma once

#include <cstddef>
#include <cstdint>

#if __cplusplus >= 201703L
#else
#define  byte uint8_t
#endif
#define ulint unsigned long

template <typename T> T e(const T &t) {
  T ret;
  auto size = sizeof(T);
  unsigned char *d = reinterpret_cast<unsigned char *>(&ret);
  const unsigned char *s = reinterpret_cast<const unsigned char *>(&t);
  s += size - 1;
  for (size_t i = 0; i < size; ++i) {
    *d = *s;
    ++d;
    --s;
  }
  return ret;
}


static inline uint32_t mach_read_from_4(const std::byte *b) {
    return ((static_cast<uint32_t>(b[0]) << 24) |
        (static_cast<uint32_t>(b[1]) << 16) |
        (static_cast<uint32_t>(b[2]) << 8) |static_cast<uint32_t>(b[3]));
}


static inline uint16_t mach_read_from_2(const std::byte* b) {
  return (((ulint)(b[0]) << 8) | (ulint)(b[1]));
}


static inline uint64_t mach_read_from_8(const std::byte* b) {
  uint64_t u64;

  u64 = mach_read_from_4(b);
  u64 <<= 32;
  u64 |= mach_read_from_4(b+4);

  return u64;
}
