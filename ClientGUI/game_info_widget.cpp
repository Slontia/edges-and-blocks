#include "game_info_widget.h"
#include "../GameCore/game.h"
#include <QLabel>

IntQLCDNumber::IntQLCDNumber(QWidget* parent, const QColor& color, const QPoint& location, const int& value = 0)
{
  lcd_ = new QLCDNumber(parent);
  QPalette lcdpat = lcd_->palette();
  lcdpat.setColor(QPalette::Normal, QPalette::WindowText, color);
  lcd_->setPalette(lcdpat);
  lcd_->setDigitCount(3);
  lcd_->move(location);
  lcd_->resize(50, 50);
  set_value(value);
}
void IntQLCDNumber::set_value(const int& value)
{
  value_ = value;
  lcd_->display(value_);
}

void IntQLCDNumber::add_value(const int& value)
{
  set_value(value_ + value);
}

GameInfo::GameInfo(QWidget* parent, const QPoint& location, const Game& game) : 
  QWidget(parent), game_(game),
  offen_own_edge_count_(this, Qt::black, QPoint(0, 20), Game::kInitOffenEdgeOwnCount),
  defen_own_edge_count_(this, Qt::red, QPoint(0, 100), Game::kInitDefenEdgeOwnCount),
  offen_occu_block_count_(this, Qt::black, QPoint(100, 20), 0),
  defen_occu_block_count_(this, Qt::red, QPoint(100, 100), 0)
{
  (new QLabel("Edges Left", this))->move(0, 0);
  (new QLabel("Blocks Occupied", this))->move(100, 0);
  move(location);
  resize(200, 160);
}

void GameInfo::impl_game_variety(const GameVariety& game_var)
{
  handle_game_variety(game_var, false);
}

void GameInfo::reset_game_variety(const GameVariety& game_var)
{
  handle_game_variety(game_var, true);
}

void GameInfo::handle_game_variety(const GameVariety& game_var, const bool& reset)
{
  int sign = reset ? -1 : 1;
  offen_own_edge_count_.add_value(sign * game_var.offen_own_edge_count_variety_);
  defen_own_edge_count_.add_value(sign * game_var.defen_own_edge_count_variety_);
}

void GameInfo::refresh_occu_block_count()
{
  offen_occu_block_count_.set_value(game_.get_board().get_block_occu_counts()[OFFEN_PLAYER]);
  defen_occu_block_count_.set_value(game_.get_board().get_block_occu_counts()[DEFEN_PLAYER]);
}
