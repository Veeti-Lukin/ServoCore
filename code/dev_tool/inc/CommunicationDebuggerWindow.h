#ifndef COMMUNICATIONDEBUGGERWINDOW_H
#define COMMUNICATIONDEBUGGERWINDOW_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class CommunicationDebuggerWindow; }
QT_END_NAMESPACE

class CommunicationDebuggerWindow : public QWidget {
Q_OBJECT

public:
    explicit CommunicationDebuggerWindow(QWidget *parent = nullptr);
    ~CommunicationDebuggerWindow() override;

private:
    Ui::CommunicationDebuggerWindow *ui;
};


#endif //COMMUNICATIONDEBUGGERWINDOW_H
