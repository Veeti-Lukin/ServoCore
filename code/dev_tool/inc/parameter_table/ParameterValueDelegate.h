#ifndef DEV_TOOL_PARAMETER_TABLE_PARAMETERVALUEDELEGATE_H
#define DEV_TOOL_PARAMETER_TABLE_PARAMETERVALUEDELEGATE_H

#include <QStyledItemDelegate>

#include "common.h"
#include "parameter_system/parameter_definitions.h"

namespace parameter_table {

/**
 * @class ParameterValueDelegate
 * @brief A delegate for handling parameter value editing in a parameter table.
 *
 * This class provides custom editors for different parameter types,
 * ensuring that the correct input widgets (e.g., spin boxes, combo boxes)
 * are used when editing parameters in the table. It inherits from
 * QStyledItemDelegate to override the default behavior of item editors.
 */
class ParameterValueDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    /**
     * @brief Constructs a ParameterValueDelegate.
     * @param rows Reference to a vector of RowData objects containing parameter data.
     * @param parent Optional parent object.
     */
    explicit ParameterValueDelegate(QVector<RowData>& rows, QObject* parent = nullptr);

    /**
     * @brief Creates an editor widget for a given index.
     *
     * This method dynamically creates an appropriate editor widget
     * (such as QDoubleSpinBox or QComboBox) based on the parameter type.
     *
     * The parent widget claims the ownership of the dynamically created editor
     *
     * @param parent The parent widget.
     * @param option Style options for the editor.
     * @param index The model index of the item being edited.
     * @return A pointer to the created editor widget.
     */
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /**
     * @brief Sets the editor data from the model.
     *
     * Populates the editor with the current value from the model,
     * ensuring that the user sees the correct initial value.
     *
     * @param editor The editor widget.
     * @param index The model index of the item being edited.
     */
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    /**
     * @brief Saves the data from the editor to the model.
     *
     * Extracts the user-inputted value from the editor widget and
     * updates the model accordingly.
     *
     * @param editor The editor widget.
     * @param model The model storing the table data.
     * @param index The model index of the item being edited.
     */
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    /**
     * @brief Updates the geometry of the editor widget.
     *
     * Ensures the editor widget is correctly positioned and sized
     * within the table cell.
     *
     * @param editor The editor widget.
     * @param option The style options containing geometry details.
     * @param index The model index of the item being edited.
     */
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

private:
    QVector<RowData>& rows_;  ///< Reference to the vector of RowData containing parameter values.
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_PARAMETERVALUEDELEGATE_H
