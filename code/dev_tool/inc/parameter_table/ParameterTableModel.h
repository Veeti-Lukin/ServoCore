#ifndef DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEMODEL_H
#define DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

#include "common.h"
#include "parameter_system/definitions.h"

namespace parameter_table {

/**
 * @class ParameterTableModel
 * @brief Represents a table model for displaying and managing parameter data.
 *
 * This class extends QAbstractTableModel, which is a base class for creating table-like data models.
 * It provides an interface for retrieving and modifying parameters that are displayed in a table view.
 *
 * The model allows:
 * - Displaying parameters with multiple attributes (ID, Name, Access, Type, Value).
 * - Sorting data based on a selected column.
 * - Editing parameter values if permitted.
 *
 * This model is used with a QTableView to provide a structured, interactive display of parameters.
 */
class ParameterTableModel final : public QAbstractTableModel {
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
     * @brief Constructs a ParameterTableModel with a reference to the parameter data.
     *
     * @param rows A reference to the vector containing parameter data.
     * @param parent The parent QObject (optional).
     */
    explicit ParameterTableModel(QVector<RowData>& rows, QObject* parent = nullptr);

    /**
     * @brief Returns the number of rows in the table.
     *
     * @param parent The parent index (ignored since this is a flat table model).
     * @return The number of rows in the table.
     */
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;

    /**
     * @brief Returns the number of columns in the table.
     *
     * @param parent The parent index (ignored since this is a flat table model).
     * @return The number of columns.
     */
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;

    /**
     * @brief Retrieves the data for a given cell in the table.
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
     * @brief Sorts the table based on the specified column and order.
     *
     * @param column_index The column index to sort by.
     * @param order The sorting order (ascending or descending).
     */
    void sort(int column_index, Qt::SortOrder order) override;

private:
    QVector<RowData>& rows_;  ///< A reference to the list of parameter data rows.
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEMODEL_H
