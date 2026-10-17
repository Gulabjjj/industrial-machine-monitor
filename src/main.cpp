#include "MainWindow.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Industrial Machine Monitoring System");
    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    // Inside the container there is no window manager: fill the virtual screen.
    if (qEnvironmentVariableIsSet("MONITOR_KIOSK")) {
        w.setWindowFlag(Qt::FramelessWindowHint);
        w.setGeometry(QGuiApplication::primaryScreen()->geometry());
    }
    w.show();
    return app.exec();
}
