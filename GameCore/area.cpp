#include "stdafx.h"
#include "area.h"
#include <cassert>

Area::Area(const Coordinate& pos, const AreaType& type, std::array<int, kPlayerTypeCount>& player_counts) : pos_(pos), type_(type), occu_player_(NO_PLAYER), player_counts_(player_counts) {}

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
  return !this->operator==(area);
}

BlockArea::BlockArea(const Coordinate& pos, std::array<int, kPlayerTypeCount>& player_counts) : Area(pos, BLOCK_AREA, player_counts) {}

BlockArea::~BlockArea() {}

void BlockArea::set_adjace(AdjaceEdges&& adjace_edges)
{
  adjace_edges_ = adjace_edges;
}

bool BlockArea::is_broken()
{
  if (occu_player_ == NO_PLAYER) return false;
  for (const EdgeAreaPtr& edge : adjace_edges_)
    if (const auto player = edge->get_player(); player != occu_player_ && player != NO_PLAYER) return true;
  return false;
}

bool BlockArea::is_occupied_by(const PlayerType& p)
{
  assert(p != NO_PLAYER);
  for (const EdgeAreaPtr& edge : adjace_edges_)
    if (edge->get_player() != p) return false;
  return true;
}

EdgeAreaPtr BlockArea::is_captured_by(const PlayerType& p)
{
  assert(p != NO_PLAYER);
  const PlayerType oppo = (p == DEFEN_PLAYER) ? OFFEN_PLAYER : DEFEN_PLAYER;
  EdgeAreaPtr oppo_edge = nullptr;
  for (EdgeAreaPtr edge : adjace_edges_)
  {
    if (edge->get_player() == NO_PLAYER)
      return nullptr; // has free edge
    if (edge->get_player() == oppo)
    {
      if (oppo_edge)
        return nullptr; // more than one oppo edges
      oppo_edge = edge;
    }
  }
  return oppo_edge;
}

EdgeArea::EdgeArea(const Coordinate& pos, const AreaType& edge_type, std::array<int, kPlayerTypeCount>& player_counts) : Area(pos, edge_type, player_counts)
{
  assert(edge_type == HORI_EDGE_AREA || edge_type == VERT_EDGE_AREA);
}

EdgeArea::~EdgeArea() {}

void EdgeArea::set_adjace(AdjaceBlocks&& adjace_blocks, AdjaceEdges&& adjace_edges)
{
  adjace_blocks_ = adjace_blocks;
  adjace_edges_ = adjace_edges;
}

bool EdgeArea::is_adjace(const EdgeArea& edge)
{
  for (const EdgeAreaPtr& e : adjace_edges_)
  {
    if (edge == *e)
    {
      return true;
    }
  }
  return false;
}

/* Assume block is one of the adjacent blocks.
*/
BlockAreaPtr EdgeArea::get_another_block(const BlockArea& block)
{
  BlockAreaPtr another_block = nullptr;
  for (const BlockAreaPtr& b : adjace_blocks_)
  {
    if (*b != block) // different blocks
    {
      assert(!another_block);
      another_block = b;
    }
  }
  assert(another_block);
  return another_block;
}