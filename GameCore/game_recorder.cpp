#include "stdafx.h"
#include "game_recorder.h"
#include "area.h"

AreaVariety::AreaVariety(const Area& area, const PlayerType& new_p) : type_(area.type_), pos_(area.pos_), old_player_(area.get_player()), new_player_(new_p) {}

AreaVariety::~AreaVariety() {}

GameVariety::GameVariety() : offen_own_edge_count_variety_(0), defen_own_edge_count_variety_(0)
{
  area_varieties_.emplace_back();
}

GameVariety::~GameVariety() {}

void GameVariety::push(const AreaVariety& var)
{
  area_varieties_.back().push_back(var);
}

void GameVariety::clear()
{
  area_varieties_.clear();
  area_varieties_.emplace_back();
}

void GameVariety::to_next_time()
{
  if (!area_varieties_.back().empty())
  {
    area_varieties_.emplace_back();
  }
}
