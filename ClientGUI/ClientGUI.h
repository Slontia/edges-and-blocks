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
�����ص�������һ�������仯����һ���ò����ͱ�ʾ������һʱ��

�ȵ���qt�ۺ�����ô������ʵ�ڲ�����std::thread

std::timed_mutex A
std::mutex B

�û������ʱ�����ͷ���A����������B����������
�仯��ʼ��ʱ��

======

���Կ����û��޸�sleepʱ�䣬����sleepʱ����������ʱ����

======

��Board��Ϊ������Widget
����functions�������飬text + event_function

======

����QWidget�ľͲ�Ҫnew��

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
