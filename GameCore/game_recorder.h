#pragma once
#include "GameCore.h"

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

class GameVariety
{
private:
  std::vector<std::vector<AreaVariety>> area_varieties_;
public:
  GameVariety();
  ~GameVariety();
  void push(const AreaVariety& area_var);
  void clear();
  void to_next_time();
  const std::vector<std::vector<AreaVariety>> get_varieties()
  {
    while (!area_varieties_.empty() && area_varieties_.back().empty())
      area_varieties_.pop_back();
    return area_varieties_;
  }
};

typedef std::shared_ptr<GameVariety> GameVarietyPtr;
