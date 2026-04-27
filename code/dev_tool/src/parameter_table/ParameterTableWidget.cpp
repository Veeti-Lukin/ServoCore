// You may need to build the project (run Qt uic code generator) to get "ui_ParameterTableWidget.h" resolved

#include "parameter_table/ParameterTableWidget.h"

#include <chrono>

#include "parameter_system/ParameterDeclaration.h"
#include "ui_ParameterTableWidget.h"

namespace parameter_table {

ParameterTableWidget::ParameterTableWidget(QWidget* parent) : QWidget(parent), ui_(new Ui::ParameterTableWidget) {
    ui_->setupUi(this);

    // Configure filter proxy model
    table_filter_proxy_model_.setFilterCaseSensitivity(Qt::CaseInsensitive);
    table_filter_proxy_model_.setFilterKeyColumn(-1);  // Use the filter to search from all the columns

    // Configure the table ui
    ui_->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui_->tableView->horizontalHeader()->setStretchLastSection(true);  // table still fills full width
    ui_->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    ui_->tableView->setSortingEnabled(true);
    // tableView->setSelectionMode(QAbstractItemView::SingleSelection); // TODO add?
    // tableView->setSelectionBehavior(QAbstractItemView::SelectItems); // TODO add?

    // Connect filter input to the proxy model
    Q_UNUSED(QObject::connect(ui_->filterLineEdit, &QLineEdit::textChanged, this,
                              [&](const QString& text) { table_filter_proxy_model_.setFilterFixedString(text); }));

    // Configure refresh buttons and the auto refresh setting.
    // Default refresh (button + timer) only re-reads Signal parameters; Saved/Runtime are written by
    // this dev tool and don't change without our knowledge. The "Force Refresh All" button re-reads
    // everything for sanity checks (e.g., another tool wrote, reconnect, etc.).
    Q_UNUSED(QObject::connect(ui_->refreshPushButton, &QPushButton::clicked, this, &refreshSignalParameterValues));
    Q_UNUSED(QObject::connect(ui_->forceRefreshAllPushButton, &QPushButton::clicked, this, &refreshAllParameterValues));
    ui_->automaticRefreshComboBox->addItem("No Automatic Refresh", QVariant(false));
    ui_->automaticRefreshComboBox->addItem("Auto Refresh Every 5 Seconds",
                                           QVariant::fromValue(std::chrono::milliseconds(5000)));
    ui_->automaticRefreshComboBox->addItem("Auto Refresh Every 2.5 Seconds",
                                           QVariant::fromValue(std::chrono::milliseconds(2500)));
    ui_->automaticRefreshComboBox->addItem("Auto Refresh Every 1 Second",
                                           QVariant::fromValue(std::chrono::milliseconds(1000)));
    ui_->automaticRefreshComboBox->addItem("Auto Refresh Every 500 Milliseconds",
                                           QVariant::fromValue(std::chrono::milliseconds(500)));
    ui_->automaticRefreshComboBox->addItem("Auto Refresh Every 250 Milliseconds",
                                           QVariant::fromValue(std::chrono::milliseconds(250)));
    ui_->automaticRefreshComboBox->addItem("Auto Refresh Every 50 Milliseconds",
                                           QVariant::fromValue(std::chrono::milliseconds(50)));
    ui_->automaticRefreshComboBox->setCurrentIndex(0);  // set on no automatic refresh
    Q_UNUSED(QObject::connect(&refresh_timer_, &QTimer::timeout, this, &refreshSignalParameterValues));
    Q_UNUSED(QObject::connect(ui_->automaticRefreshComboBox, &QComboBox::currentIndexChanged, this, [&] {
        QVariant value = ui_->automaticRefreshComboBox->currentData();
        if (value == QVariant(false)) {
            refresh_timer_.stop();
            return;
        }

        std::chrono::milliseconds interval = value.value<std::chrono::milliseconds>();
        refresh_timer_.setInterval(interval);
        refresh_timer_.start();
    }));
}

ParameterTableWidget::~ParameterTableWidget() { delete ui_; }

void ParameterTableWidget::initialize(servo_core_control_api::Device& device) {
    /** ONLY DO THE INITIALIZATION HERE THAT CANNOT BE DONE BEFORE HAVING THE HANDLE TO THE DEVICE **/
    device_      = &device;
    rows_        = fetchParameters();

    table_model_ = new ParameterTableModel(rows_, this);
    // The model is applied through the filter proxy model to provide ability to filter what rows are visible or not
    table_filter_proxy_model_.setSourceModel(table_model_);
    ui_->tableView->setModel(&table_filter_proxy_model_);

    // When the user edits a cell, push the new value to the device.
    Q_UNUSED(QObject::connect(table_model_, &ParameterTableModel::parameterValueChanged, this,
                              &ParameterTableWidget::writeParameterValue));

    value_delegate_ = new ParameterValueDelegate(rows_, this);
    ui_->tableView->setItemDelegateForColumn(static_cast<int>(Columns::value), value_delegate_);
    // TODO is this necessary
    ui_->tableView->repaint();
}

const RowData& ParameterTableWidget::getRowDataByIndex(int row) { return rows_[row]; }

QVector<RowData> ParameterTableWidget::fetchParameters() const {
    QVector<RowData> parameters;
    for (uint8_t id : device_->fetchRegisteredParamIds()) {
        parameter_system::ParameterMetaData meta_data = device_->fetchParameterMetaData(id);
        QVariant                            value     = getParameterValue(id, meta_data.value_type);
        parameters.push_back({meta_data, value});
    }

    return parameters;
}

void ParameterTableWidget::refreshSignalParameterValues() {
    // Only Signal parameters can change without our knowledge — the device sets them autonomously.
    // Saved and Runtime parameters are written by the master (this dev tool) and stay put until we
    // write them again, so re-reading them every refresh just wastes bus bandwidth.
    for (size_t i = 0; i < rows_.size(); i++) {
        RowData& row = rows_[i];
        if (row.meta_data.type != parameter_system::ParameterType::signal) continue;

        QVariant    new_value        = getParameterValue(row.meta_data.id, row.meta_data.value_type);
        QModelIndex table_cell_index = table_model_->index(i, static_cast<int>(Columns::value));
        Q_UNUSED(table_model_->setData(table_cell_index, new_value, Qt::EditRole));
    }
}

void ParameterTableWidget::refreshAllParameterValues() {
    // Re-reads every parameter regardless of category. Used as a manual sanity check (e.g., another
    // tool may have written, after reconnect, debugging) — not used by the auto-refresh timer.
    for (size_t i = 0; i < rows_.size(); i++) {
        RowData&    row              = rows_[i];
        QVariant    new_value        = getParameterValue(row.meta_data.id, row.meta_data.value_type);
        QModelIndex table_cell_index = table_model_->index(i, static_cast<int>(Columns::value));
        Q_UNUSED(table_model_->setData(table_cell_index, new_value, Qt::EditRole));
    }
}

QVariant ParameterTableWidget::getParameterValue(parameter_system::ParameterID        id,
                                                 parameter_system::ParameterValueType type) const {
    using parameter_system::ParameterDeclaration;
    using parameter_system::ParameterValueType;

    switch (type) {
        case ParameterValueType::uint8:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::uint8>{id}));
        case ParameterValueType::uint16:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::uint16>{id}));
        case ParameterValueType::uint32:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::uint32>{id}));
        case ParameterValueType::uint64:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::uint64>{id}));
        case ParameterValueType::int8:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterValueType::int8>{id}));
        case ParameterValueType::int16:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::int16>{id}));
        case ParameterValueType::int32:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::int32>{id}));
        case ParameterValueType::int64:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::int64>{id}));
        case ParameterValueType::floating_point:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::floating_point>{id}));
        case ParameterValueType::double_float:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::double_float>{id}));
        case ParameterValueType::boolean:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::boolean>{id}));

        case ParameterValueType::none:
            qDebug() << "ParameterTableWidget::getParameterValue"
                     << "unhandled type";
            break;
    }
    return {};
}

