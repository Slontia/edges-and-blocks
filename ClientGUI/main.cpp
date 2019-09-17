#include "ClientGUI.h"
#include "new_game_widget.h"
#include <QtWidgets/QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QSignalMapper>

void set_font(QApplication& a)
{
  QFont font = a.font();
  //font.setPointSize(9);
  font.setPixelSize(12);
  a.setFont(font);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    set_font(a);
    NewGameWidget w(NULL);
    w.show();
    return a.exec();
}
