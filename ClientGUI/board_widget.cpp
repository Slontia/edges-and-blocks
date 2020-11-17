#include "board_widget.h"
#include <cassert>
#include <iostream>
#define REVERSE_PARA(a, b, reverse) (reverse) ? (b) : (a), (reverse) ? (a) : (b)

std::unordered_map<int, QString> AreaButton::player2color_ =
{
  {OFFEN_PLAYER, "black"},
  {DEFEN_PLAYER, "red"},
  {NO_PLAYER, "rgba(255,255,255,100)"}
};

std::unordered_map<int, QString> EdgeButton::player2hovercolor_ =
{
  {OFFEN_PLAYER, "#888888"},
  {DEFEN_PLAYER, "#FF8888"}
};

const QString EdgeButton::selected_color_ = "yellow";

AreaButton::AreaButton(const QPoint& loc_offset, const AreaPos& pos, QWidget* parent) :
  QPushButton(parent), pos_(pos), player_(NO_PLAYER)
{
  assert(pos.x() >= 0 && pos.y() >= 0);
  move(loc_offset + pos * kUnitWidth);
  set_color(player2color_[NO_PLAYER]);
}

void AreaButton::set_color(const QString& color)
{
  setStyleSheet(static_cast<QString>("QPushButton{border-style:none;padding:10px;border-radius:3px;background-color:") + color + ";}");
}

void AreaButton::set_player(const PlayerType& old_player, const PlayerType& new_player, const bool play_sound)
{
  assert(old_player == player_);
  player_ = new_player;
  set_color(player2color_[new_player]);
}

Coordinate AreaButton::get_pos() const
{
  return Coordinate {static_cast<unsigned int>(pos_.x()), static_cast<unsigned int>(pos_.y())};
}

BlockButton::BlockButton(const AreaPos& pos, QWidget* parent) :
  AreaButton(QPoint(kZeroLoc, kZeroLoc), pos, parent)
{
  resize(kBlockSideLength, kBlockSideLength);
  setEnabled(false);
}

void BlockButton::set_player(const PlayerType& old_player, const PlayerType& new_player, const bool play_sound)
{
  AreaButton::set_player(old_player, new_player, play_sound);
  if (play_sound)
  {
    play_sound_as_resource(new_player == NO_PLAYER ? DESTROY_BLOCK_WAVE : CREATE_BLOCK_WAVE);
  }
}

EdgeButton::EdgeButton(const int& side_unit_num, const AreaPos& pos, const bool& is_vert, QWidget* parent) :
  AreaButton(QPoint(REVERSE_PARA(kZeroLoc, 0, is_vert)), pos, parent), 
  edge_type_(is_vert ? VERT_EDGE_AREA : HORI_EDGE_AREA), 
  color_(player2color_[NO_PLAYER]), hover_color_("")
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
  connect(this, SIGNAL(clicked()), parent->parentWidget(), SLOT(EdgeButtonEvent()));
}

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

void EdgeButton::set_player(const PlayerType& old_player, const PlayerType& new_player, const bool play_sound)
{
  AreaButton::set_player(old_player, new_player, play_sound);
  set_hover_color("");
  if (play_sound && new_player != NO_PLAYER)
  {
    play_sound_as_resource(FALL_EDGE_WAVE);
  }
}

BoardWidget::BoardWidget(const unsigned int side_len, const QPoint& location, QWidget* parent)
  : side_len_(side_len), QWidget(parent)
{
  for (int x = 0; x < side_len_; ++x)
  {
    buttons_[BLOCK_AREA].emplace_back();
    buttons_[HORI_EDGE_AREA].emplace_back();
    buttons_[VERT_EDGE_AREA].emplace_back();
    for (int y = 0; y < side_len_; ++y)
    {
      const AreaPos pos(x, y);
      buttons_[BLOCK_AREA][x].emplace_back(new BlockButton(pos, this));
      buttons_[HORI_EDGE_AREA][x].emplace_back(new EdgeButton(side_len_, pos, false, this));
      buttons_[VERT_EDGE_AREA][x].emplace_back(new EdgeButton(side_len_, pos, true, this));
    }
  }
  move(location);
  resize(QSize(kUnitWidth, kUnitWidth) * side_len_ + QSize(kGapWidth + kEdgeWidth / 2, kGapWidth + kEdgeWidth / 2));
  play_sound_as_resource(START_GAME_WAVE);
}

