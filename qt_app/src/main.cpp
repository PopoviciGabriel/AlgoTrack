#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QPalette>
#include <QColor>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. Forțăm stilul Fusion clean cross-platform
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // 2. NEUTRALIZARE WINDOWS 11 DARK MODE
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(248, 250, 252));
    lightPalette.setColor(QPalette::WindowText, QColor(30, 41, 59));
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(248, 250, 252));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(30, 41, 59));
    lightPalette.setColor(QPalette::Text, QColor(30, 41, 59));
    lightPalette.setColor(QPalette::Button, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ButtonText, QColor(30, 41, 59));
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(37, 99, 235));
    lightPalette.setColor(QPalette::Highlight, QColor(219, 234, 254));
    lightPalette.setColor(QPalette::HighlightedText, QColor(30, 64, 175));

    QApplication::setPalette(lightPalette);

    // 3. ÎNCĂRCARE DIN QRC (Resource Compiler)
    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        a.setStyleSheet(stream.readAll());
        qDebug() << "Style sheet loaded successfully from resources.";
    }
    else
    {
        qWarning() << "Critical: Could not load style sheet from resources.";
    }

    MainWindow w;
    w.show();
    return a.exec();
}