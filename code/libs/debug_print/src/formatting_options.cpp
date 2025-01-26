#include "debug_print/internal/formatting_options.h"

constexpr char K_BINARY_NUMERIC_BASE_CHAR = 'b';
constexpr char K_HEX_NUMERIC_BASE_CHAR    = 'x';

namespace debug_print::internal {

extern FormattingOptions g_formatting_options{};

bool tryParseFormattingOptions(char option) {
    bool is_formatting_option = true;

    switch (option) {
        case K_HEX_NUMERIC_BASE_CHAR:
            g_formatting_options.numeric_base = NumericBase::hex;
            break;
        case K_BINARY_NUMERIC_BASE_CHAR:
            g_formatting_options.numeric_base = NumericBase::binary;
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
            g_formatting_options.number_precision = (option - '0');
            break;
        default:
            is_formatting_option = false;
            break;
    }

    return is_formatting_option;
}

void resetFormattingOptions() {
    // reset options to defaults defined in FormattingOptions type definition
    g_formatting_options = FormattingOptions();
}

void setFormattingOptions(FormattingOptions options) { g_formatting_options = options; }

FormattingOptions getFormattingOptions() { return g_formatting_options; }

}  // namespace debug_print::internal