#include <QApplication>

#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("ServoCore DevTool");

    app.setStyle("Fusion");

    MainWindow main_window;
    main_window.show();

    return app.exec();
}