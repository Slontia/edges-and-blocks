#include "new_game_widget.h"
#include "ClientGUI.h"
#include <iostream>
#include "client.h"

NewGameWidget::NewGameWidget(QWidget *parent) : QMainWindow(parent)
{
  setFixedSize(200, 100);
  QPushButton *local_game = new QPushButton("Local Game", this);
  QObject::connect(local_game, SIGNAL(clicked()), this, SLOT(open_client_gui_local()));
  QPushButton *network_game = new QPushButton("Network Game", this);
  network_game->move(0, 30);
}

void NewGameWidget::open_client_gui_local()
{
  open_client_gui(std::make_shared<ClientGUI>());
}
void NewGameWidget::open_client_gui_network()
{
  try
  {
    std::unique_ptr<Client> client = std::make_unique<Client>();
    open_client_gui(std::dynamic_pointer_cast<ClientGUI>(std::make_shared<ClientGUINetwork>(client)));
  }
  catch (std::exception& e)

  {
    std::cout << e.what() << std::endl;
  }
}

void NewGameWidget::open_client_gui(std::shared_ptr<ClientGUI>& client_gui)
{
  if (client_gui_)
  {
    client_gui->close();
  }
  client_gui_ = std::move(client_gui);
  client_gui_->show();
  hide();
}