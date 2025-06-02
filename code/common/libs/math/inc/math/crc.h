#ifndef COMMON_LIBS_MATH_CRC_H
#define COMMON_LIBS_MATH_CRC_H

#include <cstdint>
#include <span>

namespace math {

/**
 * @brief Computes CRC-8-CCITT (ATM) over a span of bytes using a lookup table.
 *
 * This implementation uses a precomputed 256-byte lookup table for fast CRC calculation.
 *
 * @param data A span of bytes to compute the CRC for.
 * @return Computed 8-bit CRC value.
 */
uint8_t generateCrc8(std::span<uint8_t> data);

}  // namespace math

#endif  // COMMON_LIBS_MATH_CRC_H
