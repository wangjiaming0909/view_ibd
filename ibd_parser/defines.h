#pragma once
#include <cstddef>
#include <cstdint>
#ifdef TESTING
#define PRIVATE public
#define PROTECTED public
#else
#define PRIVATE private
#define PROTECTED protected
#endif

using byte = std::byte;
#define ulint unsigned long

#define PAGE_SIZE 16384
#define PAGE_BTR_SEG_LEAF 36

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


static inline uint32_t mach_read_from_4(const byte *b) {
    return ((static_cast<uint32_t>(b[0]) << 24) |
        (static_cast<uint32_t>(b[1]) << 16) |
        (static_cast<uint32_t>(b[2]) << 8) |static_cast<uint32_t>(b[3]));
}

static inline uint8_t mach_read_from_1(const byte*b) {
    return uint8_t(b[0]);
}

static inline uint16_t mach_read_from_2(const byte* b) {
  return (((ulint)(b[0]) << 8) | (ulint)(b[1]));
}


static inline uint64_t mach_read_from_8(const byte* b) {
  uint64_t u64;

  u64 = mach_read_from_4(b);
  u64 <<= 32;
  u64 |= mach_read_from_4(b+4);

  return u64;
}

static inline ulint rec_get_bit_field_1(const byte* rec, ulint offs, ulint mask, ulint shift) {
  return ((mach_read_from_1(rec-offs) & mask) >> shift);
}

static inline uint16_t rec_get_bit_field_2(const byte *rec, ulint offs, ulint mask, ulint shift) {
  return ((mach_read_from_2(rec-offs) & mask) >> shift);
}

static inline ulint align_offset(const void*ptr, ulint align_no) {
  return (((ulint)ptr) & (align_no - 1));
}

static inline void* align_down(const void*ptr, ulint align_no) {
  return ((void*)((((ulint)ptr)) & ~(align_no - 1)));
}

static inline byte* page_align(const void* ptr) {
    return (byte*)align_down(ptr, PAGE_SIZE);
}
