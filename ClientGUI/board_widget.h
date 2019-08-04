#pragma once
#include <QWidget>
#include <QPushButton>
#include <unordered_map>
#include <QPoint>
#include <array>
#include "../GameCore/GameCore.h"
#include "../GameCore/game.h"
#include <winsock2.h>
#include <windows.h>
#include "ui_ClientGUI.h"

static const int kBlockSideLength = 45;
static const int kEdgeWidth = 16;
static const int kGapWidth = 4;
static const int kUnitWidth = kBlockSideLength + kEdgeWidth + kGapWidth * 2;

static const int kZeroLoc = kEdgeWidth + kGapWidth;

typedef QPoint AreaPos;

class AreaButton : public QPushButton
{
protected:
  const AreaPos pos_;
  PlayerType player_;
  AreaButton(const QPoint& loc_offset, const AreaPos& pos, QWidget* parent);
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
  BlockButton(const AreaPos& pos, QWidget* parent);
};

class EdgeButton : public AreaButton
{
public:
  AreaType edge_type_;
public:
  static const QString selected_color_;
public:
  EdgeButton(const int& side_unit_num, const AreaPos& pos, const bool& is_vert, QWidget* parent);
  void select();
  void cancel_select();
  AreaType get_edge_type() const;
};

class BoardWidget : public QWidget
{
private:
  const int kSleepMs = 200;
  std::array<std::array<std::array<AreaButton*, Game::kBoardSideLen>, Game::kBoardSideLen>, kAreaTypeCount> buttons_;

public:
  BoardWidget(QWidget* parent, const QPoint& location);
  void impl_game_variety(const GameVariety& game_var);
  void reset_game_variety(const GameVariety& game_var);
};
