#pragma once

#include <QtWidgets/QMainWindow>
#include <QPushButton>
#include <unordered_map>
#include <QTextEdit>

#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"
#include "../GameCore/game.h"
#include <windows.h>
#include "moving_selector.h"

/*
两个回调函数，一个触发变化，另一个置布尔型表示进入下一时间

先调研qt槽函数怎么并发，实在不行用std::thread

std::timed_mutex A
std::mutex B

用户点击的时候先释放锁A，再申请锁B，再申请锁
变化开始的时候

======

可以考虑用户修改sleep时间，但是sleep时间算在消耗时间里

======

将Board作为独立的Widget
创建functions传入数组，text + event_function

======

不是QWidget的就不要new了

*/

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
 void PassButtonEvent();
 void RetractButtonEvent();
  
private:
  QPushButton* pass_;
  const QSize kWindowSize = QSize(800, 600);
  const QPoint kBoardLocation = QPoint(20, 20);
  const QPoint kTurningLocation = QPoint(600, 40);
  const QPoint kGameInfoLocation = QPoint(600, 100);
  const QPoint kFunctionsLocation = QPoint(600, 270);
  const QPoint kNotificationLocation = QPoint(600, 400);
  Ui::ClientGUIClass ui;
  std::unique_ptr<Game> game_;
  std::unique_ptr<MovingSelectManager> select_manager_;
  QTextEdit* notification_;
  GameInfo* game_info_;
  TurningSwitcher* turning_switcher_;
  GameFunctions* functions_;
  BoardWidget* board_;
  void try_act(const EdgeButton* target_edge);
  void impl_game_variety(const GameVariety& game_var);
  void reset_game_variety(const GameVariety& game_var);
};
