#include "BDeditor.h"

#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BaseEditor w;
    w.show();
    return a.exec();
}
