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
#include <Mmsystem.h>
#include "resource.h"
#include "ui_ClientGUI.h"
#pragma comment(lib,"winmm.lib")

static constexpr int kBlockSideLength = 45;
static constexpr int kEdgeWidth = 16; // better even
static constexpr int kGapWidth = 4; // better even
static constexpr int kEdgeLength = kBlockSideLength + kEdgeWidth + kGapWidth;
static constexpr int kUnitWidth = kBlockSideLength + kEdgeWidth + kGapWidth * 2;
static constexpr int kZeroLoc = kEdgeWidth + kGapWidth; // the x or y of the first block

typedef QPoint AreaPos;

class AreaButton : public QPushButton
{
protected:
  const AreaPos pos_;
  PlayerType player_;
  AreaButton(const QPoint& loc_offset, const AreaPos& pos, QWidget* parent);
  virtual void set_color(const QString& color);
public:
  static std::unordered_map<int, QString> player2color_;
  PlayerType get_player() const { return player_; }
  virtual void set_player(const PlayerType& old_player, const PlayerType& new_player, const bool play_sound);
  Coordinate get_pos() const;
};

class BlockButton : public AreaButton
{
public:
  BlockButton(const AreaPos& pos, QWidget* parent);
  virtual void set_player(const PlayerType& old_player, const PlayerType& new_player, const bool play_sound);
};

class EdgeButton : public AreaButton
{
public:
  AreaType edge_type_;
public:
  static const QString selected_color_;
  static std::unordered_map<int, QString> player2hovercolor_;
public:
  EdgeButton(const int& side_unit_num, const AreaPos& pos, const bool& is_vert, QWidget* parent);
  void select();
  void cancel_select();
  AreaType get_edge_type() const;
  void set_hover_color(const QString& color);
  virtual void set_player(const PlayerType& old_player, const PlayerType& new_player, const bool play_sound);
private:
  QString color_;
  QString hover_color_;
  virtual void set_color(const QString& color);
  void refresh_stylesheet();
};

class BoardWidget : public QWidget
{
public:
  BoardWidget(unsigned int side_len, const QPoint& location, QWidget* parent = Q_NULLPTR);
  void impl_game_variety(const GameVariety& game_var);
  void reset_game_variety(const GameVariety& game_var);
  void set_enable(bool enable);
  void set_hover_color(const PlayerType& cur_player);

private:
  static constexpr int kSleepMs = 200;
  const unsigned int side_len_;
  std::array<std::vector<std::vector<AreaButton*>>, kAreaTypeCount> buttons_;
};

extern HMODULE hModule;
bool play_sound_as_resource(const int sound_id);
