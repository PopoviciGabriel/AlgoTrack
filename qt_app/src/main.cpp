#include "MainWindow.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QFont uiFont;
    uiFont.setFamilies({"Segoe UI Variable Text", "Segoe UI Variable", "Segoe UI", "SF Pro Text"});
    uiFont.setPointSize(10);
    app.setFont(uiFont);

    QPalette palette;
    const QColor text(245, 245, 247);
    palette.setColor(QPalette::Window, QColor(16, 16, 18));
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, QColor(28, 28, 30));
    palette.setColor(QPalette::AlternateBase, QColor(44, 44, 46));
    palette.setColor(QPalette::ToolTipBase, QColor(44, 44, 46));
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, QColor(44, 44, 46));
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::Highlight, QColor(10, 132, 255, 80));
    palette.setColor(QPalette::HighlightedText, text);
    palette.setColor(QPalette::PlaceholderText, QColor(235, 235, 245, 120));
    app.setPalette(palette);

    MainWindow window;
    window.show();
    return app.exec();
}
