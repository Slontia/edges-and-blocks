#pragma once
#include "GameCore.h"
#include <memory>
#include <vector>

class Area;

struct AreaVariety
{
  const AreaType type_;
  const Coordinate pos_;
  const PlayerType old_player_;
  const PlayerType new_player_;
  AreaVariety(const Area& area, const PlayerType& new_p);
  ~AreaVariety();
};

struct GameVariety
{
  std::vector<std::vector<AreaVariety>> area_varieties_;
  int offen_own_edge_count_variety_;
  int defen_own_edge_count_variety_;
  GameVariety();
  ~GameVariety();
  void push(const AreaVariety& area_var);
  void clear();
  void to_next_time();
};

typedef std::shared_ptr<GameVariety> GameVarietyPtr;
