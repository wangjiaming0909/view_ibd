#pragma once

#include <cstddef>
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
