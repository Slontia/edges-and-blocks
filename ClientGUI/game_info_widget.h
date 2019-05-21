#pragma once
#include <QWidget>
#include <QLCDNumber>

class Game;

class GameInfo : public QWidget
{
private:
  const Game& game_;
  QLCDNumber* offen_own_edge_count;
  QLCDNumber* defen_own_edge_count;
  QLCDNumber* offen_occu_block_count;
  QLCDNumber* defen_occu_block_count;

public:
  GameInfo(QWidget* parent, const QPoint& location, const Game& game);
  void refresh_info();
};