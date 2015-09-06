#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("inspectrum");
    MainWindow mainWin;
    mainWin.show();
    if (argc > 1) {
        mainWin.openFile(QString::fromLatin1(argv[1]));
    }
    return a.exec();
}