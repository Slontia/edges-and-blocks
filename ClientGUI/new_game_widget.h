#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <memory>

class ClientGUI;

class NewGameWidget : public QMainWindow
{
  Q_OBJECT

public:
  NewGameWidget(QWidget *parent = Q_NULLPTR);

public slots:
  void open_client_gui_local();
  void open_client_gui_network();

private:
  void open_client_gui(std::shared_ptr<ClientGUI>& client_gui);

private:
  std::shared_ptr<ClientGUI> client_gui_ = nullptr;
};