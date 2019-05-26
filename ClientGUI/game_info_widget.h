#pragma once
#include <QWidget>
#include <QLCDNumber>
#include "../GameCore/game.h"

class Game;

class IntQLCDNumber
{
private:
  QLCDNumber* lcd_;
  int value_;
public:
  IntQLCDNumber(QWidget* parent, const QColor& color, const QPoint& location, const int& values);
  void set_value(const int& value);
  void add_value(const int& value);
};

class GameInfo : public QWidget
{
private:
  const Game& game_;
  IntQLCDNumber offen_own_edge_count_;
  IntQLCDNumber defen_own_edge_count_;
  IntQLCDNumber offen_occu_block_count_;
  IntQLCDNumber defen_occu_block_count_;
  void handle_game_variety(const GameVariety& game_var, const bool& reset);

public:
  GameInfo(QWidget* parent, const QPoint& location, const Game& game);
  void impl_game_variety(const GameVariety& game_var);
  void reset_game_variety(const GameVariety& game_var);
  void refresh_occu_block_count();
};
