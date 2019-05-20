#pragma once
#include "area.h"

class Board
{
public:
  typedef std::array<int, kPlayerTypeCount> OccuCounts;

private:
  unsigned int side_len_;
  std::vector<std::vector<BlockAreaPtr>> blocks_;
  std::vector<std::vector<EdgeAreaPtr>> hori_edges_;
  std::vector<std::vector<EdgeAreaPtr>> vert_edges_;
  OccuCounts block_occu_counts_;
  OccuCounts edge_occu_counts_;
  void build_board();
  void init_board();

public:
  Board(const unsigned int& side_len);
  ~Board();
  bool is_valid_pos(const Coordinate& pos) const;
  BlockAreaPtr get_block(const Coordinate& pos);
  EdgeAreaPtr get_edge(const Coordinate& pos, const AreaType& edge_type);
  const OccuCounts& get_block_occu_counts() const;
  const OccuCounts& get_edge_occu_counts() const;
};