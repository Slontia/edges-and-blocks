#pragma once

#include <QtWidgets/QMainWindow>
#include <QPushButton>
#include <unordered_map>
#include <QTextEdit>
#include <QThread>

#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"
#include "../GameCore/game.h"
#include "moving_selector.h"

class ClientAsyncWrapper;

class ClientGUI;
class MovingSelectManager;
class GameInfo;
class TurningSwitcher;
class GameFunctions;
class EdgeButton;
class AreaButton;
class BoardWidget;

class ClientGUI : public QMainWindow
{
  Q_OBJECT

public:
  ClientGUI(QWidget *parent = Q_NULLPTR);

public slots:
  void EdgeButtonEvent();
  virtual void PassButtonEvent();
  virtual void RetractButtonEvent();
  virtual void show_new_game_widget();
  
protected:
  std::unique_ptr<MovingSelectManager> select_manager_;
  std::unique_ptr<Game> game_;
  TurningSwitcher* turning_switcher_;
  QTextEdit* notification_;
  GameFunctions* functions_;
  BoardWidget* board_;
  virtual bool try_act(const EdgeButton* target_edge);
  void impl_game_variety(const GameVariety& game_var);
  void reset_game_variety(const GameVariety& game_var);

private:
  QPushButton* pass_;
  const QSize kWindowSize = QSize(800, 600);
  const QPoint kBoardLocation = QPoint(20, 20);
  const QPoint kTurningLocation = QPoint(600, 40);
  const QPoint kGameInfoLocation = QPoint(600, 100);
  const QPoint kFunctionsLocation = QPoint(600, 270);
  const QPoint kNotificationLocation = QPoint(600, 400);
  Ui::ClientGUIClass ui;
  GameInfo* game_info_;
};

class ClientGUINetwork : public ClientGUI
{
  Q_OBJECT

public:
  ClientGUINetwork(std::unique_ptr<ClientAsyncWrapper>& client, const bool& is_offen, QWidget *parent = Q_NULLPTR);

public slots:
  virtual void PassButtonEvent();
  virtual void RetractButtonEvent();

protected:
  virtual bool try_act(const EdgeButton* target_edge);

private:
  std::unique_ptr<ClientAsyncWrapper> client_;
  void set_act_enable(bool enable);
  void receive_and_process_request_async();
};
