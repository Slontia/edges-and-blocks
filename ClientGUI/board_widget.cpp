#include "board_widget.h"
#include <cassert>

std::unordered_map<int, QString> AreaButton::player2color_ =
{
  {OFFEN_PLAYER, "black"},
  {DEFEN_PLAYER, "red"},
  {NO_PLAYER, "white"}
};

const QString EdgeButton::selected_color_ = "yellow";

AreaButton::AreaButton(const QPoint& board_loc, const QPoint& loc_offset, const AreaPos& pos, QWidget* parent) :
  QPushButton(parent), pos_(pos), player_(NO_PLAYER)
{
  assert(pos.x() >= 0 && pos.y() >= 0);
  move(board_loc + loc_offset + pos * kUnitWidth);
  set_color(player2color_[NO_PLAYER]);
}

void AreaButton::set_color(const QString& color)
{
  setStyleSheet(static_cast<QString>("QPushButton{background-color:") + color + ";}");
}

void AreaButton::set_player(const PlayerType& old_player, const PlayerType& new_player)
{
  assert(old_player == player_);
  player_ = new_player;
  set_color(player2color_[new_player]);
}

Coordinate AreaButton::get_pos() const
{
  return Coordinate {static_cast<unsigned int>(pos_.x()), static_cast<unsigned int>(pos_.y())};
}

BlockButton::BlockButton(const QPoint& board_loc, const AreaPos& pos, QWidget* parent) :
  AreaButton(board_loc, QPoint(kZeroLoc, kZeroLoc), pos, parent)
{
  resize(kBlockSideLength, kBlockSideLength);
}

#define REVERSE_PARA(a, b, reverse) (reverse) ? (b) : (a), (reverse) ? (a) : (b)
EdgeButton::EdgeButton(const QPoint& board_loc, const int& side_unit_num, const AreaPos& pos, const bool& is_vert, QWidget* parent) :
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

void EdgeButton::select()
{
  set_color(selected_color_);
}

void EdgeButton::cancel_select()
{
  set_color(player2color_[player_]);
}

AreaType EdgeButton::get_edge_type() const
{
  return edge_type_;
}
