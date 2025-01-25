#include "debug_print/internal/print_type_overloads.h"

#include <climits>
#include <cmath>
#include <cstddef>
#include <span>

#include "debug_print/debug_print.h"
#include "debug_print/internal/formatting_options.h"

namespace debug_print::internal {

constexpr const char* K_HEXA_DIGITS = "0123456789ABCDEF";

namespace helpers {

template <typename IntegerType>
consteval uint64_t calculateBufferSizeForInteger(NumericBase base) {
    uint64_t number_of_bits = sizeof(IntegerType) * CHAR_BIT;

    switch (base) {
        case NumericBase::decimal:
            return log10(pow(2, number_of_bits) - 1) + 1;
        case NumericBase::hex:
            return ceil(log(pow(2, number_of_bits) - 1) / (4 * log(2))) + 1;
        case NumericBase::binary:
            return number_of_bits;
        default:
            return 0;
    }
}

}  // namespace helpers

void printType(char c) { getPutCharFunction()(c); }

void printType(const char* str) {
    for (const char* p = str; *p != '\0'; ++p) {
        getPutCharFunction()(*p);
    }
}

void printType(unsigned int value) { printType((uint64_t)value); }

void printType(uint8_t value) { printType((uint64_t)value); }

void printType(uint16_t value) { printType((uint64_t)value); }

void printType(uint32_t value) { printType((uint64_t)value); }

void printType(uint64_t value) {
    // TODO refactor this
    switch (getFormattingOptions().numeric_base) {
        case NumericBase::decimal: {
            if (value == 0) {
                getPutCharFunction()('0');
                return;
            }

            char buffer[helpers::calculateBufferSizeForInteger<uint64_t>(NumericBase::decimal)];

            size_t i = 0;
            while (value > 0) {
                buffer[i] = (value % 10) + '0';
                value /= 10;
                if (value > 0) i++;
            }

            while (true) {
                if (i == SIZE_MAX) {
                    break;
                }

                getPutCharFunction()(buffer[i]);
                i--;
            }
        } break;
        case NumericBase::hex: {
            getPutCharFunction()('0');
            getPutCharFunction()('x');

            if (value == 0) {
                getPutCharFunction()('0');
                return;
            }

            char buffer[helpers::calculateBufferSizeForInteger<uint64_t>(NumericBase::hex)];

            size_t i = 0;
            while (value > 0) {
                buffer[i] = K_HEXA_DIGITS[value % 16];
                value /= 16;
                if (value > 0) i++;
            }

            while (true) {
                if (i == SIZE_MAX) {
                    break;
                }

                getPutCharFunction()(buffer[i]);
                i--;
            }
        } break;
        case NumericBase::binary: {
            getPutCharFunction()('0');
            getPutCharFunction()('b');

            if (value == 0) {
                getPutCharFunction()('0');
                return;
            }

            char buffer[helpers::calculateBufferSizeForInteger<uint64_t>(NumericBase::binary)];

            size_t i = 0;
            while (value > 0) {
                buffer[i] = (value % 2) ? '1' : '0';
                value /= 2;
                i++;
            }

            // Print the binary representation in reverse
            while (i > 0) {
                i--;
                getPutCharFunction()(buffer[i]);
            }
        } break;
    }
}
void printType(int value) { printType((int64_t)value); }

void printType(int8_t value) { printType((int64_t)value); }

void printType(int16_t value) { printType((int64_t)value); }

void printType(int32_t value) { printType((int64_t)value); }

void printType(int64_t value) {
    if (value < 0) {
        getPutCharFunction()('-');

        if (value == INT64_MIN) {
            // negative max is one bigger than positive
            // so add one before swapping the integer to positive
            value += 1;
        }

        value = -value;
    }

    printType((uint64_t)value);
}

void printType(float value) { printType((double)value); }

void printType(double value) {
    unsigned int precision = getFormattingOptions().number_precision;

    if (value < 0) {
        getPutCharFunction()('-');
        value = -value;
    }

    // handle integer part
    int64_t integer_part = (int64_t)value;
    printType(integer_part);

    // handle fractional part
    double fractional_part = value - integer_part;
    getPutCharFunction()('.');

    for (; precision != 0; precision--) {
        fractional_part *= 10;
        size_t digit = (size_t)fractional_part;
        fractional_part -= digit;

        getPutCharFunction()(digit + '0');
    }
}

void printType(bool value) {
    if (value)
        printType("true");
    else
        printType("false");
}

}  // namespace debug_print::internal