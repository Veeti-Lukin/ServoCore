#ifndef DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEMODEL_H
#define DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEMODEL_H

#include <QAbstractItemModel>

#include "common.h"
#include "parameter_system/common.h"

namespace parameter_table {

/**
 * @class ParameterTableModel
 * @brief Tree model over the parameters of a device, grouped into parameter groups.
 *
 * This class extends QAbstractItemModel, the base class for models whose rows form a tree.
 * It provides an interface for retrieving and modifying parameters that are displayed in a
 * QTreeView.
 *
 * The model allows:
 * - Displaying parameters with multiple attributes (Name, ID, Category, Access, Type, Value).
 * - Nesting parameters under group rows, to any depth.
 * - Editing parameter values if permitted.
 *
 * Rows live in a RowData tree owned by the caller; the model only reads and writes through the
 * reference it is given. Sorting and filtering are not implemented here — they belong to the
 * proxy model in front of this one, which is why this class exposes the underlying values
 * through the k_sort_role and k_filter_text_role roles.
 */
class ParameterTableModel final : public QAbstractItemModel {
    Q_OBJECT

signals:

    /**
     * @brief Emitted when a parameter value is changed by the user.
     *
     * Used to notify other components that a parameter value has been modified.
     *
     * @param id The unique identifier of the parameter that was changed.
     * @param value The new value assigned to the parameter.
     */
    void parameterValueChanged(parameter_system::ParameterID id, const QVariant& value);

public:
    /**
     * @brief Constructs a ParameterTableModel over an existing row tree.
     *
     * @param root The root node of the tree. It is never shown itself; its children become the
     *             top level rows. The tree has to outlive the model.
     * @param parent The parent QObject (optional).
     */
    explicit ParameterTableModel(RowData& root, QObject* parent = nullptr);

    /**
     * @brief Creates the index of a child row.
     *
     * @param row The position of the row under @p parent.
     * @param column The column of the cell.
     * @param parent The parent row, or an invalid index for the top level rows.
     * @return The index of the cell, or an invalid index if there is no such cell.
     */
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    /**
     * @brief Returns the index of the row that @p index sits under.
     *
     * @param index The index whose parent is asked for.
     * @return The parent row's index, or an invalid index for a top level row.
     */
    [[nodiscard]] QModelIndex parent(const QModelIndex& index) const override;

    /**
     * @brief Returns the number of rows directly underneath a row.
     *
     * @param parent The parent row, or an invalid index to count the top level rows.
     * @return The number of child rows.
     */
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;

    /**
     * @brief Returns the number of columns.
     *
     * Every row has the same columns, at every level of the tree.
     *
     * @param parent The parent index (ignored).
     * @return The number of columns.
     */
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;

    /**
     * @brief Retrieves the data for a given cell.
     *
     * @param index The model index representing the requested cell.
     * @param role The role that specifies the type of data to retrieve (e.g., display text).
     * @return The requested data or an empty QVariant if the index is invalid.
     */
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    /**
     * @brief Retrieves the header data for the table.
     *
     * @param section The index of the row or column header.
     * @param orientation Specifies whether it is a row or column header.
     * @param role The role that specifies the type of data to retrieve.
     * @return The header label text or an empty QVariant if the role is not DisplayRole.
     */
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * @brief Returns the interaction flags for a given cell.
     *
     * Defines whether a cell is selectable, editable, or disabled.
     *
     * @param index The index of the cell.
     * @return The item flags that determine how the cell can be interacted with.
     */
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * @brief Updates the value of a cell in the table.
     *
     * If the parameter is editable, the value will be updated and relevant signals emitted.
     *
     * @param index The index of the cell.
     * @param value The new value to set.
     * @param role The role specifying the type of data being set.
     * @return True if the update was successful, false otherwise.
     */
    [[nodiscard]] bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    /**
     * @brief Programmatic value update from a device read — does NOT emit parameterValueChanged.
     *
     * Used by refresh paths that just want to push the latest device value into the cell
     * without triggering a write-back round-trip.
     *
     * @param parameter_row The row whose value cell should be updated.
     * @param value         New display value.
     */
    void updateValueFromDevice(RowData& parameter_row, const QVariant& value);

    /**
     * @brief Returns the row node an index of this model points at.
     *
     * Only valid for indexes of this model. An index taken from a proxy model has to be mapped
     * to the source first.
     *
     * @param index The index to resolve.
     * @return The node, or nullptr for an invalid index.
     */
    [[nodiscard]] static RowData* nodeFromIndex(const QModelIndex& index);

    /**
     * @brief Returns the index of a cell of a node.
     *
     * @param node The node the cell belongs to. Has to be part of this model's tree.
     * @param column The column of the cell.
     * @return The index of the cell.
     */
    [[nodiscard]] QModelIndex indexOfNode(RowData& node, Columns column) const;

private:
    RowData& root_;  ///< Root of the row tree. Never shown; only its descendants are.
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEMODEL_H
