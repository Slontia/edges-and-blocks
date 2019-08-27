#include "new_game_widget.h"
#include "ClientGUI.h"
#include <iostream>
#include "client.h"
#include <QDebug>
#include <QMessageBox>
#include <cassert>

NewGameWidget::NewGameWidget(QWidget *parent) : QMainWindow(parent), client_(nullptr),
    local_game_(new QPushButton("Local Game", this)), network_game_(new QPushButton("Network Game", this))
{
  setFixedSize(200, 100);
  QObject::connect(local_game_, SIGNAL(clicked()), this, SLOT(open_client_gui_local()));
  QObject::connect(network_game_, SIGNAL(clicked()), this, SLOT(wait_for_open_client_gui_network()));
  network_game_->move(0, 30);
}

void NewGameWidget::open_client_gui_local()
{
  open_client_gui(std::make_shared<ClientGUI>());
}

void NewGameWidget::wait_for_open_client_gui_network()
{
  network_forbidden_new_game();
  try
  {
    assert(client_ == nullptr);
    client_ = std::make_unique<ClientAsyncWrapper>();
    client_->wait_for_game_start_async([&](bool is_offen)
    {
      /* We use shared_ptr instead of unique_ptr to support polymorphic. */
      auto client_gui = std::make_shared<ClientGUINetwork>(client_, is_offen);
      open_client_gui(std::dynamic_pointer_cast<ClientGUI>(client_gui));
    });
  }
  catch (std::exception& e)
  {
    network_allow_new_game();
    QMessageBox::critical(this, "Error", (QString) "Cannot connect to server.\nInfo: " + e.what());
  }
}

void NewGameWidget::cancel_client_gui_network()
{
  // TODO: send signal to client_, cancel socket
  client_ = nullptr;
  network_allow_new_game();
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

void NewGameWidget::network_forbidden_new_game()
{
  network_game_->disconnect();
  QObject::connect(network_game_, SIGNAL(clicked()), this, SLOT(cancel_client_gui_network()));
  local_game_->setEnabled(false);
  network_game_->setText("Cancel");
}

void NewGameWidget::network_allow_new_game()
{
  network_game_->disconnect();
  QObject::connect(network_game_, SIGNAL(clicked()), this, SLOT(wait_for_open_client_gui_network()));
  local_game_->setEnabled(true);
  network_game_->setText("Network Game");
}