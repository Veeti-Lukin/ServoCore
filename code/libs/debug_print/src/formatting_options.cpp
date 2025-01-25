#include "debug_print/internal/formatting_options.h"

constexpr char K_BINARY_NUMERIC_BASE_CHAR = 'b';
constexpr char K_HEX_NUMERIC_BASE_CHAR    = 'x';

namespace debug_print::internal {

extern FormattingOptions formatting_options{};

bool tryParseFormattingOptions(char option) {
    bool is_formatting_option = true;

    switch (option) {
        case K_HEX_NUMERIC_BASE_CHAR:
            formatting_options.numeric_base = NumericBase::hex;
            break;
        case K_BINARY_NUMERIC_BASE_CHAR:
            formatting_options.numeric_base = NumericBase::binary;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            formatting_options.number_precision = (option - '0');
            break;
        default:
            is_formatting_option = false;
            break;
    }

    return is_formatting_option;
}

void resetFormattingOptions() {
    // reset options to defaults defined in FormattingOptions type definition
    formatting_options = FormattingOptions();
}

FormattingOptions getFormattingOptions() { return formatting_options; }

}  // namespace debug_print::internal