void ParameterTableWidget::writeParameterValue(parameter_system::ParameterID id, const QVariant& value) {
    using parameter_system::ParameterDeclaration;
    using parameter_system::ParameterValueType;
    using ResponseCode = serial_communication_framework::ResponseCode;

    // Find the row to look up the value type — the signal only carries id + value.
    const RowData* row = nullptr;
    for (const RowData& candidate : rows_) {
        if (candidate.meta_data.id == id) {
            row = &candidate;
            break;
        }
    }
    if (row == nullptr) {
        qDebug() << "ParameterTableWidget::writeParameterValue: unknown parameter id" << id;
        return;
    }

    ResponseCode result = ResponseCode::ok;
    switch (row->meta_data.value_type) {
        case ParameterValueType::uint8:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::uint8>{id},
                                                  value.value<uint8_t>());
            break;
        case ParameterValueType::uint16:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::uint16>{id},
                                                  value.value<uint16_t>());
            break;
        case ParameterValueType::uint32:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::uint32>{id},
                                                  value.value<uint32_t>());
            break;
        case ParameterValueType::uint64:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::uint64>{id},
                                                  value.value<uint64_t>());
            break;
        case ParameterValueType::int8:
            result =
                device_->writeParameterValue(ParameterDeclaration<ParameterValueType::int8>{id}, value.value<int8_t>());
            break;
        case ParameterValueType::int16:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::int16>{id},
                                                  value.value<int16_t>());
            break;
        case ParameterValueType::int32:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::int32>{id},
                                                  value.value<int32_t>());
            break;
        case ParameterValueType::int64:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::int64>{id},
                                                  value.value<int64_t>());
            break;
        case ParameterValueType::floating_point:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::floating_point>{id},
                                                  value.value<float>());
            break;
        case ParameterValueType::double_float:
            result = device_->writeParameterValue(ParameterDeclaration<ParameterValueType::double_float>{id},
                                                  value.value<double>());
            break;
        case ParameterValueType::boolean:
            result =
                device_->writeParameterValue(ParameterDeclaration<ParameterValueType::boolean>{id}, value.toBool());
            break;
        case ParameterValueType::none:
            qDebug() << "ParameterTableWidget::writeParameterValue: unhandled type 'none' for id" << id;
            return;
    }

    if (result != ResponseCode::ok) {
        qDebug() << "ParameterTableWidget::writeParameterValue: device returned error code" << static_cast<int>(result)
                 << "for id" << id;
        // TODO: revert the cell to the previous value, or surface an error to the user.
    }
}

}  // namespace parameter_table
