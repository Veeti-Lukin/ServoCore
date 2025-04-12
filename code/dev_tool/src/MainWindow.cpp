// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
// UI file
#include "ui_MainWindow.h"
// ####

#include <QBoxLayout>
#include <QSerialPortInfo>
#include <QStackedLayout>
#include <QString>

#include "helpers.h"
#include "parameter_table/ParameterTableModel.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle(QCoreApplication::applicationName());

    setupComPortSelector();
    // TODO find the devices

    Q_UNUSED(QObject::connect(ui->refreshDevicesPushButton, &QPushButton::clicked, this, refreshDeviceList));
    Q_UNUSED(QObject::connect(ui->deviceListWidget, &QListWidget::itemPressed, this, openDeviceDelegateWidget));
    ui->deviceListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    Q_UNUSED(QObject::connect(ui->deviceListWidget, &QListWidget::customContextMenuRequested, this,
                              [] { qDebug() << "ADSAD"; }));
}

MainWindow::~MainWindow() {
    delete ui;
    delete context_;
}

void MainWindow::onComPortChanged(int index) {
    QString com_port_name = ui->comPortSelectionComboBox->itemData(index).toString();

    delete context_;
    if (!com_port_name.isEmpty()) {
        context_ = new servo_core_control_api::windows::Context(com_port_name.toStdString());
    }
    refreshDeviceList();
}

void MainWindow::refreshDeviceList() {
    // TODO somehow close the current device widget to not cause any issues when the device instance destructs
    // ui->deviceDelegateContainerWidget->childAt(0, 0)->close();

    devices_.clear();
    ui->deviceListWidget->clear();

    if (context_ == nullptr) return;

    /* TODO Timeouts do not work yet on the serial framework so this cant be done yet
    for (size_t i = 0; i <= Device::K_MAX_DEVICE_ID; i++) {
        std::optional<Device> opt_device = context_->tryFindDeviceById(0);
        // device was not found, at least until the timeout triggered
        if (!opt_device) continue;

        devices_.push_back(*opt_device);
        ui->deviceListWidget->addItem(helpers::intToHexString(i));
    } */
    std::optional<servo_core_control_api::Device> opt_device = context_->tryFindDeviceById(0);
    if (!opt_device) {
        qDebug() << "Device not found";
        return;
    }
    devices_.push_back(*opt_device);
    ui->deviceListWidget->addItem(helpers::intToHexString(0));
}

void MainWindow::openDeviceDelegateWidget() {
    size_t                          selected_device_id = ui->deviceListWidget->currentItem()->text().toInt();
    servo_core_control_api::Device& selected_device =
        *std::find_if(devices_.begin(), devices_.end(),
                      [&](servo_core_control_api::Device& device) { return device.getId() == selected_device_id; });

    ui->centralwidget->layout()->addWidget(new DeviceControlWidget(selected_device, ui->centralwidget));
}

void MainWindow::setupComPortSelector() {
    // Empty initialization for the selector
    ui->comPortSelectionComboBox->addItem("Not selected", "");

    for (const QSerialPortInfo& serial_port_info : QSerialPortInfo::availablePorts()) {
        ui->comPortSelectionComboBox->addItem(serial_port_info.portName() + " - " + serial_port_info.description(),
                                              serial_port_info.portName());
    }

    Q_UNUSED(QObject::connect(ui->comPortSelectionComboBox, &QComboBox::currentIndexChanged, this,
                              &MainWindow::onComPortChanged));
}
