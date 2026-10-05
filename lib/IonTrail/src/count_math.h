#pragma once
#include <stdint.h>

// Saturate instead of silently wrapping a long-running instrument's count.
inline bool addCounts(uint32_t current, uint32_t increment, uint32_t& result) {
  if (increment > UINT32_MAX - current) {
    result = UINT32_MAX;
    return false;
  }
  result = current + increment;
  return true;
}
