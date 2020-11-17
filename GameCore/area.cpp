#include "stdafx.h"
#include "area.h"
#include <cassert>
#include "board.h"
#include "game.h"

Area::Area(Board& board, const Coordinate& pos, const AreaType& type, std::array<int, kPlayerTypeCount>& player_counts)
  : board_(board), pos_(pos), type_(type), occu_player_(NO_PLAYER), player_counts_(player_counts) {}

Area::~Area() {}

AreaVariety Area::set_player(const PlayerType& p)
{
  assert(occu_player_ != p);
  AreaVariety var(*this, p);
  --player_counts_[occu_player_];
  ++player_counts_[p];
  occu_player_ = p;
  return var;
}

PlayerType Area::get_player() const
{
  return occu_player_;
}

bool Area::operator==(const Area& area) const
{
  return pos_.x_ == area.pos_.x_ && pos_.y_ == area.pos_.y_ && type_ == area.type_;
}

bool Area::operator!=(const Area& area) const
{
  return !operator==(area);
}

BlockArea::BlockArea(Board& board, const Coordinate& pos, std::array<int, kPlayerTypeCount>& player_counts) : 
  Area(board, pos, BLOCK_AREA, player_counts){}

BlockArea::~BlockArea() {}

BlockArea::AdjaceEdges BlockArea::get_adjace_edges() const
{
  auto get_edge = [this](const unsigned int& x, const unsigned int& y, const AreaType& type)
  {
    assert(type == HORI_EDGE_AREA || type == VERT_EDGE_AREA);
    return std::dynamic_pointer_cast<EdgeArea>(board_.get_area_safe(Coordinate(x, y), type));
  };
  return AdjaceEdges {
    get_edge(pos_.x_, pos_.y_, HORI_EDGE_AREA),
    get_edge(pos_.x_, pos_.y_ + 1, HORI_EDGE_AREA),
    get_edge(pos_.x_, pos_.y_, VERT_EDGE_AREA),
    get_edge(pos_.x_ + 1, pos_.y_, VERT_EDGE_AREA)
  };
}

bool BlockArea::is_broken() const
{
  if (occu_player_ == NO_PLAYER) { return false; } 
  for (const auto& edge : get_adjace_edges())
  {
    if (const auto player = edge->get_player(); player != occu_player_ && player != NO_PLAYER)
    {
      return true;
    }
  }
  return false;
}

bool BlockArea::is_occupied_by(const PlayerType& p) const
{
  assert(p != NO_PLAYER);
  for (const auto& edge : get_adjace_edges())
  {
    if (edge->get_player() != p) { return false; }
  }
  return true;
}

std::pair<int32_t, int32_t> BlockArea::score() const
{
  static const int32_t kScoreTable[kEdgeCountAdjaceBlock + 1][kEdgeCountAdjaceBlock + 1] =
  {
    {0 ,0 ,0 ,0 ,0 },
    {1 ,0 ,0 ,0 ,0 },
    {2 ,2 ,0 ,0 ,0 },
    {4 ,1 ,0 ,0 ,0 },
    {4 ,0 ,0 ,0 ,0 }
  };
  std::pair<int32_t, int32_t> edge_counts = { 0, 0 };
  for (const auto& edge : get_adjace_edges())
  {
    if (const PlayerType p = edge->get_player(); p == OFFEN_PLAYER)
    {
      ++std::get<OFFEN_PLAYER>(edge_counts);
    }
    else if (p == DEFEN_PLAYER)
    {
      ++std::get<DEFEN_PLAYER>(edge_counts);
    }
  }
  return { kScoreTable[std::get<0>(edge_counts)][std::get<1>(edge_counts)],
           kScoreTable[std::get<1>(edge_counts)][std::get<0>(edge_counts)] };
}