void BoardWidget::impl_game_variety(const GameVariety& game_var)
{
  bool first_act = true;
  for (const auto& vars : game_var.area_varieties_)
  {
    if (vars.empty())
    {
      continue;
    }
    for (const AreaVariety& var : vars)
    {
      buttons_[var.type_][var.pos_.x_][var.pos_.y_]->set_player(var.old_player_, var.new_player_, first_act || var.type_ == BLOCK_AREA /* play_sound */);
    }
    first_act = false;
    qApp->processEvents();
    Sleep(kSleepMs);
  }
}

void BoardWidget::reset_game_variety(const GameVariety& game_var)
{
  for (int i = game_var.area_varieties_.size() - 1; i >= 0; --i)
  {
    for (const auto& var : game_var.area_varieties_[i])
    {
      buttons_[var.type_][var.pos_.x_][var.pos_.y_]->set_player(var.new_player_, var.old_player_, false /* play_sound */);
    }
  }
}

void EdgeButton::set_color(const QString& color)
{
  color_ = color;
  refresh_stylesheet();
}

void EdgeButton::set_hover_color(const QString& color)
{
  hover_color_ = color;
  refresh_stylesheet();
}

void EdgeButton::refresh_stylesheet()
{
  QString style = "";
  if (!color_.isEmpty()) { style += "QPushButton{border-style:none;background-color:" + color_ + ";}";  }
  if (!hover_color_.isEmpty()) { style += "QPushButton:hover{background-color:" + hover_color_ + ";}"; }
  setStyleSheet(style);
}

void BoardWidget::set_hover_color(const PlayerType& cur_player)
{
  auto set_edge_buttons_enable = [&](AreaType type)
  {
    for (const auto& area_row_btns : buttons_[type])
    {
      for (const auto& button : area_row_btns)
      {
        if (button->get_player() == NO_PLAYER)
        {
          reinterpret_cast<EdgeButton *const>(button)->set_hover_color(EdgeButton::player2hovercolor_[cur_player]);
        }
      }
    }
  };
  set_edge_buttons_enable(HORI_EDGE_AREA);
  set_edge_buttons_enable(VERT_EDGE_AREA);
  
}

void BoardWidget::set_enable(bool enable)
{
  auto set_edge_buttons_enable = [&](AreaType type)
  {
    for (const auto& area_row_btns : buttons_[type])
    {
      for (const auto& button : area_row_btns)
      {
        button->setEnabled(enable);
      }
    }
  };
  set_edge_buttons_enable(HORI_EDGE_AREA);
  set_edge_buttons_enable(VERT_EDGE_AREA);
}

HMODULE hModule;

bool play_sound_as_resource(const int sound_id)
{
  bool ret = false;

  // Find the WAVE resource. 
  auto hResInfo = FindResource(hModule, MAKEINTRESOURCE(sound_id), TEXT("WAVE"));
  if (hResInfo == NULL) { return false; }

  // Load the WAVE resource. 
  auto hRes = LoadResource(hModule, hResInfo);
  if (hRes == NULL) { return false; }

  // Lock the WAVE resource and play it. 
  auto lpRes = LockResource(hRes);
  if (lpRes != NULL)
  {
    ret = sndPlaySound(static_cast<LPCWSTR>(lpRes), SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    UnlockResource(hRes);
  }
  else
  {
    ret = false;
  }

  // Free the WAVE resource and return success or failure. 
  FreeResource(hRes);
  return ret;
}
