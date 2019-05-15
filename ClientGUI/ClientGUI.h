#pragma once

#include <QtWidgets/QMainWindow>
#include <qpushbutton.h>
#include <unordered_map>

#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"
#include <cassert>
#include <QMessageBox>

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



class TurningSwitcher
{
private:
  PlayerType turn_;
public:
  TurningSwitcher() : turn_(OFFEN_PLAYER) {}
  PlayerType get_turn() const
  {
    assert(turn_ != NO_PLAYER);
    return turn_;
  }
  void switch_turn()
  {
    assert(turn_ != NO_PLAYER);
    turn_ = (turn_ == OFFEN_PLAYER) ? DEFEN_PLAYER : OFFEN_PLAYER;
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

struct ClientManager
{
  Game game_;
  TurningSwitcher turning_switcher_;
  MovingSelectManager select_manager_;
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
      try_act(manager_.select_manager_.get_edge(), edge);
    else if (edge->get_player() == manager_.turning_switcher_.get_turn())
      manager_.select_manager_.set_edge(edge);
    else
      manager_.select_manager_.clear_edge();
  }

private:
  void try_act(const EdgeButton* selected_edge, const EdgeButton* target_edge)
  {
    try
    {
      GameVariety var = selected_edge ?
        manager_.game_.Move(selected_edge->get_edge_type(), selected_edge->get_pos(),
                            target_edge->get_edge_type(), target_edge->get_pos(),
                            manager_.turning_switcher_.get_turn()) :
        manager_.game_.Place(target_edge->get_edge_type(), target_edge->get_pos(),
                             manager_.turning_switcher_.get_turn());
      handle_game_variety(var);
      manager_.turning_switcher_.switch_turn();
    }
    catch (game_exception exp)
    {
      // TODO: show error
      manager_.select_manager_.clear_edge();
    }
  }

  void handle_game_variety(const GameVariety& game_var)
  {
    const auto& area_vars = game_var.get_varieties();
    for (const auto& vars : area_vars)
      for (const AreaVariety& var : vars)
        buttons_[var.type_][var.pos_.x_][var.pos_.y_]->set_player(var.old_player_, var.new_player_);
  }

private:
  Ui::ClientGUIClass ui;
  ClientManager manager_;
  std::array<std::array<std::array<AreaButton*, Game::kBoardSideLen>, Game::kBoardSideLen>, kAreaTypeCount> buttons_;
  void draw_board();
};
