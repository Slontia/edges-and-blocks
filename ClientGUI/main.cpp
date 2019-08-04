#include "ClientGUI.h"
#include "new_game_widget.h"
#include <QtWidgets/QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QSignalMapper>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NewGameWidget w(NULL);
    w.show();
    return a.exec();
}
