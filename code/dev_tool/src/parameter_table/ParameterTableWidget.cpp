// You may need to build the project (run Qt uic code generator) to get "ui_ParameterTableWidget.h" resolved

#include "parameter_table/ParameterTableWidget.h"

#include <chrono>
#include <cstring>

#include <QHeaderView>
#include <QRandomGenerator>

#include "parameter_system/ParameterDeclaration.h"
#include "ui_ParameterTableWidget.h"

namespace parameter_table {
namespace {

/// @brief Fills in a ParameterMetaData, copying the name into its fixed size buffer.
parameter_system::ParameterMetaData makeMetaData(parameter_system::ParameterID id, const char* name,
                                                 parameter_system::ParameterCategory  category,
                                                 parameter_system::ParameterValueType value_type,
                                                 parameter_system::ReadWriteAccess     access) {
    parameter_system::ParameterMetaData meta_data = {};
    meta_data.id                                  = id;
    meta_data.category                            = category;
    meta_data.value_type                          = value_type;
    meta_data.read_write_access                   = access;

    std::strncpy(meta_data.name, name, parameter_system::ParameterMetaData::K_PARAMETER_NAME_MAX_LENGTH - 1);
    return meta_data;
}

}  // namespace

// ----------------------------------------------------------------------------------------------
// --- MOCK PARAMETERS (issue #3, stage 1) ---
//
// The grouping is being built in the dev tool first, so that the look can be settled before the
// firmware learns to report which group a parameter belongs to. Until then there is nothing real
// to read: this hardcoded tree stands in for a device, and buildParameterTree() below uses it
// instead of talking over the bus.
//
// To go back to a real device: delete this block, and restore the commented out fetch in
// buildParameterTree().
// ----------------------------------------------------------------------------------------------
namespace {

using parameter_system::ParameterCategory;
using parameter_system::ParameterValueType;
using parameter_system::ReadWriteAccess;

/// @brief Appends a mock parameter row under @p parent.
void addMockParameter(RowData& parent, parameter_system::ParameterID id, const char* name, ParameterCategory category,
                      ParameterValueType value_type, ReadWriteAccess access, const QVariant& value) {
    Q_UNUSED(parent.addChild(makeParameterRow(makeMetaData(id, name, category, value_type, access), value)))
}

/// @brief Builds a device's worth of parameters, nested a few groups deep.
void buildMockParameterTree(RowData& root) {
    // Loose parameters at the top level, to keep an eye on how ungrouped ones read next to the
    // groups — that is exactly what a device reporting no grouping at all would look like.
    addMockParameter(root, 0x00, "Device ID", ParameterCategory::saved_parameter, ParameterValueType::uint8,
                     ReadWriteAccess::read_write, QVariant::fromValue<quint16>(1));
    addMockParameter(root, 0x01, "Firmware Version", ParameterCategory::runtime_parameter, ParameterValueType::uint32,
                     ReadWriteAccess::read_only, QVariant::fromValue<quint32>(0x00010204));

    RowData* motor = root.addChild(makeGroupRow("Motor"));
    addMockParameter(*motor, 0x10, "Pole Pairs", ParameterCategory::saved_parameter, ParameterValueType::uint8,
                     ReadWriteAccess::read_write, QVariant::fromValue<quint16>(7));
    addMockParameter(*motor, 0x11, "Phase Resistance", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 0.085f);
    addMockParameter(*motor, 0x12, "Phase Inductance", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 0.000034f);
    addMockParameter(*motor, 0x13, "Max Current", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 24.0f);
    addMockParameter(*motor, 0x14, "Direction Inverted", ParameterCategory::saved_parameter,
                     ParameterValueType::boolean, ReadWriteAccess::read_write, false);

    // Nested a level deeper than the rest, to see how far the indentation carries.
    RowData* control      = root.addChild(makeGroupRow("Control"));

    RowData* current_loop = control->addChild(makeGroupRow("Current Loop"));
    addMockParameter(*current_loop, 0x20, "Kp", ParameterCategory::saved_parameter, ParameterValueType::floating_point,
                     ReadWriteAccess::read_write, 0.45f);
    addMockParameter(*current_loop, 0x21, "Ki", ParameterCategory::saved_parameter, ParameterValueType::floating_point,
                     ReadWriteAccess::read_write, 120.0f);
    addMockParameter(*current_loop, 0x22, "Bandwidth", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 1500.0f);

    RowData* velocity_loop = control->addChild(makeGroupRow("Velocity Loop"));
    addMockParameter(*velocity_loop, 0x30, "Kp", ParameterCategory::saved_parameter, ParameterValueType::floating_point,
                     ReadWriteAccess::read_write, 0.02f);
    addMockParameter(*velocity_loop, 0x31, "Ki", ParameterCategory::saved_parameter, ParameterValueType::floating_point,
                     ReadWriteAccess::read_write, 0.4f);
    addMockParameter(*velocity_loop, 0x32, "Max Velocity", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 320.0f);

    RowData* position_loop = control->addChild(makeGroupRow("Position Loop"));
    addMockParameter(*position_loop, 0x40, "Kp", ParameterCategory::saved_parameter, ParameterValueType::floating_point,
                     ReadWriteAccess::read_write, 12.5f);
    addMockParameter(*position_loop, 0x41, "Max Position Error", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 0.25f);

    RowData* encoder = root.addChild(makeGroupRow("Encoder"));
    addMockParameter(*encoder, 0x50, "Counts Per Revolution", ParameterCategory::saved_parameter,
                     ParameterValueType::uint32, ReadWriteAccess::read_write, QVariant::fromValue<quint32>(16384));
    addMockParameter(*encoder, 0x51, "Zero Offset", ParameterCategory::saved_parameter, ParameterValueType::int32,
                     ReadWriteAccess::read_write, -412);
    addMockParameter(*encoder, 0x52, "Inverted", ParameterCategory::saved_parameter, ParameterValueType::boolean,
                     ReadWriteAccess::read_write, false);

    RowData* protection = root.addChild(makeGroupRow("Protection"));
    addMockParameter(*protection, 0x60, "Over Temperature Limit", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 85.0f);
    addMockParameter(*protection, 0x61, "Over Voltage Limit", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 28.0f);
    addMockParameter(*protection, 0x62, "Under Voltage Limit", ParameterCategory::saved_parameter,
                     ParameterValueType::floating_point, ReadWriteAccess::read_write, 10.5f);
    addMockParameter(*protection, 0x63, "Fault Latch Enabled", ParameterCategory::saved_parameter,
                     ParameterValueType::boolean, ReadWriteAccess::read_write, true);

    // Signals are read only, so this group also covers how the greyed out value cells look.
    RowData* diagnostics = root.addChild(makeGroupRow("Diagnostics"));
    addMockParameter(*diagnostics, 0x70, "Bus Voltage", ParameterCategory::signal, ParameterValueType::floating_point,
                     ReadWriteAccess::read_only, 23.8f);
    addMockParameter(*diagnostics, 0x71, "Board Temperature", ParameterCategory::signal,
                     ParameterValueType::floating_point, ReadWriteAccess::read_only, 41.2f);
    addMockParameter(*diagnostics, 0x72, "Phase Current Q", ParameterCategory::signal,
                     ParameterValueType::floating_point, ReadWriteAccess::read_only, 3.15f);
    addMockParameter(*diagnostics, 0x73, "Phase Current D", ParameterCategory::signal,
                     ParameterValueType::floating_point, ReadWriteAccess::read_only, -0.08f);
    addMockParameter(*diagnostics, 0x74, "Measured Velocity", ParameterCategory::signal,
                     ParameterValueType::floating_point, ReadWriteAccess::read_only, 128.4f);
    addMockParameter(*diagnostics, 0x75, "Measured Position", ParameterCategory::signal, ParameterValueType::int32,
                     ReadWriteAccess::read_only, 7213);
    addMockParameter(*diagnostics, 0x76, "Fault Flags", ParameterCategory::signal, ParameterValueType::uint16,
                     ReadWriteAccess::read_only, QVariant::fromValue<quint16>(0));
    addMockParameter(*diagnostics, 0x77, "Uptime", ParameterCategory::signal, ParameterValueType::uint32,
                     ReadWriteAccess::read_only, QVariant::fromValue<quint32>(38471));
}

/// @brief Nudges a mock value, so that refreshing a signal visibly does something.
QVariant jitterMockValue(const RowData& row) {
    const double jitter = QRandomGenerator::global()->bounded(-100, 101) / 1000.0;  // +/- 10%

    switch (row.meta_data.value_type) {
        case ParameterValueType::floating_point:
        case ParameterValueType::double_float:
            return static_cast<float>(row.value.toDouble() * (1.0 + jitter));
        case ParameterValueType::int8:
        case ParameterValueType::int16:
        case ParameterValueType::int32:
        case ParameterValueType::int64:
            return static_cast<int>(row.value.toInt() * (1.0 + jitter));
        default:
            return row.value;
    }
}

}  // namespace
// --- END MOCK PARAMETERS ---
// ----------------------------------------------------------------------------------------------

ParameterTableWidget::ParameterTableWidget(QWidget* parent) : QWidget(parent), ui_(new Ui::ParameterTableWidget) {
    ui_->setupUi(this);

    // Configure the tree ui
    ui_->parameterTreeView->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui_->parameterTreeView->header()->setStretchLastSection(true);  // tree still fills full width
    ui_->parameterTreeView->setUniformRowHeights(true);
    ui_->parameterTreeView->setAllColumnsShowFocus(true);
    // Nesting is shown by indentation alone, so the expander is the only decoration a group gets.
    ui_->parameterTreeView->setRootIsDecorated(true);

    // The status line is a footnote about the filter, not part of the data.
    QPalette status_palette = ui_->statusLabel->palette();
    status_palette.setColor(QPalette::WindowText, status_palette.color(QPalette::Disabled, QPalette::WindowText));
    ui_->statusLabel->setPalette(status_palette);

    // Connect filter input to the proxy model
    Q_UNUSED(QObject::connect(ui_->filterLineEdit, &QLineEdit::textChanged, this, &ParameterTableWidget::applyFilter));

    Q_UNUSED(QObject::connect(ui_->expandAllToolButton, &QToolButton::clicked, this,
                              [this] { ui_->parameterTreeView->expandAll(); }));
    Q_UNUSED(QObject::connect(ui_->collapseAllToolButton, &QToolButton::clicked, this,
                              [this] { ui_->parameterTreeView->collapseAll(); }));

    // Configure refresh buttons and the auto refresh setting.
    // Default refresh (button + timer) only re-reads Signal parameters; Saved/Runtime are written by
    // this dev tool and don't change without our knowledge. The "Force Refresh All" button re-reads
    // everything for sanity checks (e.g., another tool wrote, reconnect, etc.).
    Q_UNUSED(QObject::connect(ui_->refreshPushButton, &QPushButton::clicked, this,
                              &ParameterTableWidget::refreshSignalParameterValues));
    Q_UNUSED(QObject::connect(ui_->forceRefreshAllPushButton, &QPushButton::clicked, this,
                              &ParameterTableWidget::refreshAllParameterValues));
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
    Q_UNUSED(QObject::connect(&refresh_timer_, &QTimer::timeout, this,
                              &ParameterTableWidget::refreshSignalParameterValues));
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

void ParameterTableWidget::initialize(servo_core_control_api::Device* device) {
    /** ONLY DO THE INITIALIZATION HERE THAT CANNOT BE DONE BEFORE HAVING THE HANDLE TO THE DEVICE **/
    device_ = device;
    buildParameterTree();

    table_model_ = new ParameterTableModel(root_, this);
    // The model is applied through the proxy model to provide ability to filter what rows are
    // visible or not, and to sort them without the model having to reorder its own tree.
    table_filter_proxy_model_.setSourceModel(table_model_);
    ui_->parameterTreeView->setModel(&table_filter_proxy_model_);

    // Sorting is only connected once there is a model, so that enabling it does not sort an
    // empty view and leave the header indicator out of step with the rows.
    ui_->parameterTreeView->setSortingEnabled(true);
    ui_->parameterTreeView->sortByColumn(static_cast<int>(Columns::id), Qt::AscendingOrder);

    // Groups start open: a collapsed group hides the parameters that are the point of the view.
    ui_->parameterTreeView->expandAll();

    // When the user edits a cell, push the new value to the device.
    Q_UNUSED(QObject::connect(table_model_, &ParameterTableModel::parameterValueChanged, this,
                              &ParameterTableWidget::writeParameterValue));

    name_delegate_ = new ParameterNameDelegate(this);
    ui_->parameterTreeView->setItemDelegateForColumn(static_cast<int>(Columns::name), name_delegate_);

    value_delegate_ = new ParameterValueDelegate(this);
    ui_->parameterTreeView->setItemDelegateForColumn(static_cast<int>(Columns::value), value_delegate_);

    updateStatusLabel();
}

void ParameterTableWidget::buildParameterTree() {
    root_ = RowData{};
    parameter_rows_.clear();

    // --- MOCK PARAMETERS (issue #3, stage 1) ---
    // Restore this once the device reports its parameter groups. Every parameter goes straight
    // under the root while it reports none, which renders as a flat table.
    //
    // for (uint8_t id : device_->fetchRegisteredParamIds()) {
    //     parameter_system::ParameterMetaData meta_data = device_->fetchParameterMetaData(id);
    //     QVariant                            value     = getParameterValue(id, meta_data.value_type);
    //     Q_UNUSED(root_.addChild(makeParameterRow(meta_data, value)))
    // }
    buildMockParameterTree(root_);
    // --- END MOCK PARAMETERS ---

    collectParameterRows(root_);
}

void ParameterTableWidget::collectParameterRows(RowData& node) {
    for (const std::unique_ptr<RowData>& child : node.children) {
        if (child->kind == RowData::Kind::parameter) parameter_rows_.push_back(child.get());
        collectParameterRows(*child);
    }
}

void ParameterTableWidget::applyFilter(const QString& text) {
    table_filter_proxy_model_.setFilterText(text);
    if (name_delegate_ != nullptr) name_delegate_->setHighlightedText(text);

    // A match inside a collapsed group would otherwise be filtered down to an unhelpful heading.
    if (!text.isEmpty()) ui_->parameterTreeView->expandAll();

    ui_->parameterTreeView->viewport()->update();
    updateStatusLabel();
}

void ParameterTableWidget::updateStatusLabel() {
    const int total_count = root_.parameterCount();

    if (table_filter_proxy_model_.filterText().isEmpty()) {
        ui_->statusLabel->setText(QStringLiteral("%1 parameters").arg(total_count));
        return;
    }

    const int visible_count = table_filter_proxy_model_.visibleParameterCount();
    ui_->statusLabel->setText(QStringLiteral("%1 of %2 parameters match").arg(visible_count).arg(total_count));
}

void ParameterTableWidget::refreshSignalParameterValues() {
    // Only Signal parameters can change without our knowledge — the device sets them autonomously.
    // Saved and Runtime parameters are written by the master (this dev tool) and stay put until we
    // write them again, so re-reading them every refresh just wastes bus bandwidth.
    for (RowData* row : parameter_rows_) {
        if (row->meta_data.category != parameter_system::ParameterCategory::signal) continue;

        // --- MOCK PARAMETERS (issue #3, stage 1) ---
        // Without a device there is nothing to read back, so the values are nudged instead. That
        // keeps the refresh button and the auto refresh interval doing something visible while
        // the look of the tree is being settled.
        if (device_ == nullptr) {
            table_model_->updateValueFromDevice(*row, jitterMockValue(*row));
            continue;
        }
        // --- END MOCK PARAMETERS ---

        QVariant new_value = getParameterValue(row->meta_data.id, row->meta_data.value_type);
        table_model_->updateValueFromDevice(*row, new_value);
    }
}

void ParameterTableWidget::refreshAllParameterValues() {
    // Re-reads every parameter regardless of category. Used as a manual sanity check (e.g., another
    // tool may have written, after reconnect, debugging) — not used by the auto-refresh timer.
    for (RowData* row : parameter_rows_) {
        // --- MOCK PARAMETERS (issue #3, stage 1) ---
        if (device_ == nullptr) {
            table_model_->updateValueFromDevice(*row, jitterMockValue(*row));
            continue;
        }
        // --- END MOCK PARAMETERS ---

        QVariant new_value = getParameterValue(row->meta_data.id, row->meta_data.value_type);
        table_model_->updateValueFromDevice(*row, new_value);
    }
}

QVariant ParameterTableWidget::getParameterValue(parameter_system::ParameterID        id,
                                                 parameter_system::ParameterValueType type) const {
    using parameter_system::ParameterDeclaration;
    using parameter_system::ParameterValueType;

    if (device_ == nullptr) return {};

    switch (type) {
        case ParameterValueType::uint8:
            // Widen to quint16 — QVariant treats uint8_t (== unsigned char) as a character
            // and would display the codepoint instead of the number.
            return QVariant::fromValue(static_cast<quint16>(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::uint8>{id})));
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
            // Widen to qint16 — QVariant treats int8_t (== signed char) as a character.
            return QVariant::fromValue(static_cast<qint16>(
                device_->readParameterValue(ParameterDeclaration<ParameterValueType::int8>{id})));
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

    // --- MOCK PARAMETERS (issue #3, stage 1) ---
    // The edit has already landed in the model, which is as far as it can go without a device.
    if (device_ == nullptr) return;
    // --- END MOCK PARAMETERS ---

    // Find the row to look up the value type — the signal only carries id + value.
    const RowData* row = nullptr;
    for (const RowData* candidate : parameter_rows_) {
        if (candidate->meta_data.id == id) {
            row = candidate;
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
