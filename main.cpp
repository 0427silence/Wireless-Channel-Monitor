#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(12, 16, 40));
    palette.setColor(QPalette::WindowText, QColor(200, 210, 240));
    palette.setColor(QPalette::Base, QColor(20, 26, 52));
    palette.setColor(QPalette::AlternateBase, QColor(26, 32, 60));
    palette.setColor(QPalette::ToolTipBase, QColor(30, 40, 70));
    palette.setColor(QPalette::ToolTipText, QColor(180, 200, 240));
    palette.setColor(QPalette::Text, QColor(200, 210, 240));
    palette.setColor(QPalette::Button, QColor(20, 28, 56));
    palette.setColor(QPalette::ButtonText, QColor(180, 200, 240));
    palette.setColor(QPalette::BrightText, Qt::cyan);
    palette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(palette);

    app.setStyleSheet(R"(
        * { font-family: "Segoe UI", "Microsoft YaHei"; }
    )");

    MainWindow window;
    window.setWindowTitle("Wireless Channel Monitor");
    window.resize(1200, 720);
    window.show();

    return app.exec();
}
