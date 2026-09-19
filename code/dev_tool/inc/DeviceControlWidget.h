#ifndef DEVICEDELEGATEWIDGET_H
#define DEVICEDELEGATEWIDGET_H

#include <QWidget>

#include "control_api/Device.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class DeviceControlWidget;
}
QT_END_NAMESPACE

class DeviceControlWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @param device The device to control. May be nullptr, in which case the widget shows mock
     *               data and never talks to a device.
     * @param parent The parent widget (optional).
     */
    explicit DeviceControlWidget(servo_core_control_api::Device* device, QWidget* parent = nullptr);
    ~        DeviceControlWidget() override;

    void setDeviceNickname(const QString& nickname);

private:
    Ui::DeviceControlWidget* ui;

    servo_core_control_api::Device* device_;  ///< Non-owning reference to the device, or nullptr.
};

#endif  // DEVICEDELEGATEWIDGET_H
