#ifndef FORMATTING_OPTIONS_H
#define FORMATTING_OPTIONS_H

#include <cstdint>

namespace debug_print::internal {

enum class NumericBase : uint8_t {
    decimal,
    hex,
    binary,
};

struct FormattingOptions {
    NumericBase  numeric_base     = NumericBase::decimal;  // Default value restored with reset
    unsigned int number_precision = 5;                     // Default value restored with reset
};

[[nodiscard]] bool tryParseFormattingOptions(char option);

void resetFormattingOptions();

void setFormattingOptions(FormattingOptions options);

FormattingOptions getFormattingOptions();

}  // namespace debug_print::internal

#endif  // FORMATTING_OPTIONS_H
