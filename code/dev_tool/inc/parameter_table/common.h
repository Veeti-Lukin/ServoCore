#ifndef DEV_TOOL_PARAMETER_TABLE_COMMON_H
#define DEV_TOOL_PARAMETER_TABLE_COMMON_H

#include <QVariant>
#include <cstdint>

#include "parameter_system/ParameterDelegate.h"

namespace parameter_table {

enum class Columns : size_t {
    id     = 0,
    name   = 1,
    access = 2,
    type   = 3,
    value  = 4,
};

static constexpr size_t K_COLUMN_COUNT = 5;

struct RowData {
    parameter_system::ParameterMetaData meta_data;
    QVariant                            value;
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_COMMON_H
