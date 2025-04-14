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
    ui_->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui_->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    ui_->tableView->setSortingEnabled(true);
    // tableView->setSelectionMode(QAbstractItemView::SingleSelection); // TODO add?
    // tableView->setSelectionBehavior(QAbstractItemView::SelectItems); // TODO add?

    // Connect filter input to the proxy model
    Q_UNUSED(QObject::connect(ui_->filterLineEdit, &QLineEdit::textChanged, this,
                              [&](const QString& text) { table_filter_proxy_model_.setFilterFixedString(text); }));

    // Configure refresh button and the auto refresh setting
    Q_UNUSED(QObject::connect(ui_->refreshPushButton, &QPushButton::clicked, this, &refreshParameterValues));
    ui_->automaticRefreshComboBox->addItem("No Refresh", QVariant(false));
    ui_->automaticRefreshComboBox->addItem("5 Seconds", QVariant::fromValue(std::chrono::milliseconds(5000)));
    ui_->automaticRefreshComboBox->addItem("2.5 Seconds", QVariant::fromValue(std::chrono::milliseconds(2500)));
    ui_->automaticRefreshComboBox->addItem("1 Second", QVariant::fromValue(std::chrono::milliseconds(1000)));
    ui_->automaticRefreshComboBox->addItem("500 Milliseconds", QVariant::fromValue(std::chrono::milliseconds(500)));
    ui_->automaticRefreshComboBox->addItem("250 Milliseconds", QVariant::fromValue(std::chrono::milliseconds(250)));
    ui_->automaticRefreshComboBox->addItem("50 Milliseconds", QVariant::fromValue(std::chrono::milliseconds(50)));
    ui_->automaticRefreshComboBox->setCurrentIndex(0);  // set on no automatic refresh
    Q_UNUSED(QObject::connect(&refresh_timer_, &QTimer::timeout, this, &refreshParameterValues));
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

    value_delegate_ = new ParameterValueDelegate(rows_, this);
    ui_->tableView->setItemDelegateForColumn(static_cast<int>(Columns::value), value_delegate_);
    // TODO is this necessary
    ui_->tableView->repaint();
}

const RowData& ParameterTableWidget::getRowDataByIndex(int row) { return rows_[row]; }

QVector<RowData> ParameterTableWidget::fetchParameters() const {
    QVector<RowData> parameters;
    for (uint8_t id : device_->getRegisteredParameterIds()) {
        parameter_system::ParameterMetaData meta_data = device_->getParameterMetaData(id);
        QVariant                            value     = getParameterValue(id, meta_data.type);
        parameters.push_back({meta_data, value});
    }

    return parameters;
}

void ParameterTableWidget::refreshParameterValues() {
    using parameter_system::ParameterDeclaration;
    using parameter_system::ParameterType;
    for (size_t i = 0; i < rows_.size(); i++) {
        RowData&    row              = rows_[i];
        QVariant    new_value        = getParameterValue(row.meta_data.id, row.meta_data.type);
        QModelIndex table_cell_index = table_model_->index(i, static_cast<int>(Columns::value));

        Q_UNUSED(table_model_->setData(table_cell_index, new_value, Qt::EditRole));
    }
}

QVariant ParameterTableWidget::getParameterValue(parameter_system::ParameterID   id,
                                                 parameter_system::ParameterType type) const {
    using parameter_system::ParameterDeclaration;
    using parameter_system::ParameterType;

    switch (type) {
        case ParameterType::uint8:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::uint8>{id}));
        case ParameterType::uint16:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::uint16>{id}));
        case ParameterType::uint32:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::uint32>{id}));
        case ParameterType::uint64:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::uint64>{id}));
        case ParameterType::int8:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::int8>{id}));
        case ParameterType::int16:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::int16>{id}));
        case ParameterType::int32:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::int32>{id}));
        case ParameterType::int64:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::int64>{id}));
        case ParameterType::floating_point:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterType::floating_point>{id}));
        case ParameterType::double_float:
            return QVariant::fromValue(
                device_->readParameterValue(ParameterDeclaration<ParameterType::double_float>{id}));
        case ParameterType::boolean:
            return QVariant::fromValue(device_->readParameterValue(ParameterDeclaration<ParameterType::boolean>{id}));

        case ParameterType::none:
            qDebug() << "ParameterTableWidget::getParameterValue"
                     << "unhandled type";
            break;
    }
    return {};
}

}  // namespace parameter_table
