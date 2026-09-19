#ifndef DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEWIDGET_H
#define DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEWIDGET_H

#include <QTimer>
#include <QVector>
#include <QWidget>

#include "ParameterFilterProxyModel.h"
#include "ParameterNameDelegate.h"
#include "ParameterTableModel.h"
#include "ParameterValueDelegate.h"
#include "control_api/Device.h"
#include "parameter_table/common.h"

namespace parameter_table {

QT_BEGIN_NAMESPACE
namespace Ui {
class ParameterTableWidget;
}
QT_END_NAMESPACE

/**
 * @class ParameterTableWidget
 * @brief A widget for displaying and managing parameters, grouped into parameter groups.
 *
 * Parameters are shown as one tree under a single shared header: groups are expandable rows and
 * the parameters in them are indented underneath. One header keeps every column the same width
 * from top to bottom, so values stay aligned across groups, and it keeps sorting by any column
 * working across all of the parameters at once.
 *
 * A device that reports no grouping puts every parameter under the implicit root, which renders
 * as the flat table this widget used to be.
 */
class ParameterTableWidget final : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief Constructs a ParameterTableWidget.
     * @param parent The parent widget (optional).
     */
    explicit ParameterTableWidget(QWidget* parent = nullptr);
    /**
     * @brief Destroys the ParameterTableWidget.
     */
    ~ParameterTableWidget() override;

    /**
     * @brief Initializes the widget with a device reference.
     * @param device The device whose parameters will be displayed. May be nullptr, in which case
     *               the widget shows mock parameters and never talks to a device.
     */
    void initialize(servo_core_control_api::Device* device);

private:
    Ui::ParameterTableWidget* ui_;  ///< UI pointer for the widget.

    servo_core_control_api::Device* device_ = nullptr;  ///< Non-owning reference to the device.

    ParameterTableModel*      table_model_    = nullptr;  ///< Model for constructing the tree.
    ParameterValueDelegate*   value_delegate_ = nullptr;  ///< Delegate for handling value editing.
    ParameterNameDelegate*    name_delegate_  = nullptr;  ///< Delegate for drawing the name column.
    ParameterFilterProxyModel table_filter_proxy_model_;  ///< Proxy model for filtering and sorting.
    QTimer                    refresh_timer_;             ///< Timer for automatic parameter refresh.

    RowData root_;  ///< Root of the row tree. Never shown; holds the top level rows.
    /// Every parameter row of the tree, flattened. Refreshing walks this instead of the tree,
    /// since a refresh does not care which group a parameter sits in.
    QVector<RowData*> parameter_rows_;

    void     buildParameterTree();
    void     collectParameterRows(RowData& node);
    void     applyFilter(const QString& text);
    void     updateStatusLabel();
    void     refreshSignalParameterValues();
    void     refreshAllParameterValues();
    QVariant getParameterValue(parameter_system::ParameterID id, parameter_system::ParameterValueType type) const;
    void     writeParameterValue(parameter_system::ParameterID id, const QVariant& value);
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEWIDGET_H
