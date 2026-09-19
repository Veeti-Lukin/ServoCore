// You may need to build the project (run Qt uic code generator) to get "ui_DeviceDelegateWidget.h" resolved

#include "DeviceControlWidget.h"

#include "helpers.h"
#include "parameter_table/ParameterTableWidget.h"
#include "ui_DeviceControlWidget.h"

DeviceControlWidget::DeviceControlWidget(servo_core_control_api::Device* device, QWidget* parent)
    : QWidget(parent), ui(new Ui::DeviceControlWidget), device_(device) {
    ui->setupUi(this);

    // --- MOCK PARAMETERS (issue #3, stage 1) ---
    // A null device is the stand-in for one that is not there. Restore the plain
    // helpers::intToHexString(device_->getId()) once the mock is gone.
    ui->deviceIdLabel->setText(device_ != nullptr ? helpers::intToHexString(device_->getId())
                                                  : QStringLiteral("mock device"));
    // --- END MOCK PARAMETERS ---

    ui->parameterTableWidget->initialize(device_);
}

DeviceControlWidget::~DeviceControlWidget() { delete ui; }

void DeviceControlWidget::setDeviceNickname(const QString& nickname) { ui->deviceNicknameLabel->setText(nickname); }
