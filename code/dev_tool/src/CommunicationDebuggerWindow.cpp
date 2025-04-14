// You may need to build the project (run Qt uic code generator) to get "ui_CommunicationDebuggerWindow.h" resolved

#include "CommunicationDebuggerWindow.h"

#include "ui_CommunicationDebuggerWindow.h"

// TODO CHANGE THE NAME TO COMMUNICATION MONITOR?

CommunicationDebuggerWindow::CommunicationDebuggerWindow(QWidget* parent)
    : QWidget(parent), ui(new Ui::CommunicationDebuggerWindow) {
    ui->setupUi(this);

    ui->communicationVisualizerTextEdit->append("<b>User:</b> Hello!");
    ui->communicationVisualizerTextEdit->append("<font color='gray'>[12:30]</font> <b>Bot:</b> Hi there!");
}

CommunicationDebuggerWindow::~CommunicationDebuggerWindow() { delete ui; }
