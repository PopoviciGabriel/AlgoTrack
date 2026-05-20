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
    // Construim o paletă grafică de tip Light Mode complet izolată de setările sistemului
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(248, 250, 252));        // Fundal ferestre (#f8fafc)
    lightPalette.setColor(QPalette::WindowText, QColor(30, 41, 59));       // Text general (#1e293b)
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));          // Fundal casete/tabele
    lightPalette.setColor(QPalette::AlternateBase, QColor(248, 250, 252)); // Rânduri alternate
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(30, 41, 59));
    lightPalette.setColor(QPalette::Text, QColor(30, 41, 59));             // Text în casete de input
    lightPalette.setColor(QPalette::Button, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ButtonText, QColor(30, 41, 59));
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(37, 99, 235));
    lightPalette.setColor(QPalette::Highlight, QColor(219, 234, 254));     // Fundal selecție albastru deschis
    lightPalette.setColor(QPalette::HighlightedText, QColor(30, 64, 175));  // Text selecție albastru închis
    
    // Aplicăm paleta curată la nivelul întregii aplicații
    QApplication::setPalette(lightPalette);

    // 3. ÎNCĂRCARE ROBUSTĂ ȘI GLOBALĂ A STILULUI (style.qss)
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList qssPaths;
    qssPaths << appDir + "/style.qss"
             << appDir + "/../style.qss"
             << appDir + "/../../style.qss"
             << appDir + "/../qt_app/style.qss"
             << "qt_app/style.qss"
             << "style.qss";

    bool qssLoaded = false;
    for (const QString &path : qssPaths)
    {
        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream stream(&file);
            a.setStyleSheet(stream.readAll()); // Aplică CSS-ul global
            qssLoaded = true;
            qDebug() << "Style sheet loaded successfully from:" << path;
            break;
        }
    }

    if (!qssLoaded) {
        qWarning() << "Critical: Could not load style sheet globally.";
    }

    MainWindow w;
    w.show();
    return a.exec();
}