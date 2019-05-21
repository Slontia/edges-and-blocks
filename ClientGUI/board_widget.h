#pragma once
#include <QPushButton>
#include <unordered_map>
#include <QPoint>
#include "../GameCore/GameCore.h"

static const int kBlockSideLength = 45;
static const int kEdgeWidth = 15;
static const int kGapWidth = 4;
static const int kUnitWidth = kBlockSideLength + kEdgeWidth + kGapWidth * 2;

static const int kZeroLoc = kEdgeWidth + kGapWidth;

typedef QPoint AreaPos;

class AreaButton : public QPushButton
{
protected:
  const AreaPos pos_;
  PlayerType player_;
  AreaButton(const QPoint& board_loc, const QPoint& loc_offset, const AreaPos& pos, QWidget* parent);
  void set_color(const QString& color);
public:
  static std::unordered_map<int, QString> player2color_;
  PlayerType get_player() const { return player_; }
  void set_player(const PlayerType& old_player, const PlayerType& new_player);
  Coordinate get_pos() const;
};

class BlockButton : public AreaButton
{
public:
  BlockButton(const QPoint& board_loc, const AreaPos& pos, QWidget* parent);
};

class EdgeButton : public AreaButton
{
public:
  AreaType edge_type_;
public:
  static const QString selected_color_;
public:
  EdgeButton(const QPoint& board_loc, const int& side_unit_num, const AreaPos& pos, const bool& is_vert, QWidget* parent);
  void select();
  void cancel_select();
  AreaType get_edge_type() const;
};