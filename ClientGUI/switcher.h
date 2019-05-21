#pragma once
#include <QLCDNumber>
#include <QWidget>
#include "../GameCore/GameCore.h"

class TurningSwitcher : public QLCDNumber
{
private:
  PlayerType turn_;

public:
  TurningSwitcher(QWidget* parent, const QPoint& location);
  PlayerType get_turn() const;
  void switch_turn();
  void refresh_lcd();
};
