#ifndef DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEWIDGET_H
#define DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEWIDGET_H

#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>
#include <QVector>
#include <QWidget>

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
 * @brief A widget for displaying and managing parameters in a tabular format.
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
     * @param device The device whose parameters will be displayed.
     */
    void initialize(servo_core_control_api::Device& device);

    /**
     * @brief Retrieves the row data for a given index.
     * @param row The row index.
     * @return A reference to the corresponding RowData.
     */
    const RowData& getRowDataByIndex(int row);

private:
    Ui::ParameterTableWidget* ui_;  ///< UI pointer for the widget.

    servo_core_control_api::Device* device_;  ///< Non-owning reference to the device.

    ParameterTableModel*    table_model_    = nullptr;  ///< Model for constructing the table.
    ParameterValueDelegate* value_delegate_ = nullptr;  ///< Delegate for handling value editing.
    QSortFilterProxyModel   table_filter_proxy_model_;  ///< Proxy model for filtering table data.
    QTimer                  refresh_timer_;             ///< Timer for automatic parameter refresh.
    QVector<RowData>        rows_;                      ///< Storage for table parameter rows.

    [[nodiscard]] QVector<RowData> fetchParameters() const;
    void                           refreshParameterValues();
    QVariant getParameterValue(parameter_system::ParameterID id, parameter_system::ParameterType type) const;
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_PARAMETERTABLEWIDGET_H
