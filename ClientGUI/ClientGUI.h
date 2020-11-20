#pragma once

#include <QtWidgets/QMainWindow>
#include <QPushButton>
#include <unordered_map>
#include <QTextEdit>
#include <QThread>

#include "ai.h"
#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"
#include "../GameCore/game.h"
#include "moving_selector.h"

#define RESOURCE_BACKGROUND_PIC "/resources/background.jpg"
#define RESOURCE_ICON "/resources/client.ico"

static constexpr int kMaxSideLen = 8;

class ClientAsyncWrapper;

class ClientGUI;
class MovingSelectManager;
class GameInfo;
class TurningSwitcher;
class GameFunctions;
class EdgeButton;
class AreaButton;
class BoardWidget;
class GameOptions;

class ClientGUI : public QMainWindow
{
  Q_OBJECT

public:
  ClientGUI(const GameOptions& options, QWidget *parent = Q_NULLPTR);

public slots:
  void EdgeButtonEvent();
  virtual void PassButtonEvent();
  virtual void RetractButtonEvent();
  virtual void start_new_game();
  
protected:
  std::unique_ptr<MovingSelectManager> select_manager_;
  std::unique_ptr<Game> game_;
  TurningSwitcher* turning_switcher_;
  QTextEdit* notification_;
  GameFunctions* functions_;
  BoardWidget* board_;
  virtual bool try_act(const EdgeButton* target_edge);
  virtual void judge_over();
  void impl_game_variety(const GameVariety& game_var);
  void reset_game_variety(const GameVariety& game_var);
  void set_act_enable(bool enable);

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
  virtual void switch_player();
};

class ClientGUINetwork : public ClientGUI
{
  Q_OBJECT

public:
  static constexpr unsigned int kSideLen = 4;
  ClientGUINetwork(std::unique_ptr<ClientAsyncWrapper>&& client, const bool& is_offen, QWidget* parent = Q_NULLPTR);

public slots:
  virtual void PassButtonEvent();
  virtual void RetractButtonEvent();

protected:
  virtual bool try_act(const EdgeButton* target_edge) override;
  virtual void closeEvent(QCloseEvent* event) override;

private:
  std::unique_ptr<ClientAsyncWrapper> client_;
  void receive_and_process_request_async();
  void lost_connection();
  virtual void judge_over() override;
  virtual void switch_player() override;
};

class ClientGUICom : public ClientGUI
{
  Q_OBJECT


public:
  ClientGUICom(const GameOptions& options, const bool is_offen, const uint32_t level, QWidget* parent = Q_NULLPTR);


public slots:
  virtual void PassButtonEvent();
  virtual void RetractButtonEvent();
  void com_act();

protected:
  virtual bool try_act(const EdgeButton* target_edge) override;

private:
  virtual void switch_player() override;

  AI ai_;
  const PlayerType p_;
};

