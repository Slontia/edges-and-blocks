#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <memory>
#include "client.h"

class ClientGUI;

class NewGameWidget : public QMainWindow
{
  Q_OBJECT

public:
  NewGameWidget(QWidget *parent = Q_NULLPTR);

public slots:
  void open_client_gui_local();
  void wait_for_open_client_gui_network();
  void cancel_client_gui_network();

private:
  std::unique_ptr<ClientAsyncWrapper> client_;
  std::shared_ptr<ClientGUI> client_gui_ = nullptr;
  QPushButton* local_game_;
  QPushButton* network_game_;
  void open_client_gui(std::shared_ptr<ClientGUI>& client_gui);
  void network_forbidden_new_game();
  void network_allow_new_game();
  
};