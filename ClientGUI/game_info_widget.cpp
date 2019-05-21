#include "game_info_widget.h"
#include "../GameCore/game.h"
#include <QLabel>

GameInfo::GameInfo(QWidget* parent, const QPoint& location, const Game& game) : QWidget(parent), game_(game)
{
  auto init_lcd = [&](QLCDNumber*& lcd, QColor color, QPoint location)
  {
    lcd = new QLCDNumber(this);
    QPalette lcdpat = lcd->palette();
    lcdpat.setColor(QPalette::Normal, QPalette::WindowText, color);
    lcd->setPalette(lcdpat);
    lcd->setDigitCount(3);
    lcd->move(location);
    lcd->resize(50, 50);
  };
  (new QLabel("Edges Left", this))->move(0, 0);
  init_lcd(offen_own_edge_count, Qt::black, QPoint(0, 20));
  init_lcd(defen_own_edge_count, Qt::red, QPoint(0, 100));
  (new QLabel("Blocks Occupied", this))->move(100, 0);
  init_lcd(offen_occu_block_count, Qt::black, QPoint(100, 20));
  init_lcd(defen_occu_block_count, Qt::red, QPoint(100, 100));
  move(location);
  resize(200, 160);
  refresh_info();
}

void GameInfo::refresh_info()
{
  offen_occu_block_count->display(game_.get_board().get_block_occu_counts()[OFFEN_PLAYER]);
  defen_occu_block_count->display(game_.get_board().get_block_occu_counts()[DEFEN_PLAYER]);
  offen_own_edge_count->display(game_.get_edge_own_counts()[OFFEN_PLAYER]);
  defen_own_edge_count->display(game_.get_edge_own_counts()[DEFEN_PLAYER]);
}
