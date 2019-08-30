#include "new_game_widget.h"
#include "ClientGUI.h"
#include <iostream>
#include "client.h"
#include <QDebug>
#include <QMessageBox>
#include <cassert>
#include <QLineEdit>
#include <QLabel>

NewGameWidget::NewGameWidget(QWidget *parent) : QMainWindow(parent), client_(nullptr), 
  local_game_(new QPushButton("Local Game", this)), 
  network_game_(new QPushButton("Network Game", this)),
  ip_edit_(new QLineEdit(this)),
  port_edit_(new QLineEdit(this))
{
  setFixedSize(225, 100);

  QObject::connect(local_game_, SIGNAL(clicked()), this, SLOT(open_client_gui_local()));
  local_game_->setFixedSize(100, 30);
  local_game_->move(10, 10);
  
  QObject::connect(network_game_, SIGNAL(clicked()), this, SLOT(wait_for_open_client_gui_network()));
  network_game_->setFixedSize(100, 30);
  network_game_->move(115, 10);
  
  (new QLabel("IP Address:",this))->move(20, 40);
  QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
  QRegExp ipRegex("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$");
  ip_edit_->setValidator(new QRegExpValidator(ipRegex, ip_edit_));
  ip_edit_->setPlaceholderText("114.51.41.91");
  ip_edit_->move(100, 45);
  ip_edit_->setFixedSize(110, 20);

  (new QLabel("Port:", this))->move(20, 65);
  port_edit_->setValidator(new QIntValidator(port_edit_));
  port_edit_->setPlaceholderText("9810");
  port_edit_->move(100, 70);
  port_edit_->setFixedSize(40, 20);
  port_edit_->setMaxLength(5);
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
    client_ = std::make_unique<ClientAsyncWrapper>(ip_edit_->text().toStdString(), port_edit_->text().toInt());
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
