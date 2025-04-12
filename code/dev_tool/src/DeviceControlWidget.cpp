// You may need to build the project (run Qt uic code generator) to get "ui_DeviceDelegateWidget.h" resolved

#include "DeviceControlWidget.h"

#include "helpers.h"
#include "parameter_table/ParameterTableWidget.h"
#include "ui_DeviceControlWidget.h"

DeviceControlWidget::DeviceControlWidget(servo_core_control_api::Device& device, QWidget* parent)
    : QWidget(parent), ui(new Ui::DeviceControlWidget), device_(device) {
    ui->setupUi(this);

    ui->deviceIdLabel->setText(helpers::intToHexString(device.getId()));

    ui->parameterTableWidget->initialize(device_);
}

DeviceControlWidget::~DeviceControlWidget() { delete ui; }

void DeviceControlWidget::setDeviceNickname(const QString& nickname) { ui->deviceNicknameLabel->setText(nickname); }
