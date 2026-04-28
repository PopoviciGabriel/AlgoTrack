#include "MainWindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AlgoTrack");

    QFont font("Segoe UI", 10);
    app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
