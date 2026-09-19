#ifndef DEV_TOOL_PARAMETER_TABLE_PARAMETERFILTERPROXYMODEL_H
#define DEV_TOOL_PARAMETER_TABLE_PARAMETERFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QString>

namespace parameter_table {

/**
 * @class ParameterFilterProxyModel
 * @brief Sorts and filters the parameter tree in front of ParameterTableModel.
 *
 * Sorting and filtering live here rather than in the model so that the model never has to
 * reorder its own rows. That keeps the node addresses it hands out through
 * QModelIndex::internalPointer() valid, and it is what QSortFilterProxyModel is for.
 *
 * Filtering a tree needs more than matching each row on its own, so filterAcceptsRow() is
 * overridden rather than configured:
 * - a group stays visible while anything underneath it matches, so matches are never hidden
 *   inside a collapsed group;
 * - everything under a group whose name matches stays visible, so searching for a group shows
 *   its contents rather than an empty heading;
 * - a group with nothing matching disappears entirely.
 */
class ParameterFilterProxyModel final : public QSortFilterProxyModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs the proxy model.
     * @param parent The parent QObject (optional).
     */
    explicit ParameterFilterProxyModel(QObject* parent = nullptr);

    /**
     * @brief Sets the text rows are filtered by. An empty text shows everything.
     * @param text The substring to look for, matched case insensitively.
     */
    void setFilterText(const QString& text);

    /// @brief The text currently being filtered by.
    [[nodiscard]] const QString& filterText() const { return filter_text_; }

    /// @brief Number of parameter rows that survive the filter, group rows not counted.
    [[nodiscard]] int visibleParameterCount() const;

protected:
    /**
     * @brief Decides whether a row of the source model is shown.
     * @param source_row The row under @p source_parent.
     * @param source_parent The parent row in the source model.
     * @return True if the row itself, one of its descendants, or one of its ancestors matches.
     */
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    /**
     * @brief Orders two sibling rows against each other.
     * @param left The row on the left of the comparison.
     * @param right The row on the right of the comparison.
     * @return True if @p left belongs before @p right.
     */
    [[nodiscard]] bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QString filter_text_;

    /// @brief True if any cell of this row contains the filter text.
    [[nodiscard]] bool rowMatches(const QModelIndex& source_index) const;
    /// @brief True if any row underneath this one matches, however deeply nested.
    [[nodiscard]] bool anyDescendantMatches(const QModelIndex& source_index) const;
    /// @brief True if this row sits underneath a group whose own heading matches.
    [[nodiscard]] bool anyAncestorMatches(const QModelIndex& source_parent) const;
    /// @brief Counts the parameter rows under a proxy index, recursing into the groups.
    [[nodiscard]] int countParameters(const QModelIndex& parent) const;
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_PARAMETERFILTERPROXYMODEL_H
