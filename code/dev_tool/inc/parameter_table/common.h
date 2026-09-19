#ifndef DEV_TOOL_PARAMETER_TABLE_COMMON_H
#define DEV_TOOL_PARAMETER_TABLE_COMMON_H

#include <cstdint>
#include <memory>
#include <vector>

#include <QString>
#include <QVariant>

#include "parameter_system/common.h"

namespace parameter_table {

/**
 * @brief Columns of the parameter tree, in the order they are shown.
 *
 * The name has to sit at index 0: Qt draws the tree expander in the first column, so any other
 * order would print the group names underneath the "ID" header, next to an expander belonging to
 * a different column.
 */
enum class Columns : size_t {
    name       = 0,
    id         = 1,
    category   = 2,
    access     = 3,
    value_type = 4,
    value      = 5,
};

static constexpr size_t K_COLUMN_COUNT = 6;

/**
 * @brief Roles the model answers on top of the standard Qt ones.
 *
 * Filtering and sorting run on these rather than on the displayed text, so that IDs compare as
 * numbers instead of as hex strings and the group member count never ends up being matched by
 * the filter.
 *
 * They also keep the delegates independent of the proxy models in front of the table model: a
 * role travels through any number of proxies untouched, while an index does not.
 */
enum Roles : int {
    /// Plain text this cell is matched against when filtering. Empty means "never matches".
    k_filter_text_role = Qt::UserRole + 1,
    /// Value to compare when sorting, in a type that compares correctly (an int, a double, ...).
    k_sort_role,
    /// RowData::Kind of the row, so views and delegates can tell a group from a parameter.
    k_row_kind_role,
    /// parameter_system::ParameterValueType of the row, for picking the right editor widget.
    k_value_type_role,
};

/**
 * @brief One row of the parameter tree: either a group heading or a single parameter.
 *
 * Rows form a tree. The tree always has a root node that is never shown; its children are the
 * top level rows. A device that reports no grouping simply puts every parameter directly under
 * that root, which renders as a flat table.
 *
 * Children are held by pointer so that their addresses stay stable no matter how the tree grows.
 * The model hands those addresses out through QModelIndex::internalPointer(), which only works
 * while they stay put.
 */
struct RowData {
    enum class Kind : uint8_t {
        group,      ///< An expandable row that only names a group of parameters.
        parameter,  ///< A leaf row describing a single parameter of the device.
    };

    Kind kind = Kind::parameter;

    /// Heading of a group row. Parameter rows take their name from meta_data instead.
    QString group_name;

    /// Only meaningful on parameter rows.
    parameter_system::ParameterMetaData meta_data = {};
    /// Only meaningful on parameter rows. Latest value read from (or written to) the device.
    QVariant value;

    /// Null on the root node only. The parent owns this node, so the pointer outlives it.
    RowData* parent = nullptr;
    /// Position of this node in parent->children. Cached because the model needs it constantly,
    /// and the tree is never reordered in place — sorting happens in the proxy model.
    int row_in_parent = 0;

    std::vector<std::unique_ptr<RowData>> children;

    /**
     * @brief Appends a child node and wires up its back pointers.
     * @param child The node to append. Moved from.
     * @return A non-owning pointer to the appended node, for adding children to it in turn.
     */
    RowData* addChild(RowData child) {
        std::unique_ptr<RowData> node = std::make_unique<RowData>(std::move(child));
        node->parent                  = this;
        node->row_in_parent           = static_cast<int>(children.size());

        children.push_back(std::move(node));
        return children.back().get();
    }

    /// @brief Number of parameter rows in this subtree, counting the ones in nested groups.
    [[nodiscard]] int parameterCount() const {
        int count = (kind == Kind::parameter) ? 1 : 0;
        for (const std::unique_ptr<RowData>& child : children) {
            count += child->parameterCount();
        }
        return count;
    }
};

/// @brief Builds a parameter row out of the metadata and value read from a device.
inline RowData makeParameterRow(const parameter_system::ParameterMetaData& meta_data, const QVariant& value) {
    RowData row;
    row.kind      = RowData::Kind::parameter;
    row.meta_data = meta_data;
    row.value     = value;
    return row;
}

/// @brief Builds a group heading row.
inline RowData makeGroupRow(const QString& name) {
    RowData row;
    row.kind       = RowData::Kind::group;
    row.group_name = name;
    return row;
}

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_COMMON_H
