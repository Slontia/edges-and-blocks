#include "ClientGUI.h"
#include "new_game_widget.h"
#include <QtWidgets/QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QSignalMapper>

void NewGameWidget::open_client_gui()
{
  if (client_gui)
  {
    client_gui->close();
    delete client_gui;
  }
  client_gui = new ClientGUI(nullptr);
  client_gui->show();
  hide();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NewGameWidget w(NULL);
    w.show();
    return a.exec();
}
