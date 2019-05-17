#pragma once

#include <QtWidgets/QMainWindow>
#include <qpushbutton.h>
#include <unordered_map>

#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"
#include <cassert>
#include <QMessageBox>
#include <QPoint>
#include <QLCDNumber>
#include <QLabel>
#include <functional>

#include <windows.h>

/*
两个回调函数，一个触发变化，另一个置布尔型表示进入下一时间

先调研qt槽函数怎么并发，实在不行用std::thread

std::timed_mutex A
std::mutex B

用户点击的时候先释放锁A，再申请锁B，再申请锁
变化开始的时候

*/

static const int kBlockSideLength = 45;
static const int kEdgeWidth = 15;
static const int kGapWidth = 4;
static const int kUnitWidth = kBlockSideLength + kEdgeWidth + kGapWidth * 2;

static const int kZeroLoc = kEdgeWidth + kGapWidth;

typedef QPoint AreaPos;

class ClientGUI;

class AreaButton : public QPushButton
{
  Q_OBJECT

protected:
  const AreaPos pos_;
  PlayerType player_;
  AreaButton(const QPoint& board_loc, const QPoint& loc_offset, const AreaPos& pos, QWidget* parent) :
    QPushButton(parent), pos_(pos), player_(NO_PLAYER)
  {
    assert(pos.x() >= 0 && pos.y() >= 0);
    move(board_loc + loc_offset + pos * kUnitWidth);
    set_color(player2color_[NO_PLAYER]);
  }

  void set_color(const QString& color)
  {
    setStyleSheet(static_cast<QString>("QPushButton{background-color:") + color + ";}");
  }

public:
  static std::unordered_map<int, QString> player2color_;
  PlayerType get_player() const { return player_; }
  void set_player(const PlayerType& old_player, const PlayerType& new_player)
  {
    assert(old_player == player_);
    player_ = new_player;
    set_color(player2color_[new_player]);
  }

  Coordinate get_pos() const
  {
    return Coordinate {static_cast<unsigned int>(pos_.x()), static_cast<unsigned int>(pos_.y())};
  }
};

class BlockButton : public AreaButton
{
public:
  BlockButton(const QPoint& board_loc, const AreaPos& pos, QWidget* parent) :
    AreaButton(board_loc, QPoint(kZeroLoc, kZeroLoc), pos, parent)
  {
    resize(kBlockSideLength, kBlockSideLength);
  }
};

class EdgeButton : public AreaButton
{
public:
  AreaType edge_type_;
public:
  static const QString selected_color_;
public:
  #define REVERSE_PARA(a, b, reverse) (reverse) ? (b) : (a), (reverse) ? (a) : (b)
  EdgeButton(const QPoint& board_loc, const int& side_unit_num, const AreaPos& pos, const bool& is_vert, QWidget* parent) :
    AreaButton(board_loc, QPoint(REVERSE_PARA(kZeroLoc, 0, is_vert)), pos, parent), edge_type_(is_vert ? VERT_EDGE_AREA : HORI_EDGE_AREA)
  {
    assert(pos.x() < side_unit_num && pos.y() < side_unit_num);
    const bool& is_side_edge = (is_vert ? pos.x() : pos.y()) == 0;
    if (is_side_edge)
    {
      resize(REVERSE_PARA(kBlockSideLength, kUnitWidth * side_unit_num + kEdgeWidth / 2, is_vert));
      setMask(QRegion(REVERSE_PARA(0, kEdgeWidth / 2, is_vert),
                      REVERSE_PARA(kBlockSideLength, kEdgeWidth / 2, is_vert)) +
              QRegion(REVERSE_PARA(0, kUnitWidth * side_unit_num, is_vert),
                      REVERSE_PARA(kBlockSideLength, kEdgeWidth / 2, is_vert)));
    }
    else
    {
      resize(REVERSE_PARA(kBlockSideLength, kEdgeWidth, is_vert));
    }
    connect(this, SIGNAL(clicked()), parent, SLOT(EdgeButtonEvent()));
  }
  #undef REVERSE_PARA

  void select()
  {
    set_color(selected_color_);
  }
  void cancel_select()
  {
    set_color(player2color_[player_]);
  }
  AreaType get_edge_type() const
  {
    return edge_type_;
  }
};



class TurningSwitcher : public QLCDNumber
{
private:
  PlayerType turn_;
public:
  TurningSwitcher(QWidget* parent, const QPoint& location) : QLCDNumber(parent), turn_(OFFEN_PLAYER)
  {
    refresh_lcd();
    setDigitCount(2);
    move(location);
    resize(100, 40);
  }
  PlayerType get_turn() const
  {
    assert(turn_ != NO_PLAYER);
    return turn_;
  }
  void switch_turn()
  {
    assert(turn_ != NO_PLAYER);
    turn_ = (turn_ == OFFEN_PLAYER) ? DEFEN_PLAYER : OFFEN_PLAYER;
    refresh_lcd();
  }
  void refresh_lcd()
  {
    display((turn_ == OFFEN_PLAYER) ? "P1" : "P2");
    QPalette lcdpat = palette();
    lcdpat.setColor(QPalette::Normal, QPalette::WindowText, (turn_ == OFFEN_PLAYER) ? Qt::black : Qt::red);
    setPalette(lcdpat);
  }
};

