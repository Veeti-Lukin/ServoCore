#include "parameter_table/ParameterTableModel.h"

#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QPalette>

#include "helpers.h"
#include "parameter_system/common.h"
#include "utils/enum_helpers.h"

namespace parameter_table {
namespace {

/// @brief Display text of a cell of a parameter row.
QVariant parameterDisplayText(const RowData& row, Columns column) {
    switch (column) {
        case Columns::name:
            return row.meta_data.name;
        case Columns::id:
            return helpers::intToHexString(row.meta_data.id);
        case Columns::category:
            return helpers::stringViewToQString(utils::enumToString(row.meta_data.category));
        case Columns::access:
            return helpers::stringViewToQString(utils::enumToString(row.meta_data.read_write_access));
        case Columns::value_type:
            return helpers::stringViewToQString(utils::enumToString(row.meta_data.value_type));
        case Columns::value:
            return row.value;
    }
    return {};
}

/**
 * @brief Text the filter matches a cell against.
 *
 * Mostly the displayed text, so that what the user sees is what they can search for. Group rows
 * only expose their heading: the member count shown next to it is decoration, and matching "12"
 * against it would be surprising.
 */
QVariant filterText(const RowData& row, Columns column) {
    if (row.kind == RowData::Kind::group) {
        return (column == Columns::name) ? row.group_name : QVariant{};
    }
    return parameterDisplayText(row, column);
}

/**
 * @brief Value the proxy model compares when sorting by a column.
 *
 * Returned in a type that compares the way the column reads: IDs as numbers rather than as the
 * hex strings they are shown as, values as numbers rather than as text.
 *
 * Group rows answer with their heading whatever the column, so that clicking a header reorders
 * the parameters inside each group while leaving the groups themselves in a stable, alphabetical
 * order.
 */
QVariant sortValue(const RowData& row, Columns column) {
    if (row.kind == RowData::Kind::group) return row.group_name;

    switch (column) {
        case Columns::name:
            return QString(row.meta_data.name);
        case Columns::id:
            return static_cast<int>(row.meta_data.id);
        case Columns::category:
            return static_cast<int>(row.meta_data.category);
        case Columns::access:
            return static_cast<int>(row.meta_data.read_write_access);
        case Columns::value_type:
            return static_cast<int>(row.meta_data.value_type);
        case Columns::value:
            // Booleans convert to 0/1 and numbers to themselves, which is the order both read in.
            return row.value.toDouble();
    }
    return {};
}

/// @brief Data of a cell of a group heading row.
QVariant groupData(const RowData& row, Columns column, int role) {
    switch (role) {
        case Qt::DisplayRole:
            // Every other column stays empty: a group has no ID, type or value of its own, and
            // filling them in would only invite the reader to compare them against the
            // parameters underneath.
            return (column == Columns::name) ? row.group_name : QVariant{};

        case Qt::FontRole: {
            QFont font = QApplication::font();
            font.setBold(true);
            return font;
        }

        default:
            return {};
    }
}

/// @brief Data of a cell of a parameter row.
QVariant parameterData(const RowData& row, Columns column, int role) {
    switch (role) {
        case Qt::DisplayRole:
            return parameterDisplayText(row, column);

        case Qt::EditRole:
            // The value delegate fills its editor from this, so it needs the value itself rather
            // than the text it is displayed as.
            return (column == Columns::value) ? row.value : QVariant{};

        case Qt::ForegroundRole:
            // Grey out the value cell on read-only rows so signal parameters read as "inactive".
            // Cell stays selectable (see flags()) so values can still be copied.
            if (column == Columns::value &&
                row.meta_data.read_write_access == parameter_system::ReadWriteAccess::read_only) {
                return QBrush(QApplication::palette().color(QPalette::Disabled, QPalette::Text));
            }
            return {};

        default:
            return {};
    }
}

}  // namespace

ParameterTableModel::ParameterTableModel(RowData& root, QObject* parent)
    : QAbstractItemModel(parent), root_(root) {}

RowData* ParameterTableModel::nodeFromIndex(const QModelIndex& index) {
    if (!index.isValid()) return nullptr;
    return static_cast<RowData*>(index.internalPointer());
}

QModelIndex ParameterTableModel::indexOfNode(RowData& node, Columns column) const {
    return createIndex(node.row_in_parent, static_cast<int>(column), &node);
}

QModelIndex ParameterTableModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return {};

