#include "parameter_table/ParameterTableModel.h"

#include "helpers.h"
#include "parameter_system/definitions.h"

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
            case static_cast<int>(Columns::access):
                return parameter_system::mapReadWriteAccessToString(row.meta_data.read_write_access);
            case static_cast<int>(Columns::type):
                return parameter_system::mapParameterTypeToString(row.meta_data.type);
            case static_cast<int>(Columns::value):
                // Check if parameter is write-only, if so return empty QVariant
                if (row.meta_data.read_write_access == parameter_system::ReadWriteAccess::write_only) {
                    return {};  // Hide value
                }
                return row.value;
        }
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
        case static_cast<int>(Columns::access):
            return "Access";
        case static_cast<int>(Columns::type):
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
        bool parameter_is_editable = row.meta_data.read_write_access == parameter_system::ReadWriteAccess::read_write ||
                                     row.meta_data.read_write_access == parameter_system::ReadWriteAccess::write_only;

        if (parameter_is_editable) return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        // if not editable default to these flags
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
        case Columns::type:
            std::sort(rows_.begin(), rows_.end(), [compare](const RowData& left, const RowData& right) {
                return compare(left.meta_data.type, right.meta_data.type);
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