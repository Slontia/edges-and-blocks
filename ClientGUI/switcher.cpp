#include "switcher.h"
#include <cassert>

TurningSwitcher::TurningSwitcher(QWidget* parent, const QPoint& location) : QLCDNumber(parent), turn_(OFFEN_PLAYER)
{
  refresh_lcd();
  setDigitCount(2);
  move(location);
  resize(100, 40);
}

PlayerType TurningSwitcher::get_turn() const
{
  assert(turn_ != NO_PLAYER);
  return turn_;
}

void TurningSwitcher::switch_turn()
{
  assert(turn_ != NO_PLAYER);
  turn_ = (turn_ == OFFEN_PLAYER) ? DEFEN_PLAYER : OFFEN_PLAYER;
  refresh_lcd();
}

void TurningSwitcher::refresh_lcd()
{
  display((turn_ == OFFEN_PLAYER) ? "P1" : "P2");
  QPalette lcdpat = palette();
  lcdpat.setColor(QPalette::Normal, QPalette::WindowText, (turn_ == OFFEN_PLAYER) ? Qt::black : Qt::red);
  setPalette(lcdpat);
}