EdgeAreaPtr BlockArea::is_captured_by(const PlayerType& p) const
{
  assert(p != NO_PLAYER);
  const PlayerType oppo = (p == DEFEN_PLAYER) ? OFFEN_PLAYER : DEFEN_PLAYER;
  EdgeAreaPtr oppo_edge = nullptr;
  for (const auto& edge : get_adjace_edges())
  {
    if (edge->get_player() == NO_PLAYER) { return nullptr; } // has free edge
    if (edge->get_player() == oppo)
    {
      if (oppo_edge) { return nullptr; } // more than one oppo edges
      oppo_edge = edge;
    }
  }
  return oppo_edge;
}

EdgeArea::EdgeArea(Board& board, const Coordinate& pos, const AreaType& edge_type, std::array<int, kPlayerTypeCount>& player_counts) : 
  Area(board, pos, edge_type, player_counts)
{ 
  assert(edge_type == HORI_EDGE_AREA || edge_type == VERT_EDGE_AREA);
}

EdgeArea::~EdgeArea() {}

EdgeArea::AdjaceBlocks EdgeArea::get_adjace_blocks() const
{
  auto get_block = [this](const unsigned int& x, const unsigned int& y)
  {
    return std::dynamic_pointer_cast<BlockArea>(board_.get_area_safe(Coordinate(x, y), BLOCK_AREA));
  };
  return (type_ == HORI_EDGE_AREA) ? AdjaceBlocks {
    /* HORI_EDGE_AREA */
    get_block(pos_.x_, pos_.y_),
    get_block(pos_.x_, pos_.y_ - 1)
  } : AdjaceBlocks {
    /* VERT_EDGE_AREA */
    get_block(pos_.x_, pos_.y_),
    get_block(pos_.x_ - 1, pos_.y_)
  };
}

EdgeArea::AdjaceEdges EdgeArea::get_adjace_edges() const
{
  auto get_edge = [this](const unsigned int& x, const unsigned int& y, const AreaType& type)
  {
    assert(type == HORI_EDGE_AREA || type == VERT_EDGE_AREA);
    return std::dynamic_pointer_cast<EdgeArea>(board_.get_area_safe(Coordinate(x, y), type));
  };
  return (type_ == HORI_EDGE_AREA) ? AdjaceEdges {
    /* HORI_EDGE_AREA */
    get_edge(pos_.x_ - 1, pos_.y_, HORI_EDGE_AREA),
    get_edge(pos_.x_ + 1, pos_.y_, HORI_EDGE_AREA),
    get_edge(pos_.x_, pos_.y_, VERT_EDGE_AREA),
    get_edge(pos_.x_ + 1, pos_.y_, VERT_EDGE_AREA),
    get_edge(pos_.x_, pos_.y_ - 1, VERT_EDGE_AREA),
    get_edge(pos_.x_ + 1, pos_.y_ - 1, VERT_EDGE_AREA)
  } : AdjaceEdges {
    /* VERT_EDGE_AREA */
    get_edge(pos_.x_, pos_.y_ - 1, VERT_EDGE_AREA),
    get_edge(pos_.x_, pos_.y_ + 1, VERT_EDGE_AREA),
    get_edge(pos_.x_, pos_.y_, HORI_EDGE_AREA),
    get_edge(pos_.x_ - 1, pos_.y_, HORI_EDGE_AREA),
    get_edge(pos_.x_, pos_.y_ + 1, HORI_EDGE_AREA),
    get_edge(pos_.x_ - 1, pos_.y_ + 1, HORI_EDGE_AREA)
  };
}

bool EdgeArea::is_adjace(const EdgeArea& edge) const
{
  for (const auto& adjaced_edge : get_adjace_edges())
  {
    if (edge == *adjaced_edge) { return true; }
  }
  return false;
}

/* Assume block is one of the adjacent blocks.
*/
BlockAreaPtr EdgeArea::get_another_block(const BlockArea& block) const
{
  for (auto& adjaced_block : get_adjace_blocks())
  {
    if (block != *adjaced_block)
    {
      return adjaced_block;
    }
  }
  assert(false);
  return nullptr;
}