#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <memory>
#include "client.h"

class ClientGUI;

class NewGameWidget : public QMainWindow
{
  Q_OBJECT

public:
  NewGameWidget(QWidget *parent = Q_NULLPTR);

public slots:
  void start_game();
  void cancel_client_gui_network();
  void refresh_enable_options();

private:
  enum GameType : uint32_t { LOCAL_GAME, COM_GAME, NETWORK_GAME };
  std::unique_ptr<ClientAsyncWrapper> client_;
  std::shared_ptr<ClientGUI> client_gui_ = nullptr;
  QPushButton* start_game_btn_;
  QButtonGroup* game_type_gbtn_;
  QAbstractButton* local_game_rbtn_;
  QAbstractButton* com_game_rbtn_;
  QAbstractButton* network_game_rbtn_;
  QLineEdit* ip_edit_;
  QLineEdit* port_edit_;
  QSpinBox* side_len_spin_;
  QSpinBox* win_blocks_spin_;
  QSpinBox* init_offen_edges_spin_;
  QSpinBox* init_defen_edges_spin_;
  void open_client_gui(std::shared_ptr<ClientGUI>& client_gui);
  void network_forbidden_new_game();
  void wait_for_open_client_gui_network();
  void enable_new_game();
};