#include "ClientGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientGUI w;
    w.show();
    return a.exec();
}