#ifndef DEV_TOOL_MAINWINDOW_H
#define DEV_TOOL_MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

#include "DeviceControlWidget.h"
#include "control_api/Device.h"
#include "control_api/windows/Context.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~        MainWindow() override;

private slots:
    void onComPortChanged(int index);
    void refreshDeviceList();
    void openDeviceDelegateWidget();

private:
    Ui::MainWindow* ui;

    servo_core_control_api::windows::Context* context_ = nullptr;

    QVector<servo_core_control_api::Device> devices_;

    void setupComPortSelector();
};

#endif  // DEV_TOOL_MAINWINDOW_H
