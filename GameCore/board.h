#pragma once
#include "area.h"
#include <array>

class Board
{
public:
  typedef std::array<int, kPlayerTypeCount> OccuCounts;
  const unsigned int side_len_;
  Board(const unsigned int& side_len);
  ~Board();
  bool is_valid_pos(const Coordinate& pos) const;
  AreaPtr get_area(const Coordinate& pos, const AreaType& edge_type);
  AreaPtr get_area_safe(const Coordinate& pos, const AreaType& edge_type);
  BlockAreaPtr get_block(const Coordinate& pos);
  EdgeAreaPtr get_edge(const Coordinate& pos, const AreaType& edge_type);
  const AreaPtr get_area(const Coordinate& pos, const AreaType& edge_type) const;
  const AreaPtr get_area_safe(const Coordinate& pos, const AreaType& edge_type) const;
  const BlockAreaPtr get_block(const Coordinate& pos) const;
  const EdgeAreaPtr get_edge(const Coordinate& pos, const AreaType& edge_type) const;
  const OccuCounts& get_block_occu_counts() const;
  const OccuCounts& get_edge_occu_counts() const;
  void reset_game_variety(const GameVariety& game_var);
  std::pair<int32_t, int32_t> score() const;

private:
  std::array<std::vector<std::vector<AreaPtr>>, kAreaTypeCount> areas_;
  OccuCounts block_occu_counts_;
  OccuCounts edge_occu_counts_;
  void build_board();
};