    const RowData* parent_node = parent.isValid() ? nodeFromIndex(parent) : &root_;
    return createIndex(row, column, parent_node->children[row].get());
}

QModelIndex ParameterTableModel::parent(const QModelIndex& index) const {
    const RowData* node = nodeFromIndex(index);
    if (node == nullptr) return {};

    // The root is not a row of its own, so its children are the top level and have no parent.
    RowData* parent_node = node->parent;
    if (parent_node == nullptr || parent_node == &root_) return {};

    // A parent index always points at column 0 — that is the column the tree structure lives in.
    return createIndex(parent_node->row_in_parent, 0, parent_node);
}

int ParameterTableModel::rowCount(const QModelIndex& parent) const {
    // Only the first column carries the child rows; the others are just cells alongside it.
    if (parent.column() > 0) return 0;

    const RowData* node = parent.isValid() ? nodeFromIndex(parent) : &root_;
    if (node == nullptr) return 0;

    return static_cast<int>(node->children.size());
}

int ParameterTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    // Unlike a flat table, every row of a tree has the full set of columns, however deep it sits.
    return K_COLUMN_COUNT;
}

QVariant ParameterTableModel::data(const QModelIndex& index, int role) const {
    const RowData* row = nodeFromIndex(index);
    if (row == nullptr) return {};

    const Columns column = static_cast<Columns>(index.column());

    switch (role) {
        case k_row_kind_role:
            return static_cast<int>(row->kind);

        case k_value_type_role:
            return (row->kind == RowData::Kind::parameter) ? QVariant(static_cast<int>(row->meta_data.value_type))
                                                           : QVariant{};

        case k_filter_text_role:
            return filterText(*row, column);

        case k_sort_role:
            return sortValue(*row, column);

        default:
            break;
    }

    if (row->kind == RowData::Kind::group) return groupData(*row, column, role);
    return parameterData(*row, column, role);
}

QVariant ParameterTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::ItemDataRole::DisplayRole) return {};

    //  Only make headers for columns on top
    // Vertical would make headers for the rows but this table does not need them
    if (orientation != Qt::Horizontal) return {};

    // Names for the column headers
    switch (section) {
        case static_cast<int>(Columns::name):
            return "Name";
        case static_cast<int>(Columns::id):
            return "ID";
        case static_cast<int>(Columns::category):
            return "Category";
        case static_cast<int>(Columns::access):
            return "Access";
        case static_cast<int>(Columns::value_type):
            return "Type";
        case static_cast<int>(Columns::value):
            return "Value";
        default:
            return {};
    }
}

Qt::ItemFlags ParameterTableModel::flags(const QModelIndex& index) const {
    const RowData* row = nodeFromIndex(index);
    if (row == nullptr) return Qt::ItemFlag::NoItemFlags;

    // A group row is a heading. It can be clicked to expand and collapse, but nothing in it is
    // editable, and its empty cells are not worth selecting.
    if (row->kind == RowData::Kind::group) return Qt::ItemIsEnabled;

    // flags for the value column
    if (index.column() == static_cast<int>(Columns::value)) {
        bool parameter_is_editable = row->meta_data.read_write_access == parameter_system::ReadWriteAccess::read_write;

        if (parameter_is_editable) return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    // flags for the other columns
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool ParameterTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    RowData* row = nodeFromIndex(index);
    if (row == nullptr || row->kind != RowData::Kind::parameter) return false;
    if (role != Qt::EditRole || index.column() != static_cast<int>(Columns::value)) return false;

    row->value = value;
    Q_EMIT dataChanged(index, index);
    Q_EMIT parameterValueChanged(row->meta_data.id, value);
    return true;
}

void ParameterTableModel::updateValueFromDevice(RowData& parameter_row, const QVariant& value) {
    if (parameter_row.kind != RowData::Kind::parameter) return;

    parameter_row.value          = value;
    const QModelIndex cell_index = indexOfNode(parameter_row, Columns::value);
    Q_EMIT dataChanged(cell_index, cell_index);
}

}  // namespace parameter_table
