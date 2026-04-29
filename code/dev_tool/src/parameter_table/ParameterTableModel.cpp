#include "parameter_table/ParameterTableModel.h"

#include <QApplication>
#include <QBrush>
#include <QPalette>

#include "helpers.h"
#include "parameter_system/common.h"

namespace parameter_table {

ParameterTableModel::ParameterTableModel(QVector<RowData>& rows, QObject* parent)
    : QAbstractTableModel(parent), rows_(rows) {}

int ParameterTableModel::rowCount(const QModelIndex& parent) const {
    // In flat table models, parent is always invalid; if it's valid, it means asking for child rows (tree structure).
    // Since parameter table does not support tree structures, return 0 if it is parent is valid
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

int ParameterTableModel::columnCount(const QModelIndex& parent) const {
    // In flat table models, parent is always invalid; if it's valid, it means asking for child rows (tree structure).
    // Since parameter table does not support tree structures, return 0 if it is parent is valid
    return (parent.isValid()) ? 0 : K_COLUMN_COUNT;
}

QVariant ParameterTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rows_.size()) return {};

    const RowData& row = rows_[index.row()];
    if (role == Qt::ItemDataRole::DisplayRole) {
        switch (index.column()) {
            case static_cast<int>(Columns::id):
                return helpers::intToHexString(row.meta_data.id);
            case static_cast<int>(Columns::name):
                return row.meta_data.name;
            case static_cast<int>(Columns::category):
                return parameter_system::mapParameterCategoryToString(row.meta_data.category);
            case static_cast<int>(Columns::access):
                return parameter_system::mapReadWriteAccessToString(row.meta_data.read_write_access);
            case static_cast<int>(Columns::value_type):
                return parameter_system::mapParameterValueTypeToString(row.meta_data.value_type);
            case static_cast<int>(Columns::value):
                return row.value;
        }
    }

    // Grey out the value cell on read-only rows so signal parameters read as "inactive".
    // Cell stays selectable (see flags()) so values can still be copied.
    if (role == Qt::ForegroundRole && index.column() == static_cast<int>(Columns::value) &&
        row.meta_data.read_write_access == parameter_system::ReadWriteAccess::read_only) {
        return QBrush(QApplication::palette().color(QPalette::Disabled, QPalette::Text));
    }

    return {};
}
QVariant ParameterTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::ItemDataRole::DisplayRole) return {};

    //  Only make headers for columns on top
    // Vertical would make headers for the rows but this table does not need them
    if (orientation != Qt::Horizontal) return {};

    // Names for the column headers
    switch (section) {
        case static_cast<int>(Columns::id):
            return "ID";
        case static_cast<int>(Columns::name):
            return "Name";
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
    if (!index.isValid()) return Qt::ItemFlag::NoItemFlags;

    // flags for the value column
    if (index.column() == static_cast<size_t>(Columns::value)) {
        RowData row                = rows_[index.row()];
        bool parameter_is_editable = row.meta_data.read_write_access == parameter_system::ReadWriteAccess::read_write;

        if (parameter_is_editable) return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    // flags for the other columns
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool ParameterTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.isValid() && role == Qt::EditRole && index.column() == static_cast<int>(Columns::value)) {
        rows_[index.row()].value = value;
        Q_EMIT dataChanged(index, index);
        Q_EMIT parameterValueChanged(rows_[index.row()].meta_data.id, value);
        return true;
    }
    return false;
}

void ParameterTableModel::updateValueFromDevice(int row_index, const QVariant& value) {
    if (row_index < 0 || row_index >= rows_.size()) return;
    rows_[row_index].value      = value;
    const QModelIndex cell_index = index(row_index, static_cast<int>(Columns::value));
    Q_EMIT dataChanged(cell_index, cell_index);
}
void ParameterTableModel::sort(int column_index, Qt::SortOrder order) {
    Columns column = static_cast<Columns>(column_index);

    auto compare   = [order](auto&& left, auto&& right) {
        return (order == Qt::AscendingOrder) ? left < right : left > right;
    };

    switch (column) {
        case Columns::id:
            std::sort(rows_.begin(), rows_.end(), [compare](const RowData& left, const RowData& right) {
                return compare(left.meta_data.id, right.meta_data.id);
            });
            break;
        case Columns::name:
            std::sort(rows_.begin(), rows_.end(), [compare](const RowData& left, const RowData& right) {
                return compare(QString(left.meta_data.name), QString(right.meta_data.name));
            });
            break;
        case Columns::access:
            std::sort(rows_.begin(), rows_.end(), [compare](const RowData& left, const RowData& right) {
                return compare(left.meta_data.read_write_access, right.meta_data.read_write_access);
            });
            break;
        case Columns::category:
            std::sort(rows_.begin(), rows_.end(), [compare](const RowData& left, const RowData& right) {
                return compare(left.meta_data.category, right.meta_data.category);
            });
            break;
        case Columns::value_type:
            std::sort(rows_.begin(), rows_.end(), [compare](const RowData& left, const RowData& right) {
                return compare(left.meta_data.value_type, right.meta_data.value_type);
            });
            break;
        case Columns::value:
            std::sort(rows_.begin(), rows_.end(), [compare](const RowData& left, const RowData& right) {
                return compare(left.value.toDouble(), right.value.toString().toDouble());
            });
            break;
    }

    // Tell that the table has changed and need to be re-rendered
    emit dataChanged(index(0, 0), index(static_cast<int>(rows_.size()) - 1, K_COLUMN_COUNT - 1));
}

}  // namespace parameter_table