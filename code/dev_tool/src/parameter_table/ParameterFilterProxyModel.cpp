#include "parameter_table/ParameterFilterProxyModel.h"

#include "parameter_table/common.h"

namespace parameter_table {

ParameterFilterProxyModel::ParameterFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
    setSortRole(k_sort_role);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void ParameterFilterProxyModel::setFilterText(const QString& text) {
    if (filter_text_ == text) return;

    // Only rows are filtered, so the proxy is told to re-evaluate nothing else.
    beginFilterChange();
    filter_text_ = text;
    endFilterChange(Direction::Rows);
}

bool ParameterFilterProxyModel::rowMatches(const QModelIndex& source_index) const {
    const QAbstractItemModel* model = sourceModel();
    if (model == nullptr) return false;

    for (int column = 0; column < model->columnCount(source_index.parent()); column++) {
        const QModelIndex cell = model->index(source_index.row(), column, source_index.parent());
        const QString     text = cell.data(k_filter_text_role).toString();

        // An empty filter text would match every row, including the cells that deliberately
        // expose nothing to the filter.
        if (text.isEmpty()) continue;
        if (text.contains(filter_text_, Qt::CaseInsensitive)) return true;
    }

    return false;
}

bool ParameterFilterProxyModel::anyDescendantMatches(const QModelIndex& source_index) const {
    const QAbstractItemModel* model = sourceModel();
    if (model == nullptr) return false;

    const int child_count = model->rowCount(source_index);
    for (int row = 0; row < child_count; row++) {
        const QModelIndex child = model->index(row, 0, source_index);
        if (rowMatches(child)) return true;
        if (anyDescendantMatches(child)) return true;
    }

    return false;
}

bool ParameterFilterProxyModel::anyAncestorMatches(const QModelIndex& source_parent) const {
    for (QModelIndex ancestor = source_parent; ancestor.isValid(); ancestor = ancestor.parent()) {
        if (rowMatches(ancestor)) return true;
    }
    return false;
}

bool ParameterFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    if (filter_text_.isEmpty()) return true;

    const QAbstractItemModel* model = sourceModel();
    if (model == nullptr) return false;

    const QModelIndex source_index = model->index(source_row, 0, source_parent);
    if (!source_index.isValid()) return false;

    // The row itself is what the user was looking for.
    if (rowMatches(source_index)) return true;

    // A group whose members matched. Keeping it visible is what lets the matches be shown at all.
    if (anyDescendantMatches(source_index)) return true;

    // A member of a group whose heading matched. Searching for a group name should show the
    // parameters in it, not just the heading.
    return anyAncestorMatches(source_parent);
}

bool ParameterFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    const int left_kind  = left.data(k_row_kind_role).toInt();
    const int right_kind = right.data(k_row_kind_role).toInt();

    if (left_kind != right_kind) {
        // Groups are headings, so they belong above the loose parameters they sit next to,
        // whichever way the column is sorted. Qt flips whatever this returns when sorting
        // descending, so the answer has to be flipped back to survive it.
        const bool left_is_group = left_kind == static_cast<int>(RowData::Kind::group);
        return (sortOrder() == Qt::AscendingOrder) ? left_is_group : !left_is_group;
    }

    const QVariant left_value  = left.data(k_sort_role);
    const QVariant right_value = right.data(k_sort_role);

    // Names, and the group headings that sort as names, read better in the order the user's
    // locale puts them in than in the order their code points happen to fall.
    if (left_value.typeId() == QMetaType::QString && right_value.typeId() == QMetaType::QString) {
        return QString::localeAwareCompare(left_value.toString(), right_value.toString()) < 0;
    }

    return QVariant::compare(left_value, right_value) == QPartialOrdering::Less;
}

int ParameterFilterProxyModel::countParameters(const QModelIndex& parent) const {
    int count = 0;

    const int child_count = rowCount(parent);
    for (int row = 0; row < child_count; row++) {
        const QModelIndex child = index(row, 0, parent);
        if (child.data(k_row_kind_role).toInt() == static_cast<int>(RowData::Kind::parameter)) count++;
        count += countParameters(child);
    }

    return count;
}

int ParameterFilterProxyModel::visibleParameterCount() const { return countParameters({}); }

}  // namespace parameter_table
