#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <locale.h>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "Russian");
	CreateMutexA(0, FALSE, "Local\\$myprogram$");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return -1;
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