class MovingSelectManager
{
private:
  EdgeButton* edge_to_move_;

public:
  MovingSelectManager() : edge_to_move_(nullptr) {}
  void set_edge(EdgeButton* edge)
  {
    assert(edge);
    clear_edge();
    edge_to_move_ = edge;
    edge->select();
  }
  void clear_edge()
  {
    if (edge_to_move_)
    {
      edge_to_move_->cancel_select();
      edge_to_move_ = nullptr;
    }
  }
  EdgeButton* get_edge()
  {
    return edge_to_move_;
  }
};

class GameInfo : public QWidget
{
private:
  const Game& game_;
  QLCDNumber* offen_own_edge_count;
  QLCDNumber* defen_own_edge_count;
  QLCDNumber* offen_occu_block_count;
  QLCDNumber* defen_occu_block_count;

public:
  GameInfo(QWidget* parent, const QPoint& location, const Game& game) : QWidget(parent), game_(game)
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
    resize(200, 200);
    refresh_info();
  }

  void refresh_info()
  {
    offen_occu_block_count->display(game_.get_board().get_block_occu_counts()[OFFEN_PLAYER]);
    defen_occu_block_count->display(game_.get_board().get_block_occu_counts()[DEFEN_PLAYER]);
    offen_own_edge_count->display(game_.get_edge_own_counts()[OFFEN_PLAYER]);
    defen_own_edge_count->display(game_.get_edge_own_counts()[DEFEN_PLAYER]);
  }
};

class GameFunctions : public QWidget
{
private:
  QPushButton* pass_;
  QPushButton* retreat_;
  QPushButton* draw_;
  QPushButton* surrender_;

public:
  GameFunctions(QWidget* parent, const QPoint& location) : QWidget(parent)
  {
    auto init_btn = [&](QPushButton*& btn, const QString& text, const QPoint& location, const char* method)
    {
      btn = new QPushButton(text, this);
      btn->resize(100, 30);
      btn->move(location);
      QObject::connect(btn, SIGNAL(clicked()), parent, method);
    };
    init_btn(pass_, "PASS", QPoint(0, 0), SLOT(PassButtonEvent()));
    init_btn(retreat_, "RETREAT", QPoint(0, 40), SLOT(foo()));
    move(location);
    resize(100, 300);
  }
};

class ClientGUI : public QMainWindow
{
  Q_OBJECT

public:
  ClientGUI(QWidget *parent = Q_NULLPTR);

 public slots:
  void EdgeButtonEvent()
  {
    auto edge = static_cast<EdgeButton*>(sender());
    if (edge->get_player() == NO_PLAYER)
    {
      try_act(edge);
      select_manager_.clear_edge();
      game_info.refresh_info();
    }
    else if (edge->get_player() == turning_switcher_.get_turn() && edge != select_manager_.get_edge())
    {
      select_manager_.set_edge(edge);
    }
    else
    {
      select_manager_.clear_edge();
    }
  }

  void PassButtonEvent()
  {
    select_manager_.clear_edge();
    turning_switcher_.switch_turn();
  }

  void foo() {}

private:
  void try_act(const EdgeButton* target_edge)
  {
    const EdgeButton* selected_edge = select_manager_.get_edge();
    try
    {
      GameVariety var = selected_edge ?
        game_.Move(selected_edge->get_edge_type(), selected_edge->get_pos(),
                   target_edge->get_edge_type(), target_edge->get_pos(),
                   turning_switcher_.get_turn()) :
        game_.Place(target_edge->get_edge_type(), target_edge->get_pos(),
                    turning_switcher_.get_turn());
      handle_game_variety(var);
      turning_switcher_.switch_turn();
    }
    catch (game_exception exp)
    {
      // TODO: show error
    }
  }

  void handle_game_variety(GameVariety& game_var)
  {
    const auto& area_vars = game_var.get_varieties();
    for (const auto& vars : area_vars)
    {
      for (const AreaVariety& var : vars)
        buttons_[var.type_][var.pos_.x_][var.pos_.y_]->set_player(var.old_player_, var.new_player_);
      qApp->processEvents();
      Sleep(kSleepMs);
    }
}
  
private:
  const int kSleepMs = 200;
  const QSize kWindowSize = QSize(800, 600);
  const QPoint kBoardLocation = QPoint(20, 20);
  const QPoint kTurningLocation = QPoint(600, 40);
  const QPoint kGameInfoLocation = QPoint(600, 100);
  const QPoint kFunctionsLocation = QPoint(600, 270);
  Ui::ClientGUIClass ui;
  Game game_;
  GameInfo game_info;
  TurningSwitcher turning_switcher_;
  MovingSelectManager select_manager_;
  GameFunctions functions_;
  std::array<std::array<std::array<AreaButton*, Game::kBoardSideLen>, Game::kBoardSideLen>, kAreaTypeCount> buttons_;
  void draw_board();
